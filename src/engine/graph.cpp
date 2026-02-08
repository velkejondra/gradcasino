#include "gradcasino/graph.hpp"
#include <cassert>

namespace gc {

namespace {
    thread_local Graph g_current_graph;
}

Graph& Graph::current() noexcept {
    return g_current_graph;
}

Graph::ScopeGuard::ScopeGuard() noexcept = default;

Graph::ScopeGuard::~ScopeGuard() {
    Graph::current().clear();
}

Graph::RegionGuard::RegionGuard() {
    Graph::current().push_region();
}

Graph::RegionGuard::~RegionGuard() {
    // Note: pop_region should usually be called manually by gc::loop
    // This destructor is a safety net, but loop logic handles the pop.
    // If stack is not empty (and not main), pop it.
    // However, the RAII pattern for loop() is tricky because we need the return value.
    // So RegionGuard is just for entering.
}

std::vector<Node::Ptr>& Graph::active_nodes() {
    if (region_stack_.empty()) {
        return nodes_;
    }
    return region_stack_.back().nodes;
}

void Graph::push_region() {
    region_stack_.emplace_back();
}

std::pair<std::vector<Node::Ptr>, std::vector<Node::Ptr>> Graph::pop_region() {
    assert(!region_stack_.empty());
    auto region = std::move(region_stack_.back());
    region_stack_.pop_back();
    return {std::move(region.nodes), std::move(region.inputs)};
}

Node::Ptr Graph::add_node(Node::Ptr node) {
    node->id = next_id_++;
    active_nodes().push_back(node);
    
    // If it's a BlockInput in a region, track it
    if (node->kind == OpKind::BlockInput && !region_stack_.empty()) {
        region_stack_.back().inputs.push_back(node);
    }
    
    return node;
}

void Graph::clear() noexcept {
    nodes_.clear();
    region_stack_.clear();
    next_id_ = 0;
    next_input_index_ = 0;
    next_param_index_ = 0;
}

std::uint32_t Graph::next_input_index() noexcept { return next_input_index_++; }
std::uint32_t Graph::next_param_index() noexcept { return next_param_index_++; }

Node::Ptr Node::constant(double value) {
    auto node = std::make_shared<Node>(OpKind::Constant);
    node->constant_value = value;
    return Graph::current().add_node(std::move(node));
}

Node::Ptr Node::input_node(std::uint32_t index, std::optional<std::string> name) {
    auto node = std::make_shared<Node>(OpKind::Input);
    node->input_index = index;
    node->name = std::move(name);
    return Graph::current().add_node(std::move(node));
}

Node::Ptr Node::param_node(double value, std::uint32_t index, std::optional<std::string> name) {
    auto node = std::make_shared<Node>(OpKind::Param);
    node->constant_value = value;
    node->input_index = index;
    node->name = std::move(name);
    return Graph::current().add_node(std::move(node));
}

Node::Ptr Node::unary(OpKind op, Ptr x) {
    return Graph::current().add_node(
        std::make_shared<Node>(op, std::vector<Ptr>{std::move(x)}));
}

Node::Ptr Node::binary(OpKind op, Ptr lhs, Ptr rhs) {
    return Graph::current().add_node(
        std::make_shared<Node>(op, std::vector<Ptr>{std::move(lhs), std::move(rhs)}));
}

Node::Ptr Node::ternary(OpKind op, Ptr a, Ptr b, Ptr c) {
    return Graph::current().add_node(
        std::make_shared<Node>(op, std::vector<Ptr>{std::move(a), std::move(b), std::move(c)}));
}

Node::Ptr Node::grad_node(Ptr output, Ptr wrt) {
    auto node = std::make_shared<Node>(OpKind::Grad, std::vector<Ptr>{std::move(output), std::move(wrt)});
    return Graph::current().add_node(std::move(node));
}

} // namespace gc
