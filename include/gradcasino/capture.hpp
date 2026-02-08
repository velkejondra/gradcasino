#pragma once

#include <vector>
#include <memory>
#include "node.hpp"
#include "graph.hpp"

namespace gc {

// Helper class to intercept node creation/copying for capture analysis
class CapturingDouble {
public:
    explicit CapturingDouble(Double& ref) 
        : ref_(ref), original_node_(ref.node()) {}
    
    // When the lambda finishes, we check if ref_.node() != original_node_
    // If different, the variable was mutated inside the loop.
    [[nodiscard]] bool modified() const {
        return ref_.node() != original_node_;
    }
    
    [[nodiscard]] Double& ref() const { return ref_; }
    [[nodiscard]] Node::Ptr original_node() const { return original_node_; }
    [[nodiscard]] Node::Ptr final_node() const { return ref_.node(); }
    
    // We also need to "reset" the ref to a BlockInput before running the body
    void bind_to_input(Node::Ptr input_node) {
        // This is the magic part: we construct a new Double wrapper around the input node
        // and assign it to the reference.
        ref_ = Double(std::move(input_node));
    }

private:
    Double& ref_;
    Node::Ptr original_node_;
};

} // namespace gc
