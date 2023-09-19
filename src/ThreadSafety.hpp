#pragma once


#include <mutex>


/**
 * Automatically locking mutex wrapper for (theoretically) any object
 * Not recommended for large objects or arrays/iterables
 * @tparam T Object type
 */
template<typename T> class ThreadSafe {
protected:
    std::mutex lock;
//    volatile T data;
    T data;
public:
    /**
     * Automatically locking mutex wrapper for (theoretically) any object
     */
    ThreadSafe() {
        this->data;
    }

    /**
     * Automatically locking mutex wrapper for (theoretically) any object
     * @param data Initial data value
     */
    explicit ThreadSafe(const T data): data(data) {    }

    /**
     * Get data value
     * @return Data value
     */
    [[nodiscard]] T get_data() {
        this->lock.lock();
        const T _data = this->data;
        this->lock.unlock();
        return _data;
    }

    /**
     * Set data value
     * @param new_data New data value
     */
    void set_data(const T new_data) {
        this->lock.lock();
        this->data = new_data;
        this->lock.unlock();
    }

    void manual_lock() {
        this->lock.lock();
    }

    void manual_unlock() {
        this->lock.unlock();
    }

    [[nodiscard]] T unsafe_get_data() {
        return this->data;
    }

    void unsafe_set_data(T new_data) {
        this->data = new_data;
    }
};

