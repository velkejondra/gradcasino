#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>

class DoubleTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

TEST_F(DoubleTest, ConstantCreation) {
    gc::Double a{2.0};
    gc::Double b{3.0};
    
    EXPECT_EQ(gc::Graph::current().size(), 2);
    EXPECT_DOUBLE_EQ(a.value(), 2.0);
    EXPECT_DOUBLE_EQ(b.value(), 3.0);
}

TEST_F(DoubleTest, ArithmeticOperations) {
    gc::Double a{2.0};
    gc::Double b{3.0};
    auto c = a + b;
    auto d = c * a;
    
    EXPECT_EQ(gc::Graph::current().size(), 4);
}

TEST_F(DoubleTest, MathFunctions) {
    gc::Double a{2.0};
    gc::Double b{3.0};
    
    auto e = gc::exp(a);
    auto f = gc::sqrt(b);
    auto g = gc::max(e, f);
    
    EXPECT_EQ(gc::Graph::current().size(), 5);
    EXPECT_EQ(e.node()->kind, gc::OpKind::Exp);
    EXPECT_EQ(f.node()->kind, gc::OpKind::Sqrt);
    EXPECT_EQ(g.node()->kind, gc::OpKind::Max);
}

TEST_F(DoubleTest, MixedArithmetic) {
    gc::Double x{10.0};
    
    auto y = x + 5.0;
    auto z = 2.0 * y;
    auto w = z / 2;
    (void)w;
    
    EXPECT_EQ(gc::Graph::current().size(), 7);
}

TEST_F(DoubleTest, NamedInput) {
    auto spot = gc::input("spot");
    
    EXPECT_EQ(spot.get_name(), "spot");
    EXPECT_EQ(spot.node()->display_name(), "spot");
}

TEST_F(DoubleTest, NamedParam) {
    auto vol = gc::param(0.2, "vol");
    
    EXPECT_EQ(vol.get_name(), "vol");
    EXPECT_DOUBLE_EQ(vol.value(), 0.2);
}

TEST_F(DoubleTest, FluentNaming) {
    auto a = gc::input();
    auto b = gc::input();
    auto sum = (a + b).name("sum");
    
    EXPECT_EQ(sum.get_name(), "sum");
}

TEST_F(DoubleTest, UnnamedDisplayName) {
    auto x = gc::input();
    
    EXPECT_FALSE(x.get_name().has_value());
    EXPECT_TRUE(x.node()->display_name().starts_with("Input_"));
}
