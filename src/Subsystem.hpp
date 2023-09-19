#pragma once


#include <chrono>
#include <thread>
#include <utility>


#include "Logging.hpp"
#include "ThreadSafety.hpp"


class Subsystem {
private:
    /**
     * Log update time if it does not meet performance thresholds
     */
    void evaluate_update_time() const  {
        if (this->critical_loop_second_threshold() >= 0 and this->update_time >= this->critical_loop_second_threshold()) {
            this->log(Log::CRITICAL, std::format("Threaded Subsystem ({}) took {} seconds to update", this->id(), this->update_time));
            return;
        }
        if (this->warning_loop_second_threshold() >= 0 and this->update_time >= this->warning_loop_second_threshold()) {
            this->log(Log::WARNING, std::format("Threaded Subsystem ({}) took {} seconds to update", this->id(), this->update_time));
            return;
        }
    }

    /**
     * Update and check for shutdown signals
     */
    void _update() {
        const std::chrono::system_clock::time_point update_start = std::chrono::high_resolution_clock::now();

        this->update();

        if (this->should_panic.get_data()) {
            this->on_panic();
            return;
        }

        const std::chrono::system_clock::time_point update_end = std::chrono::high_resolution_clock::now();

        this->update_time = std::max(((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(update_end - update_start).count())) / 1e9f, 1e-9f);
        this->evaluate_update_time();
    }

    /**
     * Subsystem initialization and update loop
     */
    void run() {
        this->init();
        this->log(Log::REGULAR, "Subsystem initialized successfully");
        while (true) {
            this->_update();
            if (this->should_panic.get_data()) {
                break;
            }
            if (this->should_shutdown.get_data()) {
                this->log(Log::REGULAR, "Shutting down normally");
                this->on_shutdown();
                return;
            }
        }
    }

protected:
    float update_time; // NOTE this is the time it takes to call update(), not the TOTAL loop time
    ThreadSafe<bool> should_shutdown; // Shutdown at earliest convenience
    ThreadSafe<bool> should_panic; // Shutdown immediately
    std::thread thread;

    /**
     * A warning log will occur when the subsystem's loop time exceeds this threshold (in seconds)
     * A negative threshold will disable this functionality
     * @return Time threshold (seconds)
     */
    [[nodiscard]] virtual constexpr float warning_loop_second_threshold() const {
        return 0.05f;
    }

    /**
     * A critical log will occur when the subsystem's loop time exceeds this threshold (in seconds)
     * A negative threshold will disable this functionality
     * @return Time threshold (seconds)
     */
    [[nodiscard]] virtual constexpr float critical_loop_second_threshold() const {
        return 0.1f;
    }

    /**
     * Called at startup
     */
    virtual void init() = 0;

    /**
     * Called every loop
     */
    virtual void update() = 0;

    /**
     * Called on normal shutdown
     * Do not try to interact with other subsystems that may have lower shutdown priority since they should have already shut down
     */
    virtual void on_shutdown() = 0;

    /**
     * Called on emergency shutdown
     * Should not take a long time to execute
     * Logs while panicking are not guaranteed to be saved
     * Do not try to interact with other subsystems that may have lower shutdown priority since they should have already shut down
     */
    virtual void on_panic() = 0;

    bool should_exit() {
        return this->should_panic.get_data() or this->should_shutdown.get_data();
    }

public:
    Subsystem() {
        this->should_shutdown.set_data(false);
        this->should_panic.set_data(false);
        this->update_time = 0;
    }

    ~Subsystem() = default;

    /**
     * ID used for logging
     * @return Subsystem ID
     */
    [[nodiscard]] virtual constexpr std::string id() const = 0;

    /**
     * Easy access to Log::get_instance().log(...) and use the subsystem's id as the log source
     * @param log_level Log Severity
     * @param log_info Information to log
     */
    void log(const Log::LogLevel log_level, const std::string& log_info) const {
        Log::get_instance().log(log_level, this->id(), log_info);
    }

    /**
     * Tell subsystem to shut down at the earliest convenience
     */
    virtual void signal_shutdown() {
        this->should_shutdown.set_data(true);
    }

    /**
     * Tell subsystem to shut down immediately
     * Subclasses with long update times should check frequently
     */
    virtual void signal_panic() {
        this->should_panic.set_data(true);
    }

    /**
     * Run subsystem init and loop in separate thread
     */
    void run_thread() {
        this->thread = std::thread(&Subsystem::run, this);
    }

    /**
     * Join running subsystem thread (block until it finishes)
     */
    void thread_join() {
        this->thread.join();
    }

    /**
     * Detach running subsystem thread (not recommended)
     */
    void thread_detach() {
        this->thread.detach();
    }
};