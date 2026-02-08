#pragma once

#include "node.hpp"
#include <span>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

namespace gc {

/// Options for interpreter execution
struct InterpreterOptions {
    /// Called after each node is evaluated (for tracing/debugging)
    std::function<void(const Node&, double)> on_eval = nullptr;
};

/// Tree-walking interpreter for graph evaluation.
/// Used as a debug/reference backend when JIT is not needed.
class Interpreter {
public:
    explicit Interpreter(InterpreterOptions opts = {});
    ~Interpreter();
    
    Interpreter(Interpreter&&) noexcept;
    Interpreter& operator=(Interpreter&&) noexcept;
    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
    
    /// Evaluate a single output node
    [[nodiscard]] double eval(
        const Node& root,
        std::span<const double> inputs,
        std::span<const double> params);
    
    /// Evaluate multiple output nodes
    [[nodiscard]] std::vector<double> eval_many(
        std::span<const Node* const> roots,
        std::span<const double> inputs,
        std::span<const double> params);
    
    /// Clear memoization cache (call between batches if graph changed)
    void reset();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gc
