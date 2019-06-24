#include <catch2/catch.hpp>

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include "share/scream_types.hpp"
#include "share/scream_pack.hpp"
#include "share/quantity.hpp"

TEST_CASE("quantity", "") {
  using namespace scream;
  using namespace boost::units;

  using Time = quantity<si::time,Real>;

  pack::Pack<Time,8> a(1.0);
  pack::Pack<Time,8> b(2.0);

  auto c = a+b;

  for (int i=0; i<8; ++i) {
    REQUIRE (c[i].value()==3.0);
  }
}
