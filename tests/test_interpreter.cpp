#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>
#include <gradcasino/interpreter.hpp>
#include <gradcasino/viz.hpp>
#include <gradcasino/loop.hpp>
#include <sstream>
#include <cmath>

class InterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

// Test basic arithmetic
TEST_F(InterpreterTest, BasicArithmetic) {
    auto a = gc::input("a");
    auto b = gc::input("b");
    auto c = gc::param(2.0, "c");
    
    auto result = (a + b) * c - gc::Double{1.0};
    
    gc::Interpreter interp;
    std::vector<double> inputs = {3.0, 4.0};  // a=3, b=4
    std::vector<double> params = {2.0};        // c=2
    
    double val = interp.eval(*result.node(), inputs, params);
    // (3 + 4) * 2 - 1 = 13
    EXPECT_DOUBLE_EQ(val, 13.0);
}

// Test math functions
TEST_F(InterpreterTest, MathFunctions) {
    auto x = gc::input("x");
    
    auto result = gc::sqrt(gc::abs(x)) + gc::exp(gc::Double{0.0}) + gc::log(gc::Double{1.0});
    
    gc::Interpreter interp;
    std::vector<double> inputs = {-4.0};
    std::vector<double> params = {};
    
    double val = interp.eval(*result.node(), inputs, params);
    // sqrt(abs(-4)) + exp(0) + log(1) = 2 + 1 + 0 = 3
    EXPECT_DOUBLE_EQ(val, 3.0);
}

// Test pow, max, min
TEST_F(InterpreterTest, BinaryFunctions) {
    auto x = gc::input("x");
    auto y = gc::input("y");
    
    auto p = gc::pow(x, y);
    auto mx = gc::max(x, y);
    auto mn = gc::min(x, y);
    auto result = p + mx + mn;
    
    gc::Interpreter interp;
    std::vector<double> inputs = {2.0, 3.0};
    std::vector<double> params = {};
    
    double val = interp.eval(*result.node(), inputs, params);
    // pow(2,3) + max(2,3) + min(2,3) = 8 + 3 + 2 = 13
    EXPECT_DOUBLE_EQ(val, 13.0);
}

// Test comparisons and select
TEST_F(InterpreterTest, Conditionals) {
    auto x = gc::input("x");
    auto threshold = gc::param(5.0, "threshold");
    
    auto cond = x > threshold;
    auto result = gc::where(cond, x * gc::Double{2.0}, x);
    
    gc::Interpreter interp;
    std::vector<double> params = {5.0};
    
    // x = 3 < 5, returns x = 3
    std::vector<double> inputs1 = {3.0};
    EXPECT_DOUBLE_EQ(interp.eval(*result.node(), inputs1, params), 3.0);
    
    // x = 7 > 5, returns x * 2 = 14
    std::vector<double> inputs2 = {7.0};
    EXPECT_DOUBLE_EQ(interp.eval(*result.node(), inputs2, params), 14.0);
}

// Test loop with single variable
TEST_F(InterpreterTest, LoopSingleVariable) {
    auto init = gc::Double{0.0};
    
    auto [result] = gc::loop(5, gc::init(init),
        [](gc::Var i, gc::Double sum) {
            (void)i;
            return gc::vars(sum + gc::Double{1.0});
        }
    );
    
    gc::Interpreter interp;
    std::vector<double> inputs = {};
    std::vector<double> params = {};
    
    double val = interp.eval(*result.node(), inputs, params);
    // Sum from 0, adding 1 five times = 5
    EXPECT_DOUBLE_EQ(val, 5.0);
}

// Test loop with multiple variables (Fibonacci)
TEST_F(InterpreterTest, LoopMultipleVariables) {
    auto a = gc::Double{0.0};
    auto b = gc::Double{1.0};
    
    auto [final_a, final_b] = gc::loop(6, gc::init(a, b),
        [](gc::Var i, gc::Double curr_a, gc::Double curr_b) {
            (void)i;
            return gc::vars(curr_b, curr_a + curr_b);
        }
    );
    
    gc::Interpreter interp;
    std::vector<double> inputs = {};
    std::vector<double> params = {};
    
    // Fibonacci: 0,1 -> 1,1 -> 1,2 -> 2,3 -> 3,5 -> 5,8 -> 8,13
    double val_a = interp.eval(*final_a.node(), inputs, params);
    double val_b = interp.eval(*final_b.node(), inputs, params);
    
    EXPECT_DOUBLE_EQ(val_a, 8.0);
    EXPECT_DOUBLE_EQ(val_b, 13.0);
}

