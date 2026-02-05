#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <optional>

namespace gc {

enum class OpKind : std::uint8_t {
    Constant,
    Input,
    Param,
    
    Add,
    Sub,
    Mul,
    Div,
    Neg,
    Sqrt,
    Exp,
    Log,
    Abs,
    Pow,
    Max,
    Min,
    
    Grad,
};

[[nodiscard]] constexpr std::string_view op_name(OpKind kind) noexcept {
    switch (kind) {
        case OpKind::Constant: return "Constant";
        case OpKind::Input:    return "Input";
        case OpKind::Param:    return "Param";
        case OpKind::Add:      return "Add";
        case OpKind::Sub:      return "Sub";
        case OpKind::Mul:      return "Mul";
        case OpKind::Div:      return "Div";
        case OpKind::Neg:      return "Neg";
        case OpKind::Sqrt:     return "Sqrt";
        case OpKind::Exp:      return "Exp";
        case OpKind::Log:      return "Log";
        case OpKind::Abs:      return "Abs";
        case OpKind::Pow:      return "Pow";
        case OpKind::Max:      return "Max";
        case OpKind::Min:      return "Min";
        case OpKind::Grad:     return "Grad";
    }
    return "Unknown";
}

class Node {
public:
    using Ptr = std::shared_ptr<Node>;

    OpKind kind;
    std::vector<Ptr> inputs;
    double constant_value{0.0};
    std::uint32_t id{0};
    std::uint32_t input_index{0};
    std::optional<std::string> name;

    constexpr explicit Node(OpKind k, std::vector<Ptr> ins = {}) noexcept
        : kind{k}, inputs{std::move(ins)} {}

    [[nodiscard]] static Ptr constant(double value);
    [[nodiscard]] static Ptr input_node(std::uint32_t index, std::optional<std::string> name = {});
    [[nodiscard]] static Ptr param_node(double value, std::uint32_t index, std::optional<std::string> name = {});
    [[nodiscard]] static Ptr unary(OpKind op, Ptr x);
    [[nodiscard]] static Ptr binary(OpKind op, Ptr lhs, Ptr rhs);
    [[nodiscard]] static Ptr grad_node(Ptr output, Ptr wrt);

    [[nodiscard]] std::string display_name() const {
        if (name) return *name;
        return std::string(op_name(kind)) + "_" + std::to_string(id);
    }

    [[nodiscard]] constexpr bool is_constant() const noexcept { 
        return kind == OpKind::Constant; 
    }
    [[nodiscard]] constexpr bool is_input() const noexcept { 
        return kind == OpKind::Input; 
    }
    [[nodiscard]] constexpr bool is_param() const noexcept {
        return kind == OpKind::Param;
    }
    [[nodiscard]] constexpr bool is_grad() const noexcept {
        return kind == OpKind::Grad;
    }
};

} // namespace gc
