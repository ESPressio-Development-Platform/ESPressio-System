#include <cassert>
#include <cstdint>
#include <memory>

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

    uint32_t ProcessorCount() const noexcept override { return 2; }

    void SleepMilliseconds(uint32_t) override {}
    void Yield() override {}
    bool SupportsProcessorAffinity() const noexcept override { return true; }
};

class TestSignal final : public ESPressio::System::Synchronization::ISignal {
private:
    bool _set = false;

public:
    explicit TestSignal(bool initiallySet) : _set(initiallySet) {}

    ESPressio::System::PlatformResult Give() noexcept override {
        _set = true;
        return ESPressio::System::PlatformResult::Succeeded();
    }

    ESPressio::System::PlatformResult GiveFromInterrupt() noexcept override {
        _set = true;
        return ESPressio::System::PlatformResult::Succeeded();
    }

    ESPressio::System::PlatformResult Wait(uint32_t) noexcept override {
        if (!_set) {
            return ESPressio::System::PlatformResult::Failed(
                ESPressio::System::PlatformStatus::Timeout
            );
        }
        _set = false;
        return ESPressio::System::PlatformResult::Succeeded();
    }

    ESPressio::System::PlatformResult Reset() noexcept override {
        _set = false;
        return ESPressio::System::PlatformResult::Succeeded();
    }
};

class TestSynchronizationProvider final
    : public ESPressio::System::Synchronization::ISynchronizationProvider {
public:
    uint32_t Created = 0;

    std::unique_ptr<ESPressio::System::Synchronization::ISignal>
    CreateBinarySignal(bool initiallySet = false) override {
        ++Created;
        return std::make_unique<TestSignal>(initiallySet);
    }
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
    assert(Execution::Provider().ProcessorCount() == 2);

    const auto now = Clock::Monotonic().NowNanoseconds();
    const auto later = Clock::Monotonic().NowNanoseconds();
    assert(later >= now);
    assert(Clock::Monotonic().ResolutionNanoseconds() >= 1);

    GPIO::InterruptCreationResult unsupported(
        PlatformResult::Failed(PlatformStatus::Unsupported)
    );
    assert(!unsupported);
    assert(unsupported.Result.Status == PlatformStatus::Unsupported);

    // Reproduce the static-construction ordering used by globally constructed
    // Threads: create the signal before a concrete provider exists, then bind
    // and use it after the platform provider is installed.
    Synchronization::ResetProvider();
    auto deferredSignal = Synchronization::CreateBinarySignal();
    assert(deferredSignal != nullptr);

    TestSynchronizationProvider synchronization;
    Synchronization::SetProvider(&synchronization);
    assert(deferredSignal->Reset());
    assert(synchronization.Created == 1);
    assert(deferredSignal->Give());
    assert(deferredSignal->Wait(0));
    assert(synchronization.Created == 1);
    Synchronization::ResetProvider();

    Execution::ResetProvider();
    assert(!Execution::Provider().SupportsProcessorAffinity());
    assert(Execution::Provider().ProcessorCount() == 1);

    return 0;
}
