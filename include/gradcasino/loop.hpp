#pragma once

#include <tuple>
#include <utility>
#include <vector>
#include <string>
#include "double.hpp"
#include "node.hpp"
#include "graph.hpp"

namespace gc {

// Helper container for loop initial values
template<typename... Ts>
struct LoopInit {
    std::tuple<Ts...> values;
};

template<typename... Ts>
[[nodiscard]] auto init(Ts&&... args) {
    return LoopInit<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(args)...)};
}

// Helper container for loop return values (yield)
template<typename... Ts>
struct LoopYield {
    std::tuple<Ts...> values;
};

template<typename... Ts>
[[nodiscard]] auto vars(Ts&&... args) {
    return LoopYield<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(args)...)};
}

// Helper to unpack tuple and return vector of nodes
template<typename Tuple, std::size_t... Is>
std::vector<Node::Ptr> output_nodes(const Tuple& t, std::index_sequence<Is...>) {
    return {std::get<Is>(t).node()...};
}

// We need a way to create BlockInput wrappers
inline Double block_arg(std::uint32_t index, std::string name = "") {
    auto node = std::make_shared<Node>(OpKind::BlockInput);
    node->input_index = index;
    if (!name.empty()) node->name = std::move(name);
    return Double{Graph::current().add_node(std::move(node))};
}

inline Double get_result(Node::Ptr loop_node, std::uint32_t index) {
    auto node = std::make_shared<Node>(OpKind::GetResult, std::vector<Node::Ptr>{loop_node});
    node->input_index = index; // Reuse input_index to store which result
    return Double{Graph::current().add_node(std::move(node))};
}

// The main loop implementation
template<typename... InitTypes, typename F>
[[nodiscard]] auto loop(std::size_t iterations, LoopInit<InitTypes...> init_vals, F&& body) {
    // 1. Capture outer update inputs
    auto outer_inputs = output_nodes(init_vals.values, std::make_index_sequence<sizeof...(InitTypes)>{});
    
    // 2. Start Region
    // We strictly use Graph::push_region/pop_region here manually instead of potentially ambiguous RAII
    Graph::current().push_region();
    
    // 3. Create Block Arguments
    // Arg 0: Loop Index
    auto index_var = block_arg(0, "loop_idx");
    
    // Args 1..N: Loop Variables
    auto make_args = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(block_arg(static_cast<std::uint32_t>(Is + 1), "arg_" + std::to_string(Is))...);
    };
    auto args_tuple = make_args(std::make_index_sequence<sizeof...(InitTypes)>{});
    
    // 4. Run Body
    auto result_yield = std::apply([&](auto&&... args) {
        return body(index_var, args...);
    }, args_tuple);
    
    // 5. Check result type match
    constexpr size_t NResults = std::tuple_size_v<decltype(result_yield.values)>;
    static_assert(NResults == sizeof...(InitTypes), "Loop body must yield same number of variables as initialized");
    
    auto yield_values = output_nodes(result_yield.values, std::make_index_sequence<NResults>{});
    
    // Create explicitly Yield node inside the region
    auto yield_node = std::make_shared<Node>(OpKind::Yield, yield_values);
    Graph::current().add_node(yield_node);
    
    // 6. Finish Region
    auto [nodes_in_region, block_inputs] = Graph::current().pop_region();
    
    // 7. Create Loop Node (in outer graph)
    auto loop_node = std::make_shared<Node>(OpKind::Loop);
    loop_node->inputs = std::move(outer_inputs); // Initial values
    loop_node->body = std::move(nodes_in_region); // Body nodes
    loop_node->args = std::move(block_inputs);    // BlockInput nodes for the body
    loop_node->iteration_count = iterations;      // Iteration count
    
    auto loop_ptr = Graph::current().add_node(loop_node);
    
    // 8. Create output wrappers
    auto make_results = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        if constexpr (sizeof...(InitTypes) == 1) {
            // Optimization: If only 1 result, the Loop node IS the result (convention)
            return std::make_tuple(Double(loop_ptr));
        } else {
            return std::make_tuple(get_result(loop_ptr, static_cast<std::uint32_t>(Is))...);
        }
    };
    
    return make_results(std::make_index_sequence<NResults>{});
}

} // namespace gc
