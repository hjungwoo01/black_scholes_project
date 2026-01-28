#ifndef OPTIONAL_DOUBLE_H
#define OPTIONAL_DOUBLE_H

struct OptionalDouble {
    bool has_value_{false};
    double value_{0.0};
    OptionalDouble() noexcept = default;
    explicit OptionalDouble(double v) noexcept : has_value_(true), value_(v) {}
    explicit operator bool() const noexcept { return has_value_; }
    double operator*() const noexcept { return value_; }
};

#endif
