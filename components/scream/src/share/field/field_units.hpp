#ifndef SCREAM_FIELD_UNITS_HPP
#define SCREAM_FIELD_UNITS_HPP

#include <string>
#include <sstream>
#include <boost/units/systems/si.hpp>
#include <boost/units/io.hpp>

namespace scream {

template<typename T>
struct MyDebug {};

namespace units {

namespace SI = boost::units::si;
using MKS = SI::system;

// Dimensionless unit
const auto one = SI::si_dimensionless;

// Short names for fundamental units
const auto m = SI::meter;
const auto s = SI::second;
const auto kg = SI::kilogram;
const auto K = SI::kelvin;
const auto A = SI::ampere;
const auto mol = SI::mole;
const auto cd = SI::candela;

// Short names for scaled versions (kilo, mega, milli,...),
// and derived quantities

// Length
const auto mm = 1e-3*m;
const auto km = 1e3*m;

// Time
const auto day = 86400*s;
const auto year = 365*day;

// Mass
const auto g = 1e-3*kg;

// Force
const auto N = kg*m/(s*s);
const auto dyn = 1e-5*N;

// Pressure
const auto Pa = N/(m*m);
const auto hPa = 1e2*Pa;
const auto kPa = 1e3*Pa;
const auto bar = 1e5*Pa;
const auto MPa = 1e6*Pa;
const auto atm = 101325*Pa;

// Energy
const auto J = N*m;
const auto kJ = 1e3*J;
const auto MJ = 1e6*J;

// Power
const auto W = J/s;
const auto kW = 1e3*W;
const auto MW = 1e6*W;

// Electro-magnetic
const auto C = A*s;     // Coulomb
const auto V = J/C;     // Volt
const auto T = N/(A*m); // Tesla
const auto Sv = J/kg;   // Sievert
const auto rem = 0.01*Sv;

template<typename BoostUnit>
std::string get_units (const BoostUnit& u) {
  std::ostringstream os;
  boost::units::set_format(os,boost::units::raw_fmt);
  os << u;
  return os.str();
}

template<typename BoostUnit>
std::string get_units () {
  static BoostUnit u;
  return get_units(u);
}

} // namespace units

} // namespace scream

#endif // SCREAM_FIELD_UNITS_HPP
