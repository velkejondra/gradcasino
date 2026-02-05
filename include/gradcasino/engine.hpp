#pragma once

#include "graph.hpp"
#include "result.hpp"
#include "double.hpp"
#include <memory>
#include <span>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>
#include <cstddef>
#include <utility>

namespace gc {

namespace detail {

class RawKernel {
public:
    RawKernel();
    ~RawKernel();
    RawKernel(RawKernel&&) noexcept;
    RawKernel& operator=(RawKernel&&) noexcept;
    RawKernel(const RawKernel&) = delete;
    RawKernel& operator=(const RawKernel&) = delete;
    
    [[nodiscard]] explicit operator bool() const noexcept;
    
    [[nodiscard]] Result<std::vector<std::vector<double>>> run(
        std::span<const std::span<const double>> inputs,
        std::size_t num_outputs) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    
    template<std::size_t, std::size_t>
    friend class Kernel;
    
    friend RawKernel compile_raw(
        std::span<const Node::Ptr> outputs,
        std::span<const Node::Ptr> inputs);
    
    explicit RawKernel(std::unique_ptr<Impl> impl) noexcept;
};

RawKernel compile_raw(
    std::span<const Node::Ptr> outputs,
    std::span<const Node::Ptr> inputs);

template<std::size_t N>
struct OutputResult {
    std::array<std::vector<double>, N> data;
    
    template<std::size_t I>
    [[nodiscard]] std::vector<double>& get() noexcept {
        static_assert(I < N, "Output index out of range");
        return data[I];
    }
    
    template<std::size_t I>
    [[nodiscard]] const std::vector<double>& get() const noexcept {
        static_assert(I < N, "Output index out of range");
        return data[I];
    }
};

} // namespace detail

template<std::size_t NOut, std::size_t NIn>
class Kernel {
public:
    Kernel(detail::RawKernel kernel, std::array<std::uint32_t, NOut> output_ids)
        : kernel_{std::move(kernel)}, output_ids_{output_ids} {}
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return static_cast<bool>(kernel_);
    }
    
    template<typename... Args>
        requires (sizeof...(Args) == NIn) &&
                 (std::convertible_to<Args, std::span<const double>> && ...)
    [[nodiscard]] Result<detail::OutputResult<NOut>> operator()(Args&&... args) const {
        std::array<std::span<const double>, NIn> inputs{
            std::span<const double>{std::forward<Args>(args)}...
        };
        
        auto result = kernel_.run(inputs, NOut);
        if (!result) {
            return result.error();
        }
        
        detail::OutputResult<NOut> output;
        for (std::size_t i = 0; i < NOut; ++i) {
            output.data[i] = std::move((*result)[i]);
        }
        return output;
    }
    
    template<std::size_t I>
    [[nodiscard]] static constexpr std::size_t output_index() noexcept {
        static_assert(I < NOut, "Output index out of range");
        return I;
    }

private:
    detail::RawKernel kernel_;
    std::array<std::uint32_t, NOut> output_ids_;
};

template<std::size_t N>
struct Outputs {
    std::array<Node::Ptr, N> nodes;
    std::array<std::uint32_t, N> ids;
};

template<std::size_t N>
struct Inputs {
    std::array<Node::Ptr, N> nodes;
};

template<typename... Ts>
    requires (std::same_as<Ts, Double> && ...)
[[nodiscard]] auto outputs(Ts... outs) {
    constexpr std::size_t N = sizeof...(Ts);
    Outputs<N> result;
    std::size_t i = 0;
    ((result.nodes[i] = outs.node(), result.ids[i] = outs.node()->id, ++i), ...);
    return result;
}

template<typename... Ts>
    requires (std::same_as<Ts, Double> && ...)
[[nodiscard]] auto inputs(Ts... ins) {
    constexpr std::size_t N = sizeof...(Ts);
    Inputs<N> result;
    std::size_t i = 0;
    ((result.nodes[i++] = ins.node()), ...);
    return result;
}

struct CompileOptions {
    bool enable_simd = true;
    bool enable_fast_math = false;
    std::size_t opt_level = 2;
    
    [[nodiscard]] bool operator==(const CompileOptions&) const = default;
};

template<std::size_t NOut, std::size_t NIn>
[[nodiscard]] Result<Kernel<NOut, NIn>> compile(
    Outputs<NOut> outs,
    Inputs<NIn> ins,
    CompileOptions options = {}) {
    
    (void)options;
    
    auto raw = detail::compile_raw(outs.nodes, ins.nodes);
    if (!raw) {
        return Error{"Compilation failed"};
    }
    
    return Kernel<NOut, NIn>{std::move(raw), outs.ids};
}

} // namespace gc

namespace std {

template<std::size_t N>
struct tuple_size<gc::detail::OutputResult<N>> : std::integral_constant<std::size_t, N> {};

template<std::size_t I, std::size_t N>
struct tuple_element<I, gc::detail::OutputResult<N>> {
    using type = std::vector<double>;
};

} // namespace std
