#pragma once
// AI generated visualizations

#include "node.hpp"
#include "graph.hpp"
#include <ostream>
#include <span>
#include <unordered_set>

namespace gc {

/// Export a graph to DOT format for visualization with Graphviz.
/// Usage: dot -Tpng graph.dot -o graph.png
void export_dot(const Graph& g, std::ostream& out);

/// Export subgraph reachable from given root nodes
void export_dot(std::span<const Node::Ptr> roots, std::ostream& out);

namespace detail {

inline void collect_nodes(const Node::Ptr& node, 
                          std::unordered_set<std::uint32_t>& visited,
                          std::vector<const Node*>& nodes) {
    if (!node || visited.count(node->id)) return;
    visited.insert(node->id);
    
    for (const auto& input : node->inputs) {
        collect_nodes(input, visited, nodes);
    }
    for (const auto& body_node : node->body) {
        collect_nodes(body_node, visited, nodes);
    }
    for (const auto& arg : node->args) {
        collect_nodes(arg, visited, nodes);
    }
    
    nodes.push_back(node.get());
}

inline const char* op_color(OpKind kind) {
    switch (kind) {
        case OpKind::Constant: return "lightblue";
        case OpKind::Input:    return "lightgreen";
        case OpKind::Param:    return "lightyellow";
        case OpKind::Loop:     return "orange";
        case OpKind::Select:   return "pink";
        case OpKind::Grad:     return "red";
        default:               return "white";
    }
}

inline const char* op_shape(OpKind kind) {
    switch (kind) {
        case OpKind::Constant:
        case OpKind::Input:
        case OpKind::Param:    return "ellipse";
        case OpKind::Loop:     return "box3d";
        case OpKind::Yield:    return "house";
        case OpKind::BlockInput: return "invhouse";
        default:               return "box";
    }
}

inline std::string op_symbol(const Node& node) {
    std::string op;
    
    switch (node.kind) {
        case OpKind::Add: op = "+"; break;
        case OpKind::Sub: op = "-"; break;
        case OpKind::Mul: op = "×"; break;
        case OpKind::Div: op = "÷"; break;
        case OpKind::Neg: op = "-"; break;
        case OpKind::Sqrt: op = "√"; break;
        case OpKind::Exp: op = "exp"; break;
        case OpKind::Log: op = "ln"; break;
        case OpKind::Abs: op = "| |"; break;
        case OpKind::Pow: op = "^"; break;
        case OpKind::Max: op = "max"; break;
        case OpKind::Min: op = "min"; break;
        case OpKind::GreaterThan: op = ">"; break;
        case OpKind::LessThan: op = "<"; break;
        case OpKind::Select: op = "?:"; break;
        case OpKind::Grad: op = "∇"; break;
        case OpKind::Constant: op = "const"; break;
        case OpKind::Input: op = "input"; break;
        case OpKind::Param: op = "param"; break;
        default: op = std::string(op_name(node.kind)); break;
    }
    
    if (node.name) {
        return op + "\\n" + *node.name;
    }
    return op;
}

inline const char* edge_label(OpKind kind, std::size_t index) {
    switch (kind) {
        case OpKind::Pow:
            return index == 0 ? "base" : "exp";
        case OpKind::Div:
            return index == 0 ? "num" : "den";
        case OpKind::Sub:
            return index == 0 ? "lhs" : "rhs";
        case OpKind::Select:
            if (index == 0) return "cond";
            if (index == 1) return "true";
            return "false";
        default:
            return nullptr;
    }
}

} // namespace detail

inline void export_dot(std::span<const Node::Ptr> roots, std::ostream& out) {
    std::unordered_set<std::uint32_t> visited;
    std::vector<const Node*> nodes;
    
    for (const auto& root : roots) {
        detail::collect_nodes(root, visited, nodes);
    }
    
    out << "digraph G {\n";
    out << "  bgcolor=\"#0d1117\";\n";
    out << "  rankdir=TB;\n";
    out << "  splines=curved;\n";
    out << "  nodesep=0.6;\n";
    out << "  ranksep=0.8;\n";
    out << "  pad=0.5;\n";
    
    out << "  node [\n";
    out << "    fontname=\"SF Pro Display, Helvetica Neue, Arial\"\n";
    out << "    fontsize=11\n";
    out << "    style=\"filled,rounded\"\n";
    out << "    penwidth=0\n";
    out << "    margin=\"0.15,0.08\"\n";
    out << "  ];\n";
    
    out << "  edge [\n";
    out << "    color=\"#30363d\"\n";
    out << "    arrowsize=0.7\n";
    out << "    penwidth=1.5\n";
    out << "  ];\n";
    
    for (const auto* node : nodes) {
        out << "  n" << node->id << " [label=\"" << detail::op_symbol(*node) << "\"";
        out << " style=filled fillcolor=\"" << detail::op_color(node->kind) << "\"";
        out << " shape=\"" << detail::op_shape(node->kind) << "\"";
        out << "];\n";
        
        for (std::size_t i = 0; i < node->inputs.size(); ++i) {
            out << "  n" << node->inputs[i]->id << " -> n" << node->id;
            const char* label = detail::edge_label(node->kind, i);
            if (label) {
                out << " [label=\"" << label << "\" fontcolor=\"#8b949e\" fontsize=9]";
            } else if (node->inputs.size() > 1) {
                out << " [label=\"" << i << "\" fontcolor=\"#8b949e\" fontsize=9]";
            }
            out << ";\n";
        }
        
        // Draw body edges for loops
        if (node->kind == OpKind::Loop) {
            for (const auto& body_node : node->body) {
                out << "  n" << body_node->id << " -> n" << node->id;
                out << " [style=dashed color=\"#8957e5\" label=\"body\" fontcolor=\"#8957e5\" fontsize=9];\n";
            }
        }
    }
    
    out << "}\n";
}

inline void export_dot(const Graph& g, std::ostream& out) {
    out << "digraph G {\n";
    out << "  bgcolor=\"#0d1117\";\n";
    out << "  rankdir=TB;\n";
    out << "  splines=curved;\n";
    out << "  nodesep=0.6;\n";
    out << "  ranksep=0.8;\n";
    out << "  pad=0.5;\n";
    
    out << "  node [\n";
    out << "    fontname=\"SF Pro Display, Helvetica Neue, Arial\"\n";
    out << "    fontsize=11\n";
    out << "    style=\"filled,rounded\"\n";
    out << "    penwidth=0\n";
    out << "    margin=\"0.15,0.08\"\n";
    out << "  ];\n";
    
    out << "  edge [\n";
    out << "    color=\"#30363d\"\n";
    out << "    arrowsize=0.7\n";
    out << "    penwidth=1.5\n";
    out << "  ];\n";
    
    g.for_each([&](const Node& node) {
        out << "  n" << node.id << " [label=\"" << detail::op_symbol(node) << "\"";
        out << " style=filled fillcolor=\"" << detail::op_color(node.kind) << "\"";
        out << " shape=\"" << detail::op_shape(node.kind) << "\"";
        out << "];\n";
        
        for (std::size_t i = 0; i < node.inputs.size(); ++i) {
            out << "  n" << node.inputs[i]->id << " -> n" << node.id;
            const char* label = detail::edge_label(node.kind, i);
            if (label) {
                out << " [label=\"" << label << "\" fontcolor=\"#8b949e\" fontsize=9]";
            } else if (node.inputs.size() > 1) {
                out << " [label=\"" << i << "\" fontcolor=\"#8b949e\" fontsize=9]";
            }
            out << ";\n";
        }
    });
    
    out << "}\n";
}

// ============================================================================
// Mermaid Export
// ============================================================================

/// Export graph to Mermaid format
/// Embed in markdown: ```mermaid\n<output>\n```
inline void export_mermaid(const Graph& g, std::ostream& out) {
    out << "flowchart TD\n";
    
    g.for_each([&](const Node& node) {
        // Node shape based on kind
        std::string shape_start, shape_end;
        switch (node.kind) {
            case OpKind::Constant:
            case OpKind::Input:
            case OpKind::Param:
                shape_start = "(("; shape_end = "))"; // Circle
                break;
            case OpKind::Loop:
                shape_start = "[[["; shape_end = "]]]"; // Subroutine
                break;
            case OpKind::Select:
                shape_start = "{"; shape_end = "}"; // Diamond
                break;
            default:
                shape_start = "["; shape_end = "]"; // Rectangle
        }
        
        out << "    n" << node.id << shape_start << "\"" << detail::op_symbol(node) << "\"" << shape_end << "\n";
        
        for (const auto& input : node.inputs) {
            out << "    n" << input->id << " --> n" << node.id << "\n";
        }
    });
    
    // Add styling
    out << "\n    %% Styling\n";
    g.for_each([&](const Node& node) {
        const char* color = nullptr;
        switch (node.kind) {
            case OpKind::Constant: color = "#a8d5ff"; break;
            case OpKind::Input:    color = "#a8ffa8"; break;
            case OpKind::Param:    color = "#ffffa8"; break;
            case OpKind::Loop:     color = "#ffcc80"; break;
            case OpKind::Select:   color = "#ffb6c1"; break;
            case OpKind::Grad:     color = "#ff8080"; break;
            default: break;
        }
        if (color) {
            out << "    style n" << node.id << " fill:" << color << "\n";
        }
    });
}

inline void export_mermaid(std::span<const Node::Ptr> roots, std::ostream& out) {
    std::unordered_set<std::uint32_t> visited;
    std::vector<const Node*> nodes;
    
    for (const auto& root : roots) {
        detail::collect_nodes(root, visited, nodes);
    }
    
    out << "flowchart TD\n";
    
    for (const auto* node : nodes) {
        std::string shape_start, shape_end;
        switch (node->kind) {
            case OpKind::Constant:
            case OpKind::Input:
            case OpKind::Param:
                shape_start = "(("; shape_end = "))";
                break;
            case OpKind::Loop:
                shape_start = "[[["; shape_end = "]]]";
                break;
            case OpKind::Select:
                shape_start = "{"; shape_end = "}";
                break;
            default:
                shape_start = "["; shape_end = "]";
        }
        
        out << "    n" << node->id << shape_start << "\"" << detail::op_symbol(*node) << "\"" << shape_end << "\n";
        
        for (const auto& input : node->inputs) {
            out << "    n" << input->id << " --> n" << node->id << "\n";
        }
    }
}

// ============================================================================
// Cytoscape.js Export (for interactive web visualization)
// ============================================================================

/// Export graph to Cytoscape.js JSON format
/// Use with: cy.add(JSON.parse(output))
inline void export_cytoscape(const Graph& g, std::ostream& out) {
    out << "{\n  \"elements\": {\n    \"nodes\": [\n";
    
    bool first_node = true;
    g.for_each([&](const Node& node) {
        if (!first_node) out << ",\n";
        first_node = false;
        
        out << "      { \"data\": { ";
        out << "\"id\": \"n" << node.id << "\", ";
        out << "\"label\": \"" << detail::op_symbol(node) << "\", ";
        out << "\"kind\": \"" << op_name(node.kind) << "\"";
        out << " } }";
    });
    
    out << "\n    ],\n    \"edges\": [\n";
    
    bool first_edge = true;
    g.for_each([&](const Node& node) {
        for (std::size_t i = 0; i < node.inputs.size(); ++i) {
            if (!first_edge) out << ",\n";
            first_edge = false;
            
            out << "      { \"data\": { ";
            out << "\"id\": \"e" << node.inputs[i]->id << "_" << node.id << "\", ";
            out << "\"source\": \"n" << node.inputs[i]->id << "\", ";
            out << "\"target\": \"n" << node.id << "\"";
            if (node.inputs.size() > 1) {
                out << ", \"label\": \"" << i << "\"";
            }
            out << " } }";
        }
    });
    
    out << "\n    ]\n  },\n";
    
    // Include a default stylesheet
    out << "  \"style\": [\n";
    out << "    { \"selector\": \"node\", \"style\": { \"label\": \"data(label)\", \"background-color\": \"#666\", \"color\": \"#fff\", \"shape\": \"roundrectangle\" } },\n";
    out << "    { \"selector\": \"node[kind='Constant']\", \"style\": { \"background-color\": \"#6db3f2\", \"shape\": \"ellipse\" } },\n";
    out << "    { \"selector\": \"node[kind='Input']\", \"style\": { \"background-color\": \"#5cb85c\", \"shape\": \"ellipse\" } },\n";
    out << "    { \"selector\": \"node[kind='Param']\", \"style\": { \"background-color\": \"#f0ad4e\", \"shape\": \"ellipse\" } },\n";
    out << "    { \"selector\": \"node[kind='Grad']\", \"style\": { \"background-color\": \"#d9534f\" } },\n";
    out << "    { \"selector\": \"node[kind='Loop']\", \"style\": { \"background-color\": \"#ff9800\" } },\n";
    out << "    { \"selector\": \"edge\", \"style\": { \"width\": 2, \"line-color\": \"#999\", \"target-arrow-shape\": \"triangle\", \"curve-style\": \"bezier\" } }\n";
    out << "  ]\n";
    out << "}\n";
}

inline void export_cytoscape(std::span<const Node::Ptr> roots, std::ostream& out) {
    std::unordered_set<std::uint32_t> visited;
    std::vector<const Node*> nodes;
    
    for (const auto& root : roots) {
        detail::collect_nodes(root, visited, nodes);
    }
    
    out << "{\n  \"elements\": {\n    \"nodes\": [\n";
    
    bool first_node = true;
    for (const auto* node : nodes) {
        if (!first_node) out << ",\n";
        first_node = false;
        
        out << "      { \"data\": { ";
        out << "\"id\": \"n" << node->id << "\", ";
        out << "\"label\": \"" << detail::op_symbol(*node) << "\", ";
        out << "\"kind\": \"" << op_name(node->kind) << "\"";
        out << " } }";
    }
    
    out << "\n    ],\n    \"edges\": [\n";
    
    bool first_edge = true;
    for (const auto* node : nodes) {
        for (std::size_t i = 0; i < node->inputs.size(); ++i) {
            if (!first_edge) out << ",\n";
            first_edge = false;
            
            out << "      { \"data\": { ";
            out << "\"id\": \"e" << node->inputs[i]->id << "_" << node->id << "\", ";
            out << "\"source\": \"n" << node->inputs[i]->id << "\", ";
            out << "\"target\": \"n" << node->id << "\"";
            if (node->inputs.size() > 1) {
                out << ", \"label\": \"" << i << "\"";
            }
            out << " } }";
        }
    }
    
    out << "\n    ]\n  }\n}\n";
}

} // namespace gc
