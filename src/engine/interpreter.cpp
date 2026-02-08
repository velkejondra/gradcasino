#include "gradcasino/interpreter.hpp"
#include "gradcasino/graph.hpp"
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace gc {

class Interpreter::Impl {
public:
    InterpreterOptions opts;
    std::unordered_map<std::uint32_t, double> cache;
    std::span<const double> inputs;
    std::span<const double> params;
    
    double eval_node(const Node& node) {
        // Check cache first
        auto it = cache.find(node.id);
        if (it != cache.end()) {
            return it->second;
        }
        
        double result = eval_uncached(node);
        cache[node.id] = result;
        
        if (opts.on_eval) {
            opts.on_eval(node, result);
        }
        
        return result;
    }
    
    double eval_uncached(const Node& node) {
        switch (node.kind) {
            case OpKind::Constant:
                return node.constant_value;
                
            case OpKind::Input:
                if (node.input_index < inputs.size()) {
                    return inputs[node.input_index];
                }
                throw std::out_of_range("Input index out of range");
                
            case OpKind::Param:
                if (node.input_index < params.size()) {
                    return params[node.input_index];
                }
                // Params fallback to constant_value if not provided
                return node.constant_value;
                
            case OpKind::Add:
                return eval_node(*node.inputs[0]) + eval_node(*node.inputs[1]);
            case OpKind::Sub:
                return eval_node(*node.inputs[0]) - eval_node(*node.inputs[1]);
            case OpKind::Mul:
                return eval_node(*node.inputs[0]) * eval_node(*node.inputs[1]);
            case OpKind::Div:
                return eval_node(*node.inputs[0]) / eval_node(*node.inputs[1]);
            case OpKind::Neg:
                return -eval_node(*node.inputs[0]);
                
            case OpKind::Sqrt:
                return std::sqrt(eval_node(*node.inputs[0]));
            case OpKind::Exp:
                return std::exp(eval_node(*node.inputs[0]));
            case OpKind::Log:
                return std::log(eval_node(*node.inputs[0]));
            case OpKind::Abs:
                return std::abs(eval_node(*node.inputs[0]));
                
            case OpKind::Pow:
                return std::pow(eval_node(*node.inputs[0]), eval_node(*node.inputs[1]));
            case OpKind::Max:
                return std::max(eval_node(*node.inputs[0]), eval_node(*node.inputs[1]));
            case OpKind::Min:
                return std::min(eval_node(*node.inputs[0]), eval_node(*node.inputs[1]));
                
            case OpKind::GreaterThan:
                return eval_node(*node.inputs[0]) > eval_node(*node.inputs[1]) ? 1.0 : 0.0;
            case OpKind::LessThan:
                return eval_node(*node.inputs[0]) < eval_node(*node.inputs[1]) ? 1.0 : 0.0;
                
            case OpKind::Select: {
                double cond = eval_node(*node.inputs[0]);
                return cond != 0.0 ? eval_node(*node.inputs[1]) : eval_node(*node.inputs[2]);
            }
            
            case OpKind::Loop:
                return eval_loop(node);
                
            case OpKind::GetResult:
                // For single-output loops, the Loop node itself is the result
                // For multi-output, we need to evaluate the loop and extract
                return eval_get_result(node);
                
            case OpKind::BlockInput:
            case OpKind::Yield:
                // These are handled within eval_loop
                throw std::runtime_error("BlockInput/Yield should not be evaluated directly");
                
            case OpKind::Grad:
                // Grad needs symbolic differentiation, not implemented in interpreter
                throw std::runtime_error("Grad not supported in interpreter (needs symbolic diff)");
        }
        
        throw std::runtime_error("Unknown OpKind");
    }
    
    double eval_loop(const Node& loop) {
        // Get initial values from loop inputs
        std::vector<double> state;
        state.reserve(loop.inputs.size());
        for (const auto& input : loop.inputs) {
            state.push_back(eval_node(*input));
        }
        
        std::size_t iterations = loop.iteration_count;
        
        // Find yield node in body
        const Node* yield_node = nullptr;
        for (const auto& body_node : loop.body) {
            if (body_node->kind == OpKind::Yield) {
                yield_node = body_node.get();
                break;
            }
        }
        
        if (!yield_node) {
            throw std::runtime_error("Loop body missing Yield node");
        }
        
        // Execute loop iterations
        for (std::size_t i = 0; i < iterations; ++i) {
            // Set up BlockInput values for this iteration
            // arg 0 = loop index, args 1..N = state variables
            std::unordered_map<std::uint32_t, double> block_values;
            
            for (const auto& arg : loop.args) {
                if (arg->input_index == 0) {
                    block_values[arg->id] = static_cast<double>(i);
                } else if (arg->input_index - 1 < state.size()) {
                    block_values[arg->id] = state[arg->input_index - 1];
                }
            }
            
            // Evaluate body with block values injected into cache
            auto old_cache = std::move(cache);
            cache = block_values;
            
            // Evaluate yield inputs to get new state
            std::vector<double> new_state;
            new_state.reserve(yield_node->inputs.size());
            for (const auto& yield_input : yield_node->inputs) {
                new_state.push_back(eval_node(*yield_input));
            }
            
            // Restore cache and apply new state
            cache = std::move(old_cache);
            state = std::move(new_state);
        }
        
        // For single-output loop, return the single state value
        if (state.size() == 1) {
            return state[0];
        }
        
        // For multi-output, cache all results and return first
        // (GetResult will extract specific ones)
        loop_results_[loop.id] = std::move(state);
        return loop_results_[loop.id][0];
    }
    
    double eval_get_result(const Node& get_result) {
        const auto& loop_node = *get_result.inputs[0];
        
        // Check if loop already evaluated
        auto it = loop_results_.find(loop_node.id);
        if (it == loop_results_.end()) {
            // Evaluate the loop first
            eval_node(loop_node);
            it = loop_results_.find(loop_node.id);
        }
        
        if (it == loop_results_.end()) {
            throw std::runtime_error("Loop evaluation failed");
        }
        
        std::uint32_t idx = get_result.input_index;
        if (idx >= it->second.size()) {
            throw std::out_of_range("GetResult index out of range");
        }
        
        return it->second[idx];
    }
    
    std::unordered_map<std::uint32_t, std::vector<double>> loop_results_;
};

Interpreter::Interpreter(InterpreterOptions opts)
    : impl_(std::make_unique<Impl>()) {
    impl_->opts = std::move(opts);
}

Interpreter::~Interpreter() = default;
Interpreter::Interpreter(Interpreter&&) noexcept = default;
Interpreter& Interpreter::operator=(Interpreter&&) noexcept = default;

double Interpreter::eval(
    const Node& root,
    std::span<const double> inputs,
    std::span<const double> params) {
    
    impl_->cache.clear();
    impl_->loop_results_.clear();
    impl_->inputs = inputs;
    impl_->params = params;
    
    return impl_->eval_node(root);
}

std::vector<double> Interpreter::eval_many(
    std::span<const Node* const> roots,
    std::span<const double> inputs,
    std::span<const double> params) {
    
    impl_->cache.clear();
    impl_->loop_results_.clear();
    impl_->inputs = inputs;
    impl_->params = params;
    
    std::vector<double> results;
    results.reserve(roots.size());
    
    for (const auto* root : roots) {
        results.push_back(impl_->eval_node(*root));
    }
    
    return results;
}

void Interpreter::reset() {
    impl_->cache.clear();
    impl_->loop_results_.clear();
}

} // namespace gc
