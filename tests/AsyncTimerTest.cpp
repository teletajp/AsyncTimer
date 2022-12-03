#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>
#include <AsyncTimer.h>

class AsyncTimerTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(AsyncTimerTest, first)
{
    ASSERT_TRUE(true);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    // getchar();
    return ret;
}