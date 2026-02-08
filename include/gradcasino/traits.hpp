#pragma once

#include "gradcasino/double.hpp"
#include <concepts>

// Helper to deduce function argument types
namespace gc::detail {

template<typename F>
struct FunctionTraits : public FunctionTraits<decltype(&F::operator())> {};

template<typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType(ClassType::*)(Args...) const> {
    using args_tuple = std::tuple<Args...>;
};

template<typename F>
using FirstArgType = typename std::tuple_element<0, typename FunctionTraits<F>::args_tuple>::type;

} // namespace gc::detail
