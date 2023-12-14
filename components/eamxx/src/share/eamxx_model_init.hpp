#ifndef EAMXX_MODEL_INIT_HPP
#define EAMXX_MODEL_INIT_HPP

#include "share/field/field.hpp"
#include "share/grid/grids_manager.hpp"
#include "share/util/scream_time_stamp.hpp"
#include "share/scream_types.hpp"

#include <ekat/logging/ekat_logger.hpp>
#include <ekat/util/ekat_factory.hpp>

#include <vector>

namespace scream
{

// Interface class for initializing fields at model init time
// Derived classes can implement whatever strategy is needed
// For instance, they could simply look for them in the IC file
// An alternative (which is the case for PG2) is to read a version
// of the fields on a different grid, and then remap to the actual
// fields that were requested

class ModelInit {
public:
  template<typename T>
  using strmap_t = std::map<std::string,T>;

  using strvec_t = std::vector<std::string>;

  ModelInit (const std::vector<Field>& eamxx_inputs,
             const std::shared_ptr<const AbstractGrid>& ic_grid,
             const util::TimeStamp& t0);

  virtual ~ModelInit () = default;

  std::shared_ptr<const AbstractGrid> get_ic_grid () const { return m_ic_grid; }

  const strmap_t<strvec_t>& get_topo_fields_names_file () const {
    return m_topo_fields_names_file;
  }
  const strmap_t<strvec_t>& get_topo_fields_names_eamxx () const {
    return m_topo_fields_names_eamxx;
  }
  const strvec_t& get_ic_fields_in_file_names () const {
    return m_ic_fields_in_file_names;
  }

  virtual void read_ic_file (const std::string& filename,
                             const std::shared_ptr<ekat::logger::LoggerBase>& logger) = 0;

  virtual void read_topo_file (const std::string& filename,
                               const std::shared_ptr<ekat::logger::LoggerBase>& logger) = 0;

  void set_constant_fields (const std::vector<std::string>& const_fields,
                            const std::shared_ptr<ekat::logger::LoggerBase>& logger);

protected:

  // Separate inputs into topo fields and IC fields
  void separate_inputs (const std::vector<Field>& eamxx_inputs);

  // Gather fields that are actually in IC file
  void gather_ic_file_fields (const std::string& filename);

  // Gather fields that are topography related
  void gather_topo_fields_names ();

  util::TimeStamp     m_t0;

  strmap_t<strvec_t>  m_topo_fields_names_file;
  strmap_t<strvec_t>  m_topo_fields_names_eamxx;
  strvec_t            m_ic_fields_in_file_names;

  // Subset of EAMxx input fields, containing fields that are topography-related
  std::vector<Field>  m_topo_fields;

  // Subset of EAMxx input fields, containing fields that are not topography-related
  std::vector<Field>  m_ic_fields;

  // Subset of m_ic_fields, containing fields that are present in the IC file
  std::vector<Field>  m_ic_fields_in_file;

  // Grid where IC conditions will be read
  std::shared_ptr<const AbstractGrid> m_ic_grid;
};

using ModelInitFactory =
    ekat::Factory<ModelInit,
                  ekat::CaseInsensitiveString,
                  std::shared_ptr<ModelInit>,
                  const std::vector<Field>&,
                  const GridsManager&,
                  const util::TimeStamp&>;

} // namespace scream

#endif // EAMXX_MODEL_INIT_HPP
