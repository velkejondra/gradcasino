#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>
#include <gradcasino/loop.hpp>

class LoopTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

TEST_F(LoopTest, SimpleAccumulation) {
    gc::Double sum{0.0};
    
    auto results = gc::loop(10, gc::init(sum), 
        [](gc::Var i, gc::Double s) {
            return gc::vars(s + i);
        }
    );
    
    auto final_sum = std::get<0>(results);
    
    // Check graph structure
    EXPECT_EQ(final_sum.node()->kind, gc::OpKind::Loop);
    auto loop_node = final_sum.node();
    EXPECT_EQ(loop_node->inputs.size(), 1); // Initial 'sum'
    EXPECT_EQ(loop_node->iteration_count, 10);
    EXPECT_FALSE(loop_node->body.empty());
    
    // Last node in body should be Yield
    EXPECT_EQ(loop_node->body.back()->kind, gc::OpKind::Yield);
}

TEST_F(LoopTest, MultipleVariables) {
    gc::Double a{0.0};
    gc::Double b{1.0};
    
    // Fibonacci-ish: a, b -> b, a+b
    auto [final_a, final_b] = gc::loop(5, gc::init(a, b),
        [](gc::Var i, gc::Double curr_a, gc::Double curr_b) {
            (void)i;
            return gc::vars(curr_b, curr_a + curr_b);
        }
    );
    
    EXPECT_EQ(final_a.node()->kind, gc::OpKind::GetResult);
    EXPECT_EQ(final_b.node()->kind, gc::OpKind::GetResult);
    
    // Both GetResult should point to same Loop
    ASSERT_EQ(final_a.node()->inputs.size(), 1);
    ASSERT_EQ(final_b.node()->inputs.size(), 1);
    EXPECT_EQ(final_a.node()->inputs[0], final_b.node()->inputs[0]);
    EXPECT_EQ(final_a.node()->inputs[0]->kind, gc::OpKind::Loop);
}

TEST_F(LoopTest, CaptureCheck) {
    gc::Double step{2.0};
    gc::Double sum{0.0};
    
    auto [res] = gc::loop(3, gc::init(sum),
        [&](gc::Var i, gc::Double s) {
            (void)i;
            // 'step' is captured from outer scope
            return gc::vars(s + step); 
        }
    );
    
    auto loop_node = res.node();
    // Body should use outer node for 'step'
    // This is valid: region nodes can point to outer nodes.
    bool found_step = false;
    for (const auto& node : loop_node->body) {
        for (const auto& inp : node->inputs) {
            if (inp == step.node()) found_step = true;
        }
    }
    EXPECT_TRUE(found_step);
}
