#pragma once

#include "node.hpp"
#include <vector>
#include <cstddef>
#include <functional>

namespace gc {

class Graph {
public:
    [[nodiscard]] static Graph& current() noexcept;

    class [[nodiscard]] ScopeGuard {
    public:
        ScopeGuard() noexcept;
        ~ScopeGuard();
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
    };

    [[nodiscard]] static ScopeGuard scoped() noexcept { return ScopeGuard{}; }

    Node::Ptr add_node(Node::Ptr node);

    [[nodiscard]] const std::vector<Node::Ptr>& nodes() const noexcept { return nodes_; }

    template<std::invocable<const Node&> F>
    void for_each(F&& visitor) const {
        for (const auto& node : nodes_) {
            std::invoke(std::forward<F>(visitor), *node);
        }
    }

    void clear() noexcept;

    [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }
    [[nodiscard]] bool empty() const noexcept { return nodes_.empty(); }

    [[nodiscard]] std::uint32_t next_input_index() noexcept { return next_input_index_++; }
    [[nodiscard]] std::uint32_t next_param_index() noexcept { return next_param_index_++; }
    [[nodiscard]] std::uint32_t input_count() const noexcept { return next_input_index_; }
    [[nodiscard]] std::uint32_t param_count() const noexcept { return next_param_index_; }

private:
    std::vector<Node::Ptr> nodes_;
    std::uint32_t next_id_{0};
    std::uint32_t next_input_index_{0};
    std::uint32_t next_param_index_{0};
};

} // namespace gc
