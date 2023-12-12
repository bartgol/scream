#include "eamxx_model_init.hpp"

#include "share/io/scream_scorpio_interface.hpp"

#include <regex>

namespace scream
{

ModelInit::
ModelInit (const std::vector<Field>& eamxx_inputs,
           const std::shared_ptr<const AbstractGrid>& ic_grid,
           const util::TimeStamp& t0)
 : m_t0(t0)
 , m_ic_grid (ic_grid)
{
  separate_inputs (eamxx_inputs);
}

void ModelInit::
set_constant_fields (const std::vector<std::string>& fnames,
                     const std::vector<double>&      fvalues,
                     const std::shared_ptr<ekat::logger::LoggerBase>& logger)
{
  EKAT_REQUIRE_MSG (fnames.size()==fvalues.size(),
      "Error! Inconsistent sizes for constant field initialization arrays.\n"
      " - num fnames : " + std::to_string(fnames.size()) + "\n"
      " - num fvalues: " + std::to_string(fvalues.size()) + "\n");

  logger->info("    [EAMxx] Initializing constant fields ...");

  // Helper lambda, to init all copies of a field (on all grids mgrs)
  auto init_constant_field = [&] (const std::string& name, double val, int icmp = -1) {
    Field* f = nullptr;
    for (auto& it : m_ic_fields) {
      if (it.name()==name) {
        f = &it;
        break;
      }
    }
    EKAT_REQUIRE_MSG (f!=nullptr,
        "Error! Attempt to prescribe constant value for a field that is not an atm input.\n"
        " - field name: " + name + "\n");

    if (icmp>=0) {
      const auto& fl = f->get_header().get_identifier().get_layout();
      EKAT_REQUIRE_MSG (fl.is_vector_layout(),
          "Error! Attempt to access vector component of a non-vector field.\n"
          "  - field name: " + name + "\n"
          "  - field layout: " + to_string(fl) + "\n"
          "  - requested comp: " + std::to_string(icmp) + "\n");
      f->get_component(icmp).deep_copy(val);
    } else {
      f->deep_copy(val);
    }
    f->get_header().get_tracking().update_time_stamp(m_t0);
  };

  // Loop over provided fnames, and call the lambda above
  for (size_t i=0; i<fnames.size(); ++i) {
    logger->info("      " + fnames[i] + " = " + std::to_string(fvalues[i]));
    auto pos = fnames[i].rfind("_cmp_");
    if (pos==std::string::npos) {
      // Not a component of a field
      init_constant_field(fnames[i],fvalues[i]);
    } else {
      // Looks like the component of a vector field.
      auto tail = fnames[i].substr(pos+5);
      EKAT_REQUIRE_MSG (tail.size()>0,
          "Error! Bad name specification for constant field initialization.\n"
          " - input name: " + fnames[i] + "\n"
          "The presence of '_cmp_' suggests this is a vector field, but no index was found after '_cmp'\n");
      try {
        int cmp = std::stoi (tail);
        init_constant_field(fnames[i],fvalues[i],cmp);
      } catch (std::exception&) {
        EKAT_ERROR_MSG (
          "Error! Bad name specification for constant field initialization.\n"
          " - input name: " + fnames[i] + "\n"
          "The presence of '_cmp_' suggests this is a vector field, but '" + tail + "' could not be parsed as an index\n");
      }
    }
  }
  logger->info("    [EAMxx] Initializing constant fields ... done!");
}

void ModelInit::
separate_inputs (const std::vector<Field>& eamxx_inputs)
{
  // Grab fields that are required by eamxx as a whole
  for (const auto& f : eamxx_inputs) {
    const auto& fid = f.get_header().get_identifier();
    const auto& gname = fid.get_grid_name();

    if (fid.name()=="phis" or fid.name()=="sgh30") {
      // Topography fields are set in a separate vector
      // Also, keep track of the field name in the topo file vs eamxx
      m_topo_fields.push_back(f);
      if (fid.name()=="phis") {
        m_topo_fields_names_file[gname].push_back("phis");
        if (gname=="Physics PG2") {
          m_topo_fields_names_file[gname].push_back("PHIS");
        } else {
          m_topo_fields_names_file[gname].push_back("PHIS_d");
        }
      } else {
        EKAT_ASSERT_MSG(gname == "Physics PG2",
            "Error! Requesting sgh30 field on " + gname +
            ", but topo file only has sgh30 for Physics PG2.\n");
        m_topo_fields_names_file[gname].push_back("SGH30");
        m_topo_fields_names_eamxx[gname].push_back("sgh30");
      }
    } else if (m_ic_grid->is_valid_alias(gname)) {
      // We consider the field as an input only if it's on the IC grid
      m_ic_fields.push_back(f);
    } else {
      EKAT_ERROR_MSG (
          "Error! Unexpected grid for non-topography-related eamxx input field.\n"
          " - field name: " + fid.name() + "\n"
          " - field grid: " + gname + "\n"
          " - ic grid aliases: " + ekat::join(m_ic_grid->aliases(),",") + "\n");
    }
  };
}

void ModelInit::
gather_ic_file_fields (const std::string& filename)
{
  using namespace ShortFieldTagsNames;

  auto grid = get_ic_grid();

  scorpio::register_file(filename,scorpio::Read);

  for (auto& f : m_ic_fields) {
    const auto& fh = f.get_header();
    const auto& fid = fh.get_identifier();

    // If field has a parent, and parent is added, no need to add this field
    auto p = fh.get_parent().lock();
    if (p) {
      const auto& pname = p->get_identifier().name();
      bool skip_f = false;
      for (const auto& it : m_ic_fields_in_file) {
        if (it.name()==pname) {
          skip_f = true;
          break;
        }
      }
      if (skip_f) continue;
    }

    // If field has children, remove any children we already added
    for (const auto& c: fh.get_children()) {
      const auto& cname = c.lock()->get_identifier().name();
      for (auto& it : m_ic_fields_in_file) {
        if (it.name()==cname) {
          // We previously added this children. Remove it by
          // swapping with last elem, and popping last elem
          std::swap(it,m_ic_fields_in_file.back());
          m_ic_fields_in_file.pop_back();
          break;
        }   
      }   
    }

    // Get the *global* layout
    auto gl = grid->get_global_layout(fid.get_layout());
    std::vector<std::string> dimnames (gl.rank());
    std::vector<int> dims = gl.dims();
    for (int i=0; i<gl.rank(); ++i) {
      const auto t = gl.tags()[i];
      dimnames[i] = grid->get_dim_name(t);
      if (t==CMP) {
        dimnames[i] += std::to_string(dims[i]);
      }
    }

    if (scorpio::has_variable(filename,f.name(),dimnames,gl.dims())) {
      m_ic_fields_in_file.push_back(f);
    }
  };

  // Store list of names of fields present in IC file
  for (const auto& f : m_ic_fields_in_file) {
    m_ic_fields_in_file_names.push_back(f.name());
  }

  scorpio::eam_pio_closefile(filename);
}

} // namespace scream
