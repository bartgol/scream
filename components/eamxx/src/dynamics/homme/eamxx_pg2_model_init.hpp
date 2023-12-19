#ifndef EAMXX_PG2_MODEL_INIT_HPP
#define EAMXX_PG2_MODEL_INIT_HPP

#include "share/eamxx_model_init.hpp"

namespace scream
{

class Pg2ModelInit : public ModelInit
{
public:
  Pg2ModelInit (const std::vector<Field>& eamxx_inputs,
                const GridsManager& gm,
                const util::TimeStamp& t0);

  ~Pg2ModelInit () = default;

  void read_ic_file (const std::string& filename,
                     const std::shared_ptr<ekat::logger::LoggerBase>& logger) override;

  void read_topo_file (const std::string& filename,
                       const std::shared_ptr<ekat::logger::LoggerBase>& logger) override;
};

} // namespace scream

#endif // EAMXX_PG2_MODEL_INIT_HPP
