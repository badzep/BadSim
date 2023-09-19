#pragma once

#include "ThreadSafety.hpp"

namespace ManagerSignals {
    static ThreadSafe<bool> shutdown;
    static ThreadSafe<bool> panic;
}