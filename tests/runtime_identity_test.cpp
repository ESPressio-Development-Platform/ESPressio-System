#include "ESPressio_RuntimeIdentity.hpp"
#include <cassert>
#include <limits>
#include <type_traits>
using namespace ESPressio::System;
int main() {
 static_assert(sizeof(RuntimeIncarnationId)==4 && sizeof(DeviceRuntimeIdentity)==20);
 static_assert(std::is_trivially_copyable_v<DeviceRuntimeIdentity>);
 static_assert(!bool(RuntimeIncarnationId{}));
 static_assert(bool(RuntimeIncarnationId{std::numeric_limits<std::uint32_t>::max()}));
 DeviceIdentifier::Storage bytes{}; bytes[0]=9; bytes[15]=7;
 DeviceRuntimeIdentity candidate{DeviceIdentifier{bytes},RuntimeIncarnationId{42}};
 assert(!RuntimeIdentity::IsInstalled() && !RuntimeIdentity::TryGet());
 DeviceRuntimeIdentity output=candidate;
 assert(!RuntimeIdentity::TryRead(output) && output==candidate);
 assert(RuntimeIdentity::Install({})==RuntimeIdentity::InstallationStatus::InvalidIdentity);
 assert(RuntimeIdentity::Install({candidate.Device,{}})==RuntimeIdentity::InstallationStatus::InvalidIdentity);
 assert(!RuntimeIdentity::IsInstalled());
 assert(RuntimeIdentity::Install(candidate)==RuntimeIdentity::InstallationStatus::Success);
 const auto* installed=RuntimeIdentity::TryGet();
 assert(installed && *installed==candidate);
 assert(RuntimeIdentity::TryRead(output) && output==candidate);
 assert(RuntimeIdentity::Install(candidate)==RuntimeIdentity::InstallationStatus::AlreadyInstalled);
 candidate.Incarnation=RuntimeIncarnationId{43};
 assert(RuntimeIdentity::Install(candidate)==RuntimeIdentity::InstallationStatus::AlreadyInstalled);
 assert(installed==RuntimeIdentity::TryGet() && installed->Incarnation.Value()==42);
}
