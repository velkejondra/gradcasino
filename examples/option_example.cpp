#include <gradcasino/gradcasino.hpp>
#include <gradcasino/viz.hpp>
#include <gradcasino/loop.hpp>
#include <fstream>
#include <iostream>

int main() {
    auto guard = gc::Graph::scoped();
    
    // === European Call Option with Monte Carlo ===
    // Parameters
    auto spot = gc::input("S₀");
    auto dW = gc::input("dW");
    
    auto strike = gc::param(100.0, "K");
    auto vol = gc::param(0.2, "σ");
    auto rate = gc::param(0.05, "r");
    auto T = gc::param(1.0, "T");
    
    // Geometric Brownian Motion: S_T = S₀ * exp((r - σ²/2)*T + σ*√T*dW)
    auto drift = (rate - gc::Double{0.5} * vol * vol) * T;
    auto diffusion = vol * gc::sqrt(T) * dW;
    auto S_T = spot * gc::exp(drift + diffusion);
    S_T.name("S_T");
    
    // Payoff: max(S_T - K, 0)
    auto intrinsic = S_T - strike;
    auto payoff = gc::max(intrinsic, gc::Double{0.0});
    payoff.name("payoff");
    
    // Discounted payoff
    auto discount = gc::exp(-rate * T);
    discount.name("discount");
    auto pv = payoff * discount;
    pv.name("PV");
    
    auto delta = gc::grad(pv, spot);
    delta.name("Δ");
    
    // === Export to all formats ===
    
    // DOT (Graphviz)
    std::ofstream dot_file("option_model.dot");
    gc::export_dot(gc::Graph::current(), dot_file);
    dot_file.close();
    
    std::ofstream mermaid_file("option_model.mermaid");
    gc::export_mermaid(gc::Graph::current(), mermaid_file);
    mermaid_file.close();
    
    std::ofstream cyto_file("option_model.cytoscape.json");
    gc::export_cytoscape(gc::Graph::current(), cyto_file);
    cyto_file.close();
    
    std::cout << "Exported to:\n";
    std::cout << "  - option_model.dot         (run: dot -Tpng option_model.dot -o option_model.png)\n";
    std::cout << "  - option_model.mermaid     (paste into GitHub markdown with ```mermaid fence)\n";
    std::cout << "  - option_model.cytoscape.json (load in Cytoscape.js viewer)\n";
    
    return 0;
}
