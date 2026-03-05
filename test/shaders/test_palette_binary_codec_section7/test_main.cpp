#include <unity.h>

namespace
{
    void test_palette_binary_codec_placeholder(void)
    {
        // Placeholder suite until binary codec spec tests are implemented.
        TEST_ASSERT_TRUE(true);
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_palette_binary_codec_placeholder);
    return UNITY_END();
}
