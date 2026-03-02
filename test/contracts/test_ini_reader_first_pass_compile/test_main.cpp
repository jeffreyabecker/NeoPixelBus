#include <unity.h>

#include <cstring>

#include "factory/IniReader.h"

namespace
{
    void assert_span_equals(lw::span<char> actual,
                            const char *expected)
    {
        TEST_ASSERT_NOT_NULL(expected);
        const size_t expectedLength = std::strlen(expected);
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expectedLength), static_cast<uint32_t>(actual.size()));
        TEST_ASSERT_EQUAL_INT(0, std::memcmp(actual.data(), expected, expectedLength));
    }

    void test_ini_reader_parse_exists_get_and_getas(void)
    {
        char config[] =
            "[bus:front]\n"
            "pixels=120\n"
            "enabled=true\n"
            "\n"
            "[bus:rear]\n"
            "pixels=64\n"
            "enabled=0\n"
            "\n"
            "[network]\n"
            "host = controller.local\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});

        char sectionFront[] = "bus:front";
        char sectionRear[] = "bus:rear";
        char sectionMissing[] = "bus:left";

        TEST_ASSERT_TRUE(reader.exists(sectionFront));
        TEST_ASSERT_TRUE(reader.exists(sectionRear));
        TEST_ASSERT_FALSE(reader.exists(sectionMissing));

        auto front = reader.get(sectionFront);

        char keyPixels[] = "pixels";
        char keyEnabled[] = "enabled";
        char keyMissing[] = "missing";

        TEST_ASSERT_TRUE(front.exists(keyPixels));
        TEST_ASSERT_FALSE(front.exists(keyMissing));

        auto missing = front.getRaw(keyMissing);
        TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(missing.size()));

        TEST_ASSERT_EQUAL_INT(120, front.get<int>(keyPixels));
        TEST_ASSERT_TRUE(front.get<bool>(keyEnabled));

        TEST_ASSERT_EQUAL_INT(64,
                      reader.get<int>(sectionRear,
                              keyPixels));
    }

    void test_ini_reader_prefixed_reader(void)
    {
        char config[] =
            "[bus:front]\n"
            "pixels=24\n"
            "\n"
            "[bus:rear]\n"
            "pixels=30\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});

        char prefixBus[] = "bus";
        auto busReader = reader.getReader(prefixBus);

        char sectionFront[] = "front";
        char sectionRear[] = "rear";
        char sectionNetwork[] = "network";
        char keyPixels[] = "pixels";

        TEST_ASSERT_TRUE(busReader.exists(sectionFront));
        TEST_ASSERT_TRUE(busReader.exists(sectionRear));
        TEST_ASSERT_FALSE(busReader.exists(sectionNetwork));

        auto front = busReader.get(sectionFront);
        TEST_ASSERT_EQUAL_INT(24, front.get<int>(keyPixels));
    }

    void test_ini_reader_comment_rules_full_line_only(void)
    {
        char config[] =
            "; full-line semicolon comment\n"
            "# full-line hash comment\n"
            "[main]\n"
            "key=value ; This will be treated as part of the value       \n"
            "; another comment\n"
            "other=123\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});

        auto main = reader.get("main");

        TEST_ASSERT_TRUE(main.exists("key"));
        TEST_ASSERT_TRUE(main.exists("other"));

        const auto raw = main.getRaw("key");
        assert_span_equals(raw, "value ; This will be treated as part of the value");
        TEST_ASSERT_EQUAL_INT(123, main.get<int>("other"));
    }

    void test_ini_reader_get_bool_supports_all_declared_tokens(void)
    {
        char config[] =
            "[bools]\n"
            "t0=true\n"
            "t1=yes\n"
            "t2=on\n"
            "t3=1\n"
            "f0=false\n"
            "f1=no\n"
            "f2=off\n"
            "f3=0\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});
        auto bools = reader.get("bools");

        TEST_ASSERT_TRUE(bools.get<bool>("t0"));
        TEST_ASSERT_TRUE(bools.get<bool>("t1"));
        TEST_ASSERT_TRUE(bools.get<bool>("t2"));
        TEST_ASSERT_TRUE(bools.get<bool>("t3"));

        TEST_ASSERT_FALSE(bools.get<bool>("f0"));
        TEST_ASSERT_FALSE(bools.get<bool>("f1"));
        TEST_ASSERT_FALSE(bools.get<bool>("f2"));
        TEST_ASSERT_FALSE(bools.get<bool>("f3"));
    }

    void test_ini_reader_section_test_bool_missing_and_empty_are_false(void)
    {
        char config[] =
            "[flags]\n"
            "set=true\n"
            "empty=\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});
        auto flags = reader.get("flags");

        TEST_ASSERT_FALSE(flags.test<bool>("missing"));
        TEST_ASSERT_FALSE(flags.test<bool>("empty"));
        TEST_ASSERT_TRUE(flags.test<bool>("set"));
    }

    void test_ini_reader_reader_helpers_delegate_to_section(void)
    {
        char config[] =
            "[main]\n"
            "count=42\n"
            "enabled=yes\n"
            "name=alpha\n"
            "zero=0\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});

        TEST_ASSERT_TRUE(reader.exists("main", "count"));
        TEST_ASSERT_FALSE(reader.exists("main", "missing"));

        assert_span_equals(reader.getRaw("main", "name"), "alpha");
        TEST_ASSERT_EQUAL_INT(42, reader.get<int>("main", "count"));

        TEST_ASSERT_TRUE(reader.test<int>("main", "count", 42));
        TEST_ASSERT_FALSE(reader.test<int>("main", "count", 43));
        TEST_ASSERT_TRUE(reader.test<bool>("main", "enabled"));
        TEST_ASSERT_FALSE(reader.test<bool>("main", "zero"));
        TEST_ASSERT_FALSE(reader.test<bool>("main", "missing"));
    }

    void test_ini_reader_section_inheritance_cascades_with_child_override(void)
    {
        char config[] =
            "[base]\n"
            "a=1\n"
            "b=2\n"
            "\n"
            "[mid&base]\n"
            "b=20\n"
            "c=3\n"
            "\n"
            "[child&mid]\n"
            "c=30\n"
            "d=4\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});
        auto child = reader.get("child");

        TEST_ASSERT_EQUAL_INT(1, child.get<int>("a"));
        TEST_ASSERT_EQUAL_INT(20, child.get<int>("b"));
        TEST_ASSERT_EQUAL_INT(30, child.get<int>("c"));
        TEST_ASSERT_EQUAL_INT(4, child.get<int>("d"));
    }

    void test_ini_reader_section_inheritance_supports_fully_qualified_parent_with_prefix_reader(void)
    {
        char config[] =
            "[env:defaults]\n"
            "host=controller.local\n"
            "port=80\n"
            "\n"
            "[env:child&env:defaults]\n"
            "port=443\n";

        auto reader = lw::factory::IniReader::parse(lw::span<char>{config, std::strlen(config)});
        auto env = reader.getReader("env");
        auto child = env.get("child");

        assert_span_equals(child.getRaw("host"), "controller.local");
        TEST_ASSERT_EQUAL_INT(443, child.get<int>("port"));
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_ini_reader_parse_exists_get_and_getas);
    RUN_TEST(test_ini_reader_prefixed_reader);
    RUN_TEST(test_ini_reader_comment_rules_full_line_only);
    RUN_TEST(test_ini_reader_get_bool_supports_all_declared_tokens);
    RUN_TEST(test_ini_reader_section_test_bool_missing_and_empty_are_false);
    RUN_TEST(test_ini_reader_reader_helpers_delegate_to_section);
    RUN_TEST(test_ini_reader_section_inheritance_cascades_with_child_override);
    RUN_TEST(test_ini_reader_section_inheritance_supports_fully_qualified_parent_with_prefix_reader);
    return UNITY_END();
}
