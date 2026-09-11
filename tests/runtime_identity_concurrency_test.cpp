#include "ESPressio_RuntimeIdentity.hpp"
#include <array>
#include <atomic>
#include <cassert>
#include <thread>
using namespace ESPressio::System;
int main() {
 std::atomic<bool> start{false}; std::atomic<int> wins{0};
 std::array<std::thread,8> installers;
 for(unsigned i=0;i<installers.size();++i) installers[i]=std::thread([&,i]{
  while(!start.load(std::memory_order_acquire)) std::this_thread::yield();
  DeviceIdentifier::Storage bytes{}; bytes.fill(static_cast<unsigned char>(i+1));
  if(RuntimeIdentity::Install({DeviceIdentifier{bytes},RuntimeIncarnationId{i+1}})==RuntimeIdentity::InstallationStatus::Success) ++wins;
 });
 std::thread reader([&]{
  while(!start.load(std::memory_order_acquire)) std::this_thread::yield();
  for(unsigned i=0;i<100000;++i) if(const auto* identity=RuntimeIdentity::TryGet()) {
   for(const auto byte:identity->Device.Bytes()) assert(byte==identity->Incarnation.Value());
  }
 });
 start.store(true,std::memory_order_release);
 for (auto& t : installers) { t.join(); }
 reader.join();
 assert(wins==1 && RuntimeIdentity::IsInstalled());
}
