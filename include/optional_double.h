#ifndef OPTIONAL_DOUBLE_H
#define OPTIONAL_DOUBLE_H

struct OptionalDouble {
    bool has_value_{false};
    double value_{0.0};
    OptionalDouble() = default;
    explicit OptionalDouble(double v) : has_value_(true), value_(v) {}
    explicit operator bool() const { return has_value_; }
    double operator*() const { return value_; }
};

#endif