// Comprehensive test: Damped oscillator with barrier (uses ALL OpKinds except Grad)
TEST_F(InterpreterTest, DampedOscillatorWithBarrier) {
    auto x0 = gc::input("x0");        // Initial position
    auto v0 = gc::input("v0");        // Initial velocity
    auto dt = gc::param(0.1, "dt");   // Time step
    auto k = gc::param(1.0, "k");     // Spring constant
    auto damp = gc::param(0.1, "damp"); // Damping
    auto barrier = gc::param(1.5, "barrier");
    
    auto [final_x, final_v] = gc::loop(10, gc::init(x0, v0),
        [&](gc::Var t, gc::Double x, gc::Double v) {
            (void)t;
            // Acceleration: F = -kx - bv
            auto accel = -k * x - damp * v;
            auto new_v = v + accel * dt;
            auto new_x = x + new_v * dt;
            
            // Clamp position to [-barrier, barrier]
            auto clamped = gc::where(new_x > barrier, barrier,
                           gc::where(new_x < -barrier, -barrier, new_x));
            
            return gc::vars(clamped, new_v);
        }
    );
    
    // Compute kinetic + potential energy
    auto energy = gc::Double{0.5} * k * final_x * final_x 
                + gc::Double{0.5} * final_v * final_v;
    
    // Log of energy (tests log, abs, add with small epsilon)
    auto log_energy = gc::log(gc::abs(energy) + gc::Double{1e-10});
    
    gc::Interpreter interp;
    std::vector<double> inputs = {1.0, 0.0};  // Start at x=1, v=0
    std::vector<double> params = {0.1, 1.0, 0.1, 1.5};  // dt, k, damp, barrier
    
    double final_pos = interp.eval(*final_x.node(), inputs, params);
    double final_vel = interp.eval(*final_v.node(), inputs, params);
    double e = interp.eval(*energy.node(), inputs, params);
    double log_e = interp.eval(*log_energy.node(), inputs, params);
    
    // Just verify it runs and produces sensible values
    EXPECT_TRUE(std::isfinite(final_pos));
    EXPECT_TRUE(std::isfinite(final_vel));
    EXPECT_TRUE(std::isfinite(e));
    EXPECT_TRUE(std::isfinite(log_e));
    EXPECT_GE(e, 0.0);  // Energy should be non-negative
    
    // Position should be within barrier
    EXPECT_LE(std::abs(final_pos), 1.5 + 0.01);
}

// Test tracing callback
TEST_F(InterpreterTest, TracingCallback) {
    auto a = gc::input("a");
    auto b = gc::input("b");
    auto result = a + b;
    
    std::vector<std::pair<std::uint32_t, double>> trace;
    
    gc::Interpreter interp({
        .on_eval = [&](const gc::Node& node, double val) {
            trace.emplace_back(node.id, val);
        }
    });
    
    std::vector<double> inputs = {3.0, 4.0};
    std::vector<double> params = {};
    
    double val = interp.eval(*result.node(), inputs, params);
    EXPECT_DOUBLE_EQ(val, 7.0);
    
    // Should have traced: Input(a), Input(b), Add
    EXPECT_GE(trace.size(), 3);
}

// Test DOT export
TEST_F(InterpreterTest, DotExport) {
    auto x = gc::input("x");
    auto y = gc::input("y");
    auto result = gc::max(x + y, gc::Double{0.0});
    
    std::stringstream ss;
    std::vector<gc::Node::Ptr> roots = {result.node()};
    gc::export_dot(roots, ss);
    
    std::string dot = ss.str();
    EXPECT_TRUE(dot.find("digraph G") != std::string::npos);
    EXPECT_TRUE(dot.find("x") != std::string::npos);
    EXPECT_TRUE(dot.find("y") != std::string::npos);
}

// Test interpreter backend via compile()
TEST_F(InterpreterTest, CompileWithInterpreterBackend) {
    auto x = gc::input("x");
    auto y = gc::input("y");
    
    auto sum = x + y;
    auto product = x * y;
    
    auto kernel = gc::compile(
        gc::outputs(sum, product),
        gc::inputs(x, y),
        gc::CompileOptions{.backend = gc::Backend::Interpreter}
    );
    
    ASSERT_TRUE(kernel.has_value());
    
    std::vector<double> xs = {1.0, 2.0, 3.0};
    std::vector<double> ys = {4.0, 5.0, 6.0};
    
    auto result = (*kernel)(xs, ys);
    ASSERT_TRUE(result.has_value());
    
    auto& [sums, products] = *result;
    
    ASSERT_EQ(sums.size(), 3);
    ASSERT_EQ(products.size(), 3);
    
    EXPECT_DOUBLE_EQ(sums[0], 5.0);   // 1+4
    EXPECT_DOUBLE_EQ(sums[1], 7.0);   // 2+5
    EXPECT_DOUBLE_EQ(sums[2], 9.0);   // 3+6
    
    EXPECT_DOUBLE_EQ(products[0], 4.0);   // 1*4
    EXPECT_DOUBLE_EQ(products[1], 10.0);  // 2*5
    EXPECT_DOUBLE_EQ(products[2], 18.0);  // 3*6
}
