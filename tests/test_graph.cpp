#include <gtest/gtest.h>
#include <gradcasino/gradcasino.hpp>

class GraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        guard_ = std::make_unique<gc::Graph::ScopeGuard>();
    }
    
    void TearDown() override {
        guard_.reset();
    }
    
    std::unique_ptr<gc::Graph::ScopeGuard> guard_;
};

TEST_F(GraphTest, EmptyGraph) {
    EXPECT_TRUE(gc::Graph::current().empty());
    EXPECT_EQ(gc::Graph::current().size(), 0);
}

TEST_F(GraphTest, NodeAddition) {
    gc::Double a{1.0};
    gc::Double b{2.0};
    auto c = a + b;
    (void)c;
    
    EXPECT_EQ(gc::Graph::current().size(), 3);
    EXPECT_FALSE(gc::Graph::current().empty());
}

TEST_F(GraphTest, ForEachIteration) {
    gc::Double a{1.0};
    gc::Double b{2.0};
    auto c = a + b;
    (void)c;
    
    std::size_t count = 0;
    gc::Graph::current().for_each([&count](const gc::Node&) {
        ++count;
    });
    
    EXPECT_EQ(count, 3);
}

TEST_F(GraphTest, InputParamCounting) {
    auto x = gc::input();
    auto y = gc::input();
    auto p = gc::param(1.0);
    (void)x; (void)y; (void)p;
    
    EXPECT_EQ(gc::Graph::current().input_count(), 2);
    EXPECT_EQ(gc::Graph::current().param_count(), 1);
}

TEST(GraphScopingTest, ScopeClearsOnExit) {
    {
        auto guard = gc::Graph::scoped();
        gc::Double a{1.0};
        EXPECT_EQ(gc::Graph::current().size(), 1);
    }
    EXPECT_TRUE(gc::Graph::current().empty());
}
