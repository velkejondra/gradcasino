#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>

class WhereTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

TEST_F(WhereTest, Comparisons) {
    gc::Double a{2.0};
    gc::Double b{3.0};
    
    auto gt = a > b;
    auto lt = a < b;
    
    EXPECT_EQ(gt.node()->kind, gc::OpKind::GreaterThan);
    EXPECT_EQ(lt.node()->kind, gc::OpKind::LessThan);
}

TEST_F(WhereTest, WhereSelection) {
    auto spot = gc::input("spot");
    auto strike = gc::param(100.0, "strike");
    
    auto payoff = gc::where(spot > strike, 
                          spot - strike, 
                          gc::Double{0.0});
    
    EXPECT_EQ(payoff.node()->kind, gc::OpKind::Select);
    EXPECT_EQ(payoff.node()->inputs.size(), 3);
    EXPECT_EQ(payoff.node()->inputs[0]->kind, gc::OpKind::GreaterThan);
    EXPECT_EQ(payoff.node()->inputs[1]->kind, gc::OpKind::Sub);
    EXPECT_EQ(payoff.node()->inputs[2]->kind, gc::OpKind::Constant);
}

TEST_F(WhereTest, NamedWhere) {
    auto a = gc::input();
    auto b = gc::input();
    
    auto res = gc::where(a > b, a, b).name("max_replacement");
    
    EXPECT_EQ(res.get_name(), "max_replacement");
}
