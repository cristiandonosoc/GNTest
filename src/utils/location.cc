// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/utils/location.h"

#include <assert.h>

#include <stdio.h>

#include <utility>
#include <vector>

namespace warhol {

namespace {

std::vector<Location>& GetLocations() {
  thread_local std::vector<Location> locations;
  return locations;
}

}  // namespace

Location Location::GetThreadCurrentLocation(const Location& call_site) {
  auto& locations = GetLocations();
  if (locations.empty())
    return call_site;
  return locations.back();
}

LocationTrigger::LocationTrigger(Location location)
    : location_(std::move(location)) {
  GetLocations().push_back(location_);
  printf("%s\n", __PRETTY_FUNCTION__);
}

LocationTrigger::~LocationTrigger() {
  auto& locations = GetLocations();
  assert(!locations.empty());
  locations.pop_back();
  printf("%s\n", __PRETTY_FUNCTION__);
}

}  // namespace warhol
