#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>
#include <vector>

class CompileTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

TEST_F(CompileTest, CompileSucceeds) {
    auto spot = gc::input("spot");
    auto dW = gc::input("dW");
    auto vol = gc::param(0.2, "vol");
    auto strike = gc::param(100.0, "strike");
    
    auto payoff = gc::max(spot * gc::exp(vol * dW) - strike, gc::Double{0.0});
    payoff.name("payoff");
    
    auto delta = gc::grad(payoff, spot);
    delta.name("delta");
    
    auto kernel = gc::compile(
        gc::outputs(payoff, delta),
        gc::inputs(spot, dW)
    );
    
    EXPECT_TRUE(kernel.has_value());
}

TEST_F(CompileTest, KernelExecution) {
    auto spot = gc::input("spot");
    auto dW = gc::input("dW");
    auto vol = gc::param(0.2, "vol");
    auto strike = gc::param(100.0, "strike");
    
    auto payoff = gc::max(spot * gc::exp(vol * dW) - strike, gc::Double{0.0});
    auto delta = gc::grad(payoff, spot);
    
    auto kernel = gc::compile(
        gc::outputs(payoff, delta),
        gc::inputs(spot, dW)
    );
    
    std::vector<double> spots(100, 100.0);
    std::vector<double> dWs(100, 0.1);
    
    auto result = kernel(spots, dWs);
    ASSERT_TRUE(result.has_value());
    
    auto& [prices, deltas] = *result;
    EXPECT_EQ(prices.size(), 100);
    EXPECT_EQ(deltas.size(), 100);
}

TEST_F(CompileTest, EmptyOutputsFails) {
    auto spot = gc::input();
    
    auto kernel = gc::compile(
        gc::outputs(),
        gc::inputs(spot)
    );
    
    EXPECT_FALSE(kernel.has_value());
}

TEST_F(CompileTest, EmptyInputsFails) {
    auto val = gc::Double{1.0};
    
    auto kernel = gc::compile(
        gc::outputs(val),
        gc::inputs()
    );
    
    EXPECT_FALSE(kernel.has_value());
}
