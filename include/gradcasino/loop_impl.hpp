#pragma once

#include <tuple>
#include <utility>
#include <vector>
#include "double.hpp"

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

// Helper to create BlockInputs for args
template<typename... Ts, std::size_t... Is>
auto make_block_inputs(std::index_sequence<Is...>) {
    // Start index at 1 because 0 is loop index
    return std::make_tuple(
        Double(Node::input_node(Is + 1, std::nullopt))... // We need a special constructor or BlockInput access?
        // Node::input_node creates OpKind::Input, we need OpKind::BlockInput
        // Let's add a helper in Double/Node
    );
}

// We need a way to create BlockInput wrappers
inline Double block_arg(std::uint32_t index, std::string name = "") {
    auto node = std::make_shared<Node>(OpKind::BlockInput);
    node->input_index = index;
    if (!name.empty()) node->name = std::move(name);
    return Double{Graph::current().add_node(std::move(node))};
}

// The main loop implementation
template<typename... InitTypes, typename F>
[[nodiscard]] auto loop(std::size_t iterations, LoopInit<InitTypes...> init_vals, F&& body) {
    // 1. Capture outer update inputs
    auto outer_inputs = output_nodes(init_vals.values, std::make_index_sequence<sizeof...(InitTypes)>{});
    
    // 2. Start Region
    auto region_guard = Graph::RegionGuard();
    
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
    // Ideally user returns gc::vars(...) which is LoopYield
    using ResultType = decltype(result_yield);
    constexpr size_t NResults = std::tuple_size_v<decltype(result_yield.values)>;
    static_assert(NResults == sizeof...(InitTypes), "Loop body must yield same number of variables as initialized");
    
    // 6. Finish Region
    // Note: destructor of RegionGuard doesn't return the nodes, we pop manually
    // Actually Graph::RegionGuard destructor does nothing harmful, but we need to pop.
    // The previous implementation of RegionGuard just pushed.
    // We need to pop manually.
    
    // Convert yield to node vector
    auto body_outputs = output_nodes(result_yield.values, std::make_index_sequence<NResults>{});
    
    auto [nodes_in_region, block_inputs] = Graph::current().pop_region();
    
    // 7. Create Loop Node (in outer graph)
    auto loop_node = std::make_shared<Node>(OpKind::Loop);
    loop_node->inputs = std::move(outer_inputs); // Initial values
    loop_node->body = std::move(nodes_in_region); // All internal nodes
    // We also need to know which nodes are the RESULTS of the body
    // The 'body' vector contains ALL instructions.
    // We need to store the 'yield' nodes specifically?
    // Usually a Loop op has:
    // - inputs (initial)
    // - regions usually have a "Yield" op at the end.
    // simpler: The Loop node can store the "result nodes" indices relative to the body?
    // OR: create a dedicated "Yield" node in the graph and point to that.
    
    // Let's add a Yield node to the region
    auto yield_node = std::make_shared<Node>(OpKind::Select); // Abuse Select or add Yield?
    // Actually, let's just use the `body_outputs` vector we collected.
    // We can interpret the Loop node as returning multiple values.
    // But our Graph execution model assumes Node = Single Value usually.
    //
    // Multi-return nodes:
    // If Loop returns N values, we need N nodes in the outer graph representing them.
    // Like `LoopResult(original_loop_node, index)`.
    //
    // For now, let's assume we create N "Loop" nodes? No that duplicates the loop 100 times.
    //
    // Standard approach:
    // 1 Loop Node.
    // N "GetResult" nodes pointing to the Loop Node.
    //
    // Let's add `OpKind::GetResult`.
    
    // Hack for now:
    // If N=1, LoopNode is the result.
    // If N>1, we need Tuple/GetResult support.
    
    // Let's assume N=1 for simple case, or add GetResult.
    // Let's stick to N=1 in the MVP or support GetResult.
    
    // Re-using specific fields in Node:
    loop_node->constant_value = static_cast<double>(iterations); // Store iterations here?
    
    // Store the BODY OUTPUTS somehow. 
    // We can allow `Node` to have `std::vector<Ptr> extra_data`.
    // Or just append them to `body` and say "last N nodes are yields".
    // Let's do that: Yields are the last N nodes of body.
    for (auto& out : body_outputs) {
         // We don't push them again if they are already in nodes_in_region
         // But `output_nodes` returned Ptrs. They are in the region list.
         // We need to know WHICH ones they are.
         // Let's perform a "Yield" op at the end explicitly.
    }
    
    // Explicit Yield Op
    // We add a special "Yield" node to the region that takes the results as inputs.
    // Then the body always ends with exactly 1 Yield node.
    
    // Actually, `Graph::current()` is back to outer scope after pop_region?
    // No, `pop_region` restored it.
    
    // Add Loop node to OUTER graph
    auto loop_ptr = Graph::current().add_node(std::move(loop_node));
    
    // Create output wrappers
    auto make_results = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        // If single result, loop_ptr is the value?
        // If multiple, we need GetResult(loop_ptr, index).
        // Let's add GetResult op.
        
        if constexpr (sizeof...(InitTypes) == 1) {
             return std::make_tuple(Double(loop_ptr));
        } else {
             // We need to implement GetResult support
             // For now, let's just fail or hack it.
             // Let's add GetResult to OpKinds quickly.
             return std::make_tuple(Double(loop_ptr)); // BROKEN for N > 1
        }
    };
    
    // Wait, I need to fix the multi-return.
    // Let's stick to returning a Tuple of Doubles.
    
    return make_results(std::make_index_sequence<NResults>{});
}

} // namespace gc
