#pragma once

#include <chrono>
#include <cstdint>

namespace lms {

/// Seconds since the Unix epoch (UTC). Stored as INTEGER in SQLite.
using Timestamp = std::int64_t;

inline constexpr std::int64_t kSecondsPerMinute = 60;
inline constexpr std::int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
inline constexpr std::int64_t kSecondsPerDay = 24 * kSecondsPerHour;

/// Abstraction over "now" so that deadline logic can be tested deterministically
/// (Dependency Inversion: services depend on IClock, not on the system clock).
class IClock {
public:
    virtual ~IClock() = default;
    virtual Timestamp now() const = 0;
};

class SystemClock final : public IClock {
public:
    Timestamp now() const override {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
};

/// Manually controlled clock used by the automated tests.
class FixedClock final : public IClock {
public:
    explicit FixedClock(Timestamp start) : now_(start) {}
    Timestamp now() const override { return now_; }
    void set(Timestamp t) { now_ = t; }
    void advance(std::int64_t seconds) { now_ += seconds; }

private:
    Timestamp now_;
};

}  // namespace lms
