#include <cassert>
#include <cstdint>

#include <ESPressio_System.hpp>

namespace {

class TestExecutionProvider final : public ESPressio::System::Execution::IExecutionProvider {
public:
    ESPressio::System::Execution::ExecutionCreationResult Create(
        ESPressio::System::Execution::ExecutionEntry entry,
        void*,
        const ESPressio::System::Execution::ExecutionConfiguration& configuration
    ) override {
        if (entry == nullptr || configuration.StackSizeBytes == 0) {
            return {
                ESPressio::System::PlatformResult::Failed(
                    ESPressio::System::PlatformStatus::InvalidArgument
                ),
                ESPressio::System::Execution::InvalidExecutionHandle
            };
        }
        return {ESPressio::System::PlatformResult::Succeeded(), 42};
    }

    ESPressio::System::PlatformResult Destroy(
        ESPressio::System::Execution::ExecutionHandle
    ) override { return ESPressio::System::PlatformResult::Succeeded(); }

    ESPressio::System::PlatformResult Suspend(
        ESPressio::System::Execution::ExecutionHandle
    ) override { return ESPressio::System::PlatformResult::Succeeded(); }

    ESPressio::System::PlatformResult Resume(
        ESPressio::System::Execution::ExecutionHandle
    ) override { return ESPressio::System::PlatformResult::Succeeded(); }

    ESPressio::System::Execution::ExecutionHandle Current() const noexcept override {
        return 42;
    }

    uint32_t MinimumFreeStackBytes(
        ESPressio::System::Execution::ExecutionHandle
    ) const noexcept override { return 2048; }

    void SleepMilliseconds(uint32_t) override {}
    void Yield() override {}
    bool SupportsProcessorAffinity() const noexcept override { return true; }
};

void Entry(void*) {}

}

int main() {
    using namespace ESPressio::System;

    const auto any = ProcessorAffinity::Any();
    const auto coreOne = ProcessorAffinity::Specific(1);
    assert(!any.IsSpecific());
    assert(coreOne.IsSpecific());
    assert(coreOne.Processor == 1);

    TestExecutionProvider execution;
    Execution::SetProvider(&execution);
    Execution::ExecutionConfiguration configuration;
    configuration.Affinity = coreOne;
    const auto created = Execution::Provider().Create(&Entry, nullptr, configuration);
    assert(created);
    assert(created.Handle == 42);
    assert(Execution::Provider().MinimumFreeStackBytes(created.Handle) == 2048);

    const auto now = Clock::Monotonic().NowNanoseconds();
    const auto later = Clock::Monotonic().NowNanoseconds();
    assert(later >= now);
    assert(Clock::Monotonic().ResolutionNanoseconds() >= 1);

    GPIO::InterruptCreationResult unsupported(
        PlatformResult::Failed(PlatformStatus::Unsupported)
    );
    assert(!unsupported);
    assert(unsupported.Result.Status == PlatformStatus::Unsupported);

    Execution::ResetProvider();
    assert(!Execution::Provider().SupportsProcessorAffinity());

    return 0;
}
