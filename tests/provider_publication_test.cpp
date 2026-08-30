#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>

#include "ESPressio_Execution.hpp"
#include "ESPressio_GPIO.hpp"
#include "ESPressio_Queue.hpp"
#include "ESPressio_Synchronization.hpp"
#include "ESPressio_SystemPlatformClock.hpp"
#include "ESPressio_SystemPlatformEntropy.hpp"

using namespace ESPressio::System;

namespace {

class TestExecutionProvider final : public Execution::IExecutionProvider {
public:
    Execution::ExecutionCreationResult Create(
        Execution::ExecutionEntry,
        void*,
        const Execution::ExecutionConfiguration&
    ) override { return {}; }
    PlatformResult Destroy(Execution::ExecutionHandle) override { return PlatformResult::Success(); }
    PlatformResult Suspend(Execution::ExecutionHandle) override { return PlatformResult::Success(); }
    PlatformResult Resume(Execution::ExecutionHandle) override { return PlatformResult::Success(); }
    Execution::ExecutionHandle Current() const noexcept override { return 1; }
    uint32_t MinimumFreeStackBytes(Execution::ExecutionHandle) const noexcept override { return 0; }
    uint32_t ProcessorCount() const noexcept override { return 1; }
    void SleepMilliseconds(uint32_t) override {}
    void Yield() override {}
    bool SupportsProcessorAffinity() const noexcept override { return true; }
};

class TestQueueProvider final : public Queue::IQueueProvider {
public:
    std::unique_ptr<Queue::IMessageQueue> Create(std::size_t, std::size_t) override {
        return {};
    }
};

class TestSynchronizationProvider final : public Synchronization::ISynchronizationProvider {
public:
    std::unique_ptr<Synchronization::ISignal> CreateBinarySignal(bool) override {
        return {};
    }
};

class TestGPIOController final : public GPIO::IController {
public:
    PlatformResult Configure(GPIO::Pin, const GPIO::PinConfiguration&) noexcept override {
        return PlatformResult::Success();
    }
    PlatformResult Write(GPIO::Pin, GPIO::State) noexcept override {
        return PlatformResult::Success();
    }
    PlatformResult Read(GPIO::Pin, GPIO::State& state) const noexcept override {
        state = GPIO::State::Low;
        return PlatformResult::Success();
    }
    GPIO::InterruptCreationResult CreateInterrupt(
        GPIO::Pin,
        const GPIO::InterruptConfiguration&,
        GPIO::InterruptCallback,
        void*
    ) override { return {}; }
    bool SupportsInterrupts() const noexcept override { return false; }
    bool SupportsInterruptAffinity() const noexcept override { return false; }
};

class TestMonotonicClock final : public Clock::IMonotonicClock {
public:
    uint64_t NowNanoseconds() const noexcept override { return 42; }
    uint64_t ResolutionNanoseconds() const noexcept override { return 1; }
    bool IsInterruptSafe() const noexcept override { return true; }
};

class TestHighResolutionProvider final : public Clock::IHighResolutionCounterProvider {
public:
    std::unique_ptr<Clock::IHighResolutionCounter> Create(uint64_t) override {
        return {};
    }
};

class TestEntropySource final : public Entropy::IEntropySource {
public:
    PlatformResult Fill(void*, std::size_t) noexcept override {
        return PlatformResult::Success();
    }
    bool IsCryptographicallySuitable() const noexcept override { return true; }
};

void StressProviderPublication() {
    TestExecutionProvider execution;
    TestQueueProvider queue;
    TestSynchronizationProvider synchronization;
    TestGPIOController gpio;
    TestMonotonicClock clock;
    TestHighResolutionProvider highResolution;
    TestEntropySource entropy;

    std::atomic<bool> start{false};
    std::atomic<bool> done{false};

    std::thread reader([&] {
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        while (!done.load(std::memory_order_acquire)) {
            auto* queueProvider = Queue::Provider();
            assert(queueProvider == nullptr || queueProvider == &queue);

            auto* synchronizationProvider = Synchronization::Provider();
            assert(
                synchronizationProvider == nullptr ||
                synchronizationProvider == &synchronization
            );

            auto* controller = GPIO::Controller();
            assert(controller == nullptr || controller == &gpio);

            auto* highResolutionProvider = Clock::HighResolutionProvider();
            assert(
                highResolutionProvider == nullptr ||
                highResolutionProvider == &highResolution
            );

            auto& activeExecution = Execution::Provider();
            assert(
                &activeExecution == &execution ||
                dynamic_cast<Execution::NullExecutionProvider*>(&activeExecution) != nullptr
            );

            auto& monotonic = Clock::Monotonic();
            assert(
                &monotonic == &clock ||
                dynamic_cast<Clock::SteadyMonotonicClock*>(&monotonic) != nullptr
            );

            auto& source = Entropy::Source();
            assert(
                &source == &entropy ||
                dynamic_cast<Entropy::NullEntropySource*>(&source) != nullptr
            );
        }
    });

    std::thread writer([&] {
        start.store(true, std::memory_order_release);
        for (uint32_t iteration = 0; iteration < 100000; ++iteration) {
            Execution::SetProvider(&execution);
            Queue::SetProvider(&queue);
            Synchronization::SetProvider(&synchronization);
            GPIO::SetController(&gpio);
            Clock::SetMonotonicClock(&clock);
            Clock::SetHighResolutionCounterProvider(&highResolution);
            Entropy::SetSource(&entropy);

            Execution::ResetProvider();
            Queue::ResetProvider();
            Synchronization::ResetProvider();
            GPIO::ResetController();
            Clock::ResetMonotonicClock();
            Clock::ResetHighResolutionCounterProvider();
            Entropy::ResetSource();
        }
        done.store(true, std::memory_order_release);
    });

    writer.join();
    reader.join();

    Execution::ResetProvider();
    Queue::ResetProvider();
    Synchronization::ResetProvider();
    GPIO::ResetController();
    Clock::ResetMonotonicClock();
    Clock::ResetHighResolutionCounterProvider();
    Entropy::ResetSource();
}

} // namespace

int main() {
    StressProviderPublication();
    return 0;
}
