
# gradcasino

## Design and Evaluation of a JIT-Compiled Monte Carlo Pricing Engine

Monte Carlo simulations are the standard method for pricing complex, path-dependent financial derivatives. To manage risk effectively, institutions require not only the price of these derivatives but also the sensitivities ("Greeks") of these derivatives to various market parameters.

Traditionally, these sensitivities are calculated using **Finite Differences**, but this method is computationally expensive for high-dimensional models. Consequently, **Adjoint Algorithmic Differentiation (AAD)** has become the standard for efficient sensitivity analysis.

However, traditional AAD implementations often suffer from a significant memory bottleneck due to the linear growth of the recording tape. By recording the full simulation history to memory, the footprint scales linearly with the simulation length, leading to cache thrashing, performance degradation, and potentially Out Of Memory (OOM) crashes in high-precision scenarios.

The objective of this thesis is to design and implement a high-performance pricing engine that utilizes **Just-In-Time (JIT) compilation** to overcome these memory limitations. The engine will focus on generating optimized machine code designed to maximize data locality, aiming to process simulations efficiently within the CPU cache hierarchy.

### Project Steps

1. **Study the principles of AAD** and the memory limitations of current state-of-the-art frameworks (e.g., tape-based libraries, array-based frameworks) when applied to scalar-heavy stochastic differential equations.
2. **Analyze and describe possible optimization strategies** for handling differentiation graphs, focusing on the trade-offs between memory storage, re-computation, and instruction-level parallelism. Create proof-of-concept prototypes to evaluate these approaches.
3. **Implement a JIT-compiled pricing engine** based on the optimization strategies selected in the previous step.
4. **Measure performance** on a set of test scenarios varying in path length and complexity. Compare the results against standard reference implementations (e.g., Finite Differences in C++ or standard industry libraries) in terms of execution time, memory footprint, and scalability.