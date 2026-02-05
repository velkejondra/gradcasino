#pragma once

#include <memory>
#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <optional>

namespace gc {

class Node;
class Graph;

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

class Double {
public:
    explicit Double(double value);

    template<Arithmetic T>
        requires (!std::same_as<T, double>)
    explicit Double(T value) : Double(static_cast<double>(value)) {}

    explicit Double(std::shared_ptr<Node> node) noexcept;

    ~Double() = default;
    Double(const Double&) = default;
    Double& operator=(const Double&) = default;
    Double(Double&&) noexcept = default;
    Double& operator=(Double&&) noexcept = default;

    [[nodiscard]] std::shared_ptr<Node> node() const noexcept { return node_; }
    [[nodiscard]] double value() const;

    Double& name(std::string_view n);
    [[nodiscard]] std::optional<std::string> get_name() const;

    friend Double operator+(Double lhs, Double rhs);
    friend Double operator-(Double lhs, Double rhs);
    friend Double operator*(Double lhs, Double rhs);
    friend Double operator/(Double lhs, Double rhs);
    friend Double operator-(Double x);

    template<Arithmetic T>
    friend Double operator+(Double lhs, T rhs) { return lhs + Double(rhs); }
    template<Arithmetic T>
    friend Double operator+(T lhs, Double rhs) { return Double(lhs) + rhs; }
    template<Arithmetic T>
    friend Double operator-(Double lhs, T rhs) { return lhs - Double(rhs); }
    template<Arithmetic T>
    friend Double operator-(T lhs, Double rhs) { return Double(lhs) - rhs; }
    template<Arithmetic T>
    friend Double operator*(Double lhs, T rhs) { return lhs * Double(rhs); }
    template<Arithmetic T>
    friend Double operator*(T lhs, Double rhs) { return Double(lhs) * rhs; }
    template<Arithmetic T>
    friend Double operator/(Double lhs, T rhs) { return lhs / Double(rhs); }
    template<Arithmetic T>
    friend Double operator/(T lhs, Double rhs) { return Double(lhs) / rhs; }

    Double& operator+=(const Double& rhs);
    Double& operator-=(const Double& rhs);
    Double& operator*=(const Double& rhs);
    Double& operator/=(const Double& rhs);

    template<Arithmetic T>
    Double& operator+=(T rhs) { return *this += Double(rhs); }
    template<Arithmetic T>
    Double& operator-=(T rhs) { return *this -= Double(rhs); }
    template<Arithmetic T>
    Double& operator*=(T rhs) { return *this *= Double(rhs); }
    template<Arithmetic T>
    Double& operator/=(T rhs) { return *this /= Double(rhs); }

private:
    std::shared_ptr<Node> node_;
};

[[nodiscard]] Double sqrt(Double x);
[[nodiscard]] Double exp(Double x);
[[nodiscard]] Double log(Double x);
[[nodiscard]] Double pow(Double base, Double exponent);
[[nodiscard]] Double max(Double a, Double b);
[[nodiscard]] Double min(Double a, Double b);
[[nodiscard]] Double abs(Double x);

[[nodiscard]] Double input(std::string_view name);
[[nodiscard]] Double input();
[[nodiscard]] Double param(double value, std::string_view name);
[[nodiscard]] Double param(double value);
[[nodiscard]] Double grad(Double output, Double wrt);

} // namespace gc
