#pragma once

#include "node.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace gc {

class Graph {
public:
    static Graph& current() noexcept;

    struct ScopeGuard {
        ScopeGuard() noexcept;
        ~ScopeGuard();
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
    };

    static std::unique_ptr<ScopeGuard> scoped() {
        return std::make_unique<ScopeGuard>();
    }

    // Region management for loops
    // NOTE: RegionGuard only pushes on construction. You MUST call pop_region()
    // manually before destruction to retrieve the region nodes.
    struct RegionGuard {
        RegionGuard();
        ~RegionGuard();
        RegionGuard(const RegionGuard&) = delete;
        RegionGuard& operator=(const RegionGuard&) = delete;
    };

    Node::Ptr add_node(Node::Ptr node);
    void clear() noexcept;

    std::uint32_t next_input_index() noexcept;
    std::uint32_t next_param_index() noexcept;
    
    [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }
    [[nodiscard]] bool empty() const noexcept { return nodes_.empty(); }
    [[nodiscard]] std::uint32_t input_count() const noexcept { return next_input_index_; }
    [[nodiscard]] std::uint32_t param_count() const noexcept { return next_param_index_; }

    void for_each(const std::function<void(const Node&)>& callback) const {
        for (const auto& node : nodes_) {
            callback(*node);
        }
    }

    void push_region();
    std::pair<std::vector<Node::Ptr>, std::vector<Node::Ptr>> pop_region();

private:
    std::vector<Node::Ptr> nodes_;
    
    // For nested regions (loops)
    struct Region {
        std::vector<Node::Ptr> nodes;
        std::vector<Node::Ptr> inputs; // BlockInputs
    };
    std::vector<Region> region_stack_;
    
    std::uint32_t next_id_{0};
    std::uint32_t next_input_index_{0};
    std::uint32_t next_param_index_{0};

    // Access to current active container (main nodes_ or top of region_stack_)
    std::vector<Node::Ptr>& active_nodes();
    friend class Node;
};

} // namespace gc
