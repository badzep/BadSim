#pragma once


#include <filesystem>


#include "Subsystem.hpp"
#include "Logging.hpp"


constexpr std::string LOG_PATH = "logs";

class LoggingSubsystem: public Subsystem {
private:
    std::string log_path;

    void init() override {
        std::filesystem::create_directory(LOG_PATH);
        this->log_path = std::format("{}/log_at_{}.txt", LOG_PATH, std::chrono::system_clock::now());
    }
    void update() override {
        Log::Logger::log_notifier.acquire();
        Log::get_instance().write_log_queue(this->log_path);
    }

    void on_shutdown() override {
        Log::get_instance().write_log_queue(this->log_path);
    }

    void on_panic() override {
        Log::get_instance().write_log_queue(this->log_path);
    };

    void signal_shutdown() override {
        this->should_shutdown.set_data(true);

        //release all semaphores
        Log::Logger::log_notifier.release();
    }

    void signal_panic() override {
        this->should_panic.set_data(true);

        //release all semaphores
        Log::Logger::log_notifier.release();
    }

    [[nodiscard]] constexpr float warning_loop_second_threshold() const override {
        return -1; // Disabled
    }

    [[nodiscard]] constexpr float critical_loop_second_threshold() const override {
        return -1; // Disabled
    }

public:
    LoggingSubsystem() = default;

    ~LoggingSubsystem() = default;

    [[nodiscard]] constexpr std::string id() const override {
        return "Log Subsystem";
    }
};
