#include <ESPressio_Execution.hpp>
#include <cassert>
using namespace ESPressio::System;
int main() {
    Execution::NullExecutionProvider provider;
    const auto result=provider.CreateJoinable([](void*){},nullptr,{});
    assert(!result && result.Result.Status==PlatformStatus::Unsupported);
    assert(result.Handle==Execution::InvalidExecutionHandle);
    assert(provider.Join(1).Status==PlatformStatus::Unsupported);
}
