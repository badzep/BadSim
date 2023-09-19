#pragma once


#include <iostream>
#include <chrono>
#include <mutex>
#include <queue>
#include <fstream>
#include <format>


#include "ManagerSignals.hpp"


namespace Log {
    /**
     * All logs are written to a log file
     * Debug and Critical logs are also written to console
     * Panic logs will start a panic shutdown
     */
    enum LogLevel{
        /**
         * For debugging \n
         * Written to both log file and console
         */
        DEBUG,

        /**
         * To keep track of regular behavior \n
         * Written to log file only
         */
        REGULAR,

        /**
         * For things that may or may not cause small issues\n
         * Written to log file only
         */
        WARNING,

        /**
         * For major issues that could cause serious problems \n
         * Written to both log file and console
         */
        CRITICAL,

        /**
         * STARTS A PANIC SHUTDOWN \n
         * Something has gone horribly wrong, need to stop immediately \n
         * Unwritten logs may be lost on panic shutdown \n
         * Written to both log file and console
         */
        PANIC
    };

    /**
     * Convert log level enum value to its corresponding name
     * @param log_level Log Severity
     */
    std::string log_level_as_string(const LogLevel log_level) {
        switch (log_level) {
            case DEBUG:
                return "DEBUG";
            case REGULAR:
                return "REGULAR";
            case WARNING:
                return "WARNING";
            case CRITICAL:
                return "CRITICAL";
            case PANIC:
                return "PANIC";
        }
        return "UNKNOWN";
    }

    /**
     * A single log message
     */
    class Message {
    public:
        const LogLevel log_level;
        const std::string source;
        const std::chrono::time_point<std::chrono::system_clock> timestamp;
        const std::string log_info;

        Message(const LogLevel log_level, std::string source, std::string log_info): log_level(log_level), source(std::move(source)), timestamp(std::chrono::system_clock::now()), log_info(std::move(log_info)) {     }

        ~Message() = default;

        [[nodiscard]] std::string get_string() const {
            return std::format("[{}] {} at {} -> {}\n", log_level_as_string(this->log_level), this->source, this->timestamp, this->log_info);
        }
    };

    /**
     * It logs
     */
    class Logger {
    private:
        std::mutex console_lock;
        std::mutex pending_logs_lock;
        std::queue<Message> pending_logs;

        Logger() = default;
        ~Logger() = default;

        /**
         * Write and remove the first log in the queue
         * @param file Log file
         */
        void write_front_log(std::ofstream &file) {
            file << this->pending_logs.front().get_string();
            this->pending_logs.pop();
        }

    public:
        static std::binary_semaphore log_notifier;
        /**
         * Get singleton instance
         */
        static Logger& get_instance() {
            static Logger logger;
            return logger;
        }

        /**
         * Write to log file
         * Critical and Debug level logs are also written to console
         * See Subsystem::log(...) if using from subsystem
         * @param log_level Log Severity
         * @param source_name Identify where the log is coming from
         * @param log_info Information to log
         */
        void log(const LogLevel log_level, const std::string& source_name, const std::string& log_info) {
            if (log_level == PANIC or log_level == CRITICAL or log_level == DEBUG) {
                std::cout << std::format("[{}] {} -> {}\n", log_level_as_string(log_level), source_name,  log_info);
            }
            this->pending_logs_lock.lock();
            this->pending_logs.emplace(log_level, source_name, log_info);
            this->pending_logs_lock.unlock();
            Log::Logger::log_notifier.release();
            if (log_level == PANIC) {
                ManagerSignals::panic.set_data(true);
            }
        }

        /**
         * Write all queued log messages to log file
         * @param file Log file
         */
        void write_log_queue(const std::string &log_path) {
            this->pending_logs_lock.lock();
            if (!this->pending_logs.empty()) {
                std::ofstream file;
                file.open(log_path, std::ios::app);
                while (true) {
                    this->write_front_log(file);
                    if (this->pending_logs.empty()) {
                        break;
                    }
                }
                file.close();
            }
            this->pending_logs_lock.unlock();
        }

        Logger(Logger const&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger const&) = delete;
        Logger& operator=(Logger &&) = delete;
    };

    std::binary_semaphore Logger::log_notifier{0};

    [[nodiscard]] Logger& get_instance() {
        return Logger::get_instance();
    }
}