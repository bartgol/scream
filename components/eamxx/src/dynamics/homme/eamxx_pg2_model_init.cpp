#include "eamxx_pg2_model_init.hpp"

#include "eamxx_homme_fv_phys_helper.hpp"

namespace scream
{
Pg2ModelInit::
Pg2ModelInit (const std::vector<Field>& eamxx_inputs,
              const GridsManager& gm,
              const util::TimeStamp& t0)
 : ModelInit (eamxx_inputs,gm.get_grid("Physics GLL"),t0)
{
  // Nothing to do here
}

void DefaultModelInit::
read_ic_file (const std::string& filename,
              const std::shared_ptr<ekat::logger::LoggerBase>& logger)
{
  using namespace ShortFieldTagsNames;

  auto& fv_phys = HommeFvPhysHelper::instance();

  gather_ic_file_fields (filename);

  // These fields must be inited (either via file or constant) in order
  // to init both phys and dyn states
  strvec_t required  = {"T_mid","horiz_winds","ps","phis","omega","pseudo_density"};

  const int ncols_gll = fv_phys.m_cgll_grid->get_num_local_dofs();
  const int gname_gll = fv_phys.m_cgll_grid->name();

  strmap_t<Field> gll_fields;
  std::map<int,FieldLayout> gll_layouts = {
    { 1, {FieldLayout ({COL        },{ncols_gll        } } },
    { 2, {FieldLayout ({COL,    LEV},{ncols_gll,  nlevs} } },
    { 3, {FieldLayout ({COL,CMP,LEV},{ncols_gll,2,nlevs} } }
  };

  for (const auto& n : required) {
    auto it_const = ekat::find(m_constant_fields,n);
    auto it_file  = ekat::find(m_ic_fields_in_file,n);
    EKAT_REQUIRE_MSG (ic_const!=m_constant_fields.end() or ic_file!=m_ic_fields_in_file.end(),
      "Error! Field '" + n + "' was not initialized (either via ic file or constant value).\n");
    const auto& f = it_const==m_constant_fields.end() ? *it_file : *it_const;
  }
  for (const std::vector<Field>& vec : {m_constant_fields, m_ic_fields_in_file}) {
    for (const auto& f : m_constant_fields) {
      if (ekat::contains(required,f.name())) {
        const auto& units = f.get_header().get_identifier().get_units();
        FieldIdentifier fid (f.name(),gll_layouts[f.rank()],units,gname_gll);
        auto it_bool = gll_fields.emplace(f.name(),fid);
        auto& gll_f = it_bool.first;
        gll_f.allocate_view();
      }
    }
  }

  for (const auto& n : required) {
    EKAT_REQUIRE_MSG (gll_fields.count(n)==1,
        "Error! Field '" + n + "' was not initialized (either via ic file or constant value).\n");
  }

  if (m_ic_fields_in_file.size()>0) {

    // Load fields
    AtmosphereInput ic_reader (filename,get_ic_grid(),m_ic_fields_in_file);
    ic_reader.set_logger(logger);
    ic_reader.read_variables();
    ic_reader.finalize();
  }

  for (auto& f : m_ic_fields_in_file) {
    f.get_header().get_tracking().update_time_stamp(m_t0);
  }

  // In order to run the FvPhys remap, we need to have all the dyn state
  // inited on the GLL grid
  for (const auto& f : m_ic_fields) {
    auto it = ekat::find(missing,f.name());
    if (it!=missing.end()) {
      if (f.name()=="T_mid") {
        fv_phys.m_T_phys
      } else if (f.name()==
      missing.erase(it);
    }
  }
  if (missing.size()>0) {
    for (const auto& f : 
  }
}


} // namespace scream
