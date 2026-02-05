#include "gradcasino/double.hpp"
#include "gradcasino/node.hpp"
#include "gradcasino/graph.hpp"
#include <limits>

namespace gc {

Double::Double(double value)
    : node_{Node::constant(value)} {}

Double::Double(std::shared_ptr<Node> node) noexcept
    : node_{std::move(node)} {}

double Double::value() const {
    if (node_->is_constant() || node_->is_param()) {
        return node_->constant_value;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

Double& Double::name(std::string_view n) {
    node_->name = std::string(n);
    return *this;
}

std::optional<std::string> Double::get_name() const {
    return node_->name;
}

Double operator+(Double lhs, Double rhs) {
    return Double{Node::binary(OpKind::Add, std::move(lhs.node_), std::move(rhs.node_))};
}

Double operator-(Double lhs, Double rhs) {
    return Double{Node::binary(OpKind::Sub, std::move(lhs.node_), std::move(rhs.node_))};
}

Double operator*(Double lhs, Double rhs) {
    return Double{Node::binary(OpKind::Mul, std::move(lhs.node_), std::move(rhs.node_))};
}

Double operator/(Double lhs, Double rhs) {
    return Double{Node::binary(OpKind::Div, std::move(lhs.node_), std::move(rhs.node_))};
}

Double operator-(Double x) {
    return Double{Node::unary(OpKind::Neg, std::move(x.node_))};
}

Double& Double::operator+=(const Double& rhs) {
    *this = *this + rhs;
    return *this;
}

Double& Double::operator-=(const Double& rhs) {
    *this = *this - rhs;
    return *this;
}

Double& Double::operator*=(const Double& rhs) {
    *this = *this * rhs;
    return *this;
}

Double& Double::operator/=(const Double& rhs) {
    *this = *this / rhs;
    return *this;
}

Double sqrt(Double x) {
    return Double{Node::unary(OpKind::Sqrt, x.node())};
}

Double exp(Double x) {
    return Double{Node::unary(OpKind::Exp, x.node())};
}

Double log(Double x) {
    return Double{Node::unary(OpKind::Log, x.node())};
}

Double pow(Double base, Double exponent) {
    return Double{Node::binary(OpKind::Pow, base.node(), exponent.node())};
}

Double max(Double a, Double b) {
    return Double{Node::binary(OpKind::Max, a.node(), b.node())};
}

Double min(Double a, Double b) {
    return Double{Node::binary(OpKind::Min, a.node(), b.node())};
}

Double abs(Double x) {
    return Double{Node::unary(OpKind::Abs, x.node())};
}

Double input(std::string_view name) {
    auto index = Graph::current().next_input_index();
    return Double{Node::input_node(index, std::string(name))};
}

Double input() {
    auto index = Graph::current().next_input_index();
    return Double{Node::input_node(index)};
}

Double param(double value, std::string_view name) {
    auto index = Graph::current().next_param_index();
    return Double{Node::param_node(value, index, std::string(name))};
}

Double param(double value) {
    auto index = Graph::current().next_param_index();
    return Double{Node::param_node(value, index)};
}

Double grad(Double output, Double wrt) {
    return Double{Node::grad_node(output.node(), wrt.node())};
}

} // namespace gc
