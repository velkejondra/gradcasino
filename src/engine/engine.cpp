#include "gradcasino/engine.hpp"
#include "gradcasino/interpreter.hpp"
#include <utility>

namespace gc::detail {

class RawKernel::Impl {
public:
    bool valid{false};
    bool use_interpreter{false};
    InterpreterOptions interp_opts;
    std::vector<Node::Ptr> output_nodes;
    std::vector<Node::Ptr> input_nodes;
};

RawKernel::RawKernel() 
    : impl_{std::make_unique<Impl>()} {}

RawKernel::RawKernel(std::unique_ptr<Impl> impl) noexcept
    : impl_{std::move(impl)} {}

RawKernel::~RawKernel() = default;
RawKernel::RawKernel(RawKernel&&) noexcept = default;
RawKernel& RawKernel::operator=(RawKernel&&) noexcept = default;

RawKernel::operator bool() const noexcept {
    return impl_ && impl_->valid;
}

Result<std::vector<std::vector<double>>> RawKernel::run(
    std::span<const std::span<const double>> inputs,
    std::size_t num_outputs) const {
    
    if (!*this) {
        return Error{"Kernel not compiled"};
    }
    
    if (inputs.empty()) {
        return Error{"No inputs provided"};
    }
    
    std::size_t batch_size = inputs[0].size();
    for (const auto& inp : inputs) {
        if (inp.size() != batch_size) {
            return Error{"Input size mismatch"};
        }
    }
    
    std::vector<std::vector<double>> results(num_outputs);
    for (auto& r : results) {
        r.resize(batch_size);
    }
    
    if (impl_->use_interpreter) {
        // Run interpreter for each batch element
        Interpreter interp(impl_->interp_opts);
        
        std::vector<const Node*> output_ptrs;
        output_ptrs.reserve(impl_->output_nodes.size());
        for (const auto& n : impl_->output_nodes) {
            output_ptrs.push_back(n.get());
        }
        
        for (std::size_t b = 0; b < batch_size; ++b) {
            // Gather inputs for this batch element
            std::vector<double> batch_inputs;
            batch_inputs.reserve(inputs.size());
            for (const auto& inp : inputs) {
                batch_inputs.push_back(inp[b]);
            }
            
            // Evaluate all outputs
            std::vector<double> params;  // No params for now
            auto batch_results = interp.eval_many(output_ptrs, batch_inputs, params);
            
            for (std::size_t o = 0; o < num_outputs && o < batch_results.size(); ++o) {
                results[o][b] = batch_results[o];
            }
            
            interp.reset();
        }
    }
    // JIT path: results already initialized to 0 (stub)
    
    return results;
}

RawKernel compile_raw(
    std::span<const Node::Ptr> outputs,
    std::span<const Node::Ptr> inputs) {
    
    if (outputs.empty() || inputs.empty()) {
        return RawKernel{};
    }
    
    auto impl = std::make_unique<RawKernel::Impl>();
    impl->valid = true;
    
    (void)inputs;
    
    return RawKernel{std::move(impl)};
}

RawKernel compile_raw_interp(
    std::span<const Node::Ptr> outputs,
    std::span<const Node::Ptr> inputs,
    InterpreterOptions opts) {
    
    if (outputs.empty() || inputs.empty()) {
        return RawKernel{};
    }
    
    auto impl = std::make_unique<RawKernel::Impl>();
    impl->valid = true;
    impl->use_interpreter = true;
    impl->interp_opts = std::move(opts);
    
    for (const auto& out : outputs) {
        impl->output_nodes.push_back(out);
    }
    for (const auto& in : inputs) {
        impl->input_nodes.push_back(in);
    }
    
    return RawKernel{std::move(impl)};
}

} // namespace gc::detail
