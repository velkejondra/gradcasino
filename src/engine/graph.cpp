#include "gradcasino/graph.hpp"

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

Node::Ptr Graph::add_node(Node::Ptr node) {
    node->id = next_id_++;
    nodes_.push_back(node);
    return node;
}

void Graph::clear() noexcept {
    nodes_.clear();
    next_id_ = 0;
    next_input_index_ = 0;
    next_param_index_ = 0;
}

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

Node::Ptr Node::grad_node(Ptr output, Ptr wrt) {
    auto node = std::make_shared<Node>(OpKind::Grad, std::vector<Ptr>{std::move(output), std::move(wrt)});
    return Graph::current().add_node(std::move(node));
}

} // namespace gc
