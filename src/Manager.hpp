#pragma once


#include <stack>
#include <memory>
#include <cinttypes>
#include <thread>
#include <list>


#include "PartialProcessingSubsystem.hpp"
#include "Subsystem.hpp"
#include "LoggingSubsystem.hpp"
#include "Cell.hpp"
#include "Constants.hpp"
#include "Render.hpp"
#include "ManagerSignals.hpp"
#include "Simulation.hpp"

constexpr unsigned int RECOMMENDED_THREAD_COUNT = 4;
constexpr unsigned int MINIMUM_THREAD_COUNT = 4;
constexpr unsigned int TICKS_PER_RENDER = 1;

constexpr bool AUTO_SAVE = true;
constexpr float AUTO_SAVE_PERIOD = 60.0f * 30.0f; // 30 minutes

class Manager {
private:
    std::shared_ptr<LoggingSubsystem> logging_subsystem;
    std::shared_ptr<RenderSubsystem> render_subsystem;

    bool paused;
    bool has_shutdown;
    unsigned int available_threads;
    unsigned int partial_processor_count;

    std::stack<std::shared_ptr<Subsystem>> subsystems;
    std::vector<std::shared_ptr<PartialProcessingSubsystem>> partial_processors;

    Simulation simulation;


    void initialize() {
        ManagerSignals::shutdown.set_data(false);
        ManagerSignals::panic.set_data(false);
        this->paused = false;
        Body::id_count = 0;

        this->check_threads();
        this->partial_processor_count = this->available_threads - 1;

        this->logging_subsystem = std::make_shared<LoggingSubsystem>();
        this->subsystems.push(this->logging_subsystem);
        this->logging_subsystem->run_thread();

        this->render_subsystem = std::make_shared<RenderSubsystem>(this->simulation.get_cells(), this->simulation.get_eggs(), this->simulation.get_foods());
        this->subsystems.push(this->render_subsystem);
        this->render_subsystem->run_thread();

//        this->simulation.setup_environment();
        this->simulation = Simulation("saves/unstable8");
        for (unsigned int thread_index = 0; thread_index < this->partial_processor_count; thread_index++) {
            std::shared_ptr<PartialProcessingSubsystem> partial_processor = std::make_shared<PartialProcessingSubsystem>(thread_index, this->partial_processor_count, this->simulation.get_cells(), this->simulation.get_eggs(), this->simulation.get_foods());
            this->subsystems.push(partial_processor);
            this->partial_processors.push_back(partial_processor);
            this->subsystems.top()->run_thread();
        }

        Log::get_instance().log(Log::REGULAR, Manager::id(), "Initialized all subsystems");
    }

    void check_threads() {
        this->available_threads = std::thread::hardware_concurrency();
        if (this->available_threads < MINIMUM_THREAD_COUNT) {
            Log::get_instance().log(Log::PANIC, Manager::id(), std::format("Thread hint ({}) does not meet minimum thread requirement ({})", this->available_threads, MINIMUM_THREAD_COUNT));
        }
        else if (this->available_threads < RECOMMENDED_THREAD_COUNT) {
            Log::get_instance().log(Log::WARNING, Manager::id(), std::format("Thread hint ({}) does not meet recommended thread count ({})", this->available_threads, RECOMMENDED_THREAD_COUNT));
        }
    }

    void shutdown() {
        if (this->has_shutdown) {
            return;
        }
        Log::get_instance().log(Log::REGULAR, Manager::id(), "Starting shutdown process");
        while (!this->subsystems.empty()) {
            this->subsystems.top()->signal_shutdown();
            this->subsystems.top()->thread_join();
            this->subsystems.pop();
        }
        this->has_shutdown = true;
    }

    /**
     * Emergency shutdown
     * Logs while panicking are not guaranteed to be saved
     */
    void panic() {
        while (!this->subsystems.empty()) {
            this->subsystems.top()->signal_panic();
            this->subsystems.top()->thread_join();
            this->subsystems.pop();
        }
        this->has_shutdown = true;
    }

public:
    Manager() {
        this->has_shutdown = false;
    }

    ~Manager() {
        if (!this->has_shutdown) {
            Log::get_instance().log(Log::CRITICAL, Manager::id(), "Manager was not shut down before destruction. Shutting down automatically.");
            this->shutdown();
        }
    }

    void run() {
        std::chrono::system_clock::time_point last_save_time = std::chrono::high_resolution_clock::now();

        unsigned long ticks = 0;
        this->initialize();
        while (true) {
            const std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
            if (AUTO_SAVE and !this->paused and ((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(start - last_save_time).count())) / 1e9f>= AUTO_SAVE_PERIOD) {
                this->simulation.save();
                last_save_time = start;
            }

            if (ManagerSignals::panic.get_data()) {
                this->panic();
                return;
            }
            if (ManagerSignals::shutdown.get_data()) {
                this->shutdown();
                return;
            }

            if (!this->render_subsystem->paused.get_data()) {
                this->tick();
                this->simulation.produce();
                this->simulation.clear();
            }

            if (IsKeyPressed(KEY_K)) {
                simulation.save();
            }

            if ((ticks % TICKS_PER_RENDER) == 0) {
                this->render_subsystem->render_notifier.release(); // yes it is pointless to use a separate render thread like this
                this->render_subsystem->finished_render_notifier.acquire(); // but eventually i will allow it to render while doing interactions
            }

            const std::chrono::system_clock::time_point end = std::chrono::high_resolution_clock::now();
            const float update_time = std::max(((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count())) / 1e9f, 1e-9f);
            const float updates_per_second = 1.0f / update_time;
            printf("Updates Per Sec: %f\n", updates_per_second);
            ticks++;
        }
    }


    void tick() {
        for (const std::shared_ptr<PartialProcessingSubsystem> &partial_processor: this->partial_processors) {
            partial_processor->do_interaction_notifier.release();
        }
        for (const std::shared_ptr<PartialProcessingSubsystem> &partial_processor: this->partial_processors) {
            partial_processor->interaction_completion_notifier.acquire();
        }

        for (const std::shared_ptr<PartialProcessingSubsystem> &partial_processor: this->partial_processors) {
            partial_processor->do_tick_notifier.release();
        }
        for (const std::shared_ptr<PartialProcessingSubsystem> &partial_processor: this->partial_processors) {
            partial_processor->tick_completion_notifier.acquire();
        }
    }
    /**
     * ID used for logging
     * @return ID
     */
    [[nodiscard]] static constexpr std::string id() {
        return "Manager";
    }
};
