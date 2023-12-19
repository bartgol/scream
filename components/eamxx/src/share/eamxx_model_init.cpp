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
set_constant_fields (const std::vector<std::string>& const_fields,
                     const std::shared_ptr<ekat::logger::LoggerBase>& logger)
{
  logger->info("    [EAMxx] Initializing constant fields ...");

  auto str2double = [&] (const std::string& fname, const std::string& val_str) {
    double val;
    try {
      val = std::stod (val_str);
    } catch (std::exception&) {
      EKAT_ERROR_MSG (
        "Error! Bad value specification for constant field initialization.\n"
        " Field name: " + fname + "\n"
        "Could not convert the string '" + val_str + "' to double.\n");
    }
    return val;
  };


  // Loop over provided fnames, and call the lambda above
  for (const auto& s : const_fields) {
    logger->info("      " + s);

    // Split input string FNAME = VALUE(S)
    auto tokens = ekat::split(s,"=");
    EKAT_REQUIRE_MSG (tokens.size()==2,
        "Error! Bad syntax specifying constant input fields.\n"
        "  Valid syntax: 'field_name = field_value(s)'\n"
        "  Input string: '" + s + "'\n");
    const auto fname   = ekat::trim(tokens[0]);
    const auto val_str = ekat::trim(tokens[1]);
    EKAT_REQUIRE_MSG (fname.size()>0,
        "Error! Bad syntax specifying constant input fields (empty field name).\n"
        " Input string: '" + s + "'\n");
    EKAT_REQUIRE_MSG (val_str.size()>0,
        "Error! Bad syntax specifying constant input fields (empty value).\n"
        " Input string: '" + s + "'\n");

    // Get the field, and ensure it's in the ic fields list
    Field* f = nullptr;
    for (auto& it : m_ic_fields) {
      if (it.name()==fname) {
        f = &it;
        break;
      }
    }
    auto get_fname = [&] (const Field& f) {
      return f.name();
    };
    EKAT_REQUIRE_MSG (f!=nullptr,
        "Error! Attempt to prescribe constant value for a field that is not an atm input.\n"
        " - field name: " + fname + "\n"
        " - atm inputs: " + ekat::join(m_ic_fields,get_fname,",") + "\n");

    // Check if we are attempting to init a vec field with f=(v1,v2,...,vN) syntax
    bool vector_values = val_str.front()=='(' and val_str.back()==')';
    if (vector_values) {
      const auto& fl = f->get_header().get_identifier().get_layout();
      EKAT_REQUIRE_MSG (fl.is_vector_layout(),
          "Error! Attempt to assing a vector of values to a non-vector field.\n"
          "  - field name: " + fname + "\n"
          "  - field layout: " + to_string(fl) + "\n"
          "  - input string: " + s + "\n");
      const auto vals = ekat::split(val_str.substr(1,val_str.size()-2),",");
      const int vec_dim = fl.get_vector_dim();
      EKAT_REQUIRE_MSG (static_cast<int>(vals.size())==fl.dim(vec_dim),
          "Error! Vector of values does not match vector field extent.\n"
          "  - field name: " + fname + "\n"
          "  - field layout: " + to_string(fl) + "\n"
          "  - input string: " + s + "\n");
      for (int icmp=0; icmp<fl.dim(vec_dim); ++icmp) {
        auto f_comp = f->subfield(vec_dim,icmp);
        auto val = str2double(fname,vals[icmp]);
        f_comp.deep_copy(val);
      }
    } else {
      // Not a vector assignment, so just init the whole field
      // NOTE: if user had a typo, like "f=(v1,v2", notice missing ')',
      //       this fcn will throw, cause "(v1,v2" cannot be converted to double
      auto val = str2double(fname,val_str);
      f->deep_copy(val);
    }
    
    f->get_header().get_tracking().update_time_stamp(m_t0);

    // Remove this field from the list of ic_fields and topo_fields
    for (auto it = m_ic_fields.begin(); it!=m_ic_fields.end(); ++it) {
      if (it->name()==fname) {
        m_constant_fields.push_back(*it);
        m_ic_fields.erase(it);
        break;
      }
    }
    for (auto& it : m_topo_fields_names_eamxx) {
      if (ekat::contains(it.second,fname)) {
        auto it2  = ekat::find(it.second,fname);
        auto pos = std::distance(it.second.begin(),it2);
        it.second.erase(it2);
        auto& file_names = m_topo_fields_names_file[it.first];
        file_names.erase(file_names.begin()+pos);
        m_constant_fields.push_back(m_topo_fields.begin()+pos);
        m_topo_fields.erase(m_topo_fields.begin()+pos);
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
        m_topo_fields_names_eamxx[gname].push_back("phis");
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
  if (m_ic_fields.empty()) {
    return;
  }

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
