#include "gradcasino/engine.hpp"
#include <utility>

namespace gc::detail {

class RawKernel::Impl {
public:
    bool valid{false};
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
    
    (void)inputs;
    
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

} // namespace gc::detail
