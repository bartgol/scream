#include "eamxx_default_model_init.hpp"

#include "share/io/scorpio_input.hpp"

namespace scream
{

DefaultModelInit::
DefaultModelInit (const std::vector<Field>& eamxx_inputs,
                  const GridsManager& gm,
                  const util::TimeStamp& t0)
 : ModelInit (eamxx_inputs,gm.get_grid("Physics"),t0)
{
  // Nothing to do here
}

void DefaultModelInit::
read_ic_file (const std::string& filename,
              const std::shared_ptr<ekat::logger::LoggerBase>& logger)
{
  gather_ic_file_fields (filename);

  if (m_ic_fields_in_file.size()==0) {
    return;
  }

  // Load fields
  AtmosphereInput ic_reader (filename,get_ic_grid(),m_ic_fields_in_file);
  ic_reader.set_logger(logger);
  ic_reader.read_variables();
  ic_reader.finalize();

  for (auto& f : m_ic_fields_in_file) {
    f.get_header().get_tracking().update_time_stamp(m_t0);
  }
}

void DefaultModelInit::
read_topo_file (const std::string& filename,
                const std::shared_ptr<ekat::logger::LoggerBase>& logger)
{
  using namespace ShortFieldTagsNames;

  std::vector<Field> fields;
  auto grid = m_ic_grid->clone("Physics",true);
  grid->reset_field_tag_name(COL,"ncol_d");
  for (auto n : m_ic_grid->aliases()) {
    grid->add_alias(n);
  }

  for (const auto& f : m_topo_fields) {
    if (f.name()=="phis") {
      fields.push_back(f.alias("PHIS_d"));
    } else if (f.name()=="sgh30") {
      EKAT_ERROR_MSG(
        "[DefaultModelInit] Error! Requesting sgh30 field from topo file.\n"
        "  This model init class does not handle Physics PG2 grid,\n"
        "  but topo file only has sgh30 for Physics PG2.\n");
    } else {
      EKAT_ERROR_MSG(
        "[DefaultModelInit] Error! Unexpected/unrecognized topo field name.\n"
        " - field name: " + f.name() + "\n");
    }
  }

  // Load fields
  AtmosphereInput ic_reader (filename,grid,fields);
  ic_reader.set_logger(logger);
  ic_reader.read_variables();
  ic_reader.finalize();

  for (auto& f : fields) {
    f.get_header().get_tracking().update_time_stamp(m_t0);
  }
}

} // namespace scream
