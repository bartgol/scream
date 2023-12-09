#ifndef EAMXX_DEFAULT_MODEL_INIT_HPP
#define EAMXX_DEFAULT_MODEL_INIT_HPP

#include "share/eamxx_model_init.hpp"

namespace scream
{

// Interface class for initializing fields at model init time
// Derived classes can implement whatever strategy is needed
// For instance, they could simply look for them in the IC file
// An alternative (which is the case for PG2) is to read a version
// of the fields on a different grid, and then remap to the actual
// fields that were requested

class DefaultModelInit : public ModelInit {
public:
  DefaultModelInit (const std::vector<Field>& eamxx_inputs,
                    const GridsManager& gm,
                    const util::TimeStamp& t0);

  ~DefaultModelInit () = default;

protected:

  void read_ic_file (const std::string& filename,
                     const std::shared_ptr<ekat::logger::LoggerBase>& logger) override;

  void read_topo_file (const std::string& filename,
                       const std::shared_ptr<ekat::logger::LoggerBase>& logger) override;
};

inline std::shared_ptr<ModelInit>
create_default_model_init (const std::vector<Field>& eamxx_inputs,
                           const GridsManager& gm,
                           const util::TimeStamp& t0)
{
  return std::make_shared<DefaultModelInit>(eamxx_inputs,gm,t0);
}

inline void register_default_model_init () {
  ModelInitFactory::instance().register_product("default",&create_default_model_init);
}

} // namespace scream

#endif // EAMXX_DEFAULT_MODEL_INIT_HPP
