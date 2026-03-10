#include <unity.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "colors/AggregateShader.h"
#include "colors/Color.h"

namespace
{
using Color = lw::Rgbcw8Color;
using IShader = lw::IShader<Color>;
using AggregateShader = lw::AggregateShader<Color>;

class AddShader : public IShader
{
  public:
    explicit AddShader(uint8_t delta) : _delta(delta) {}

    void apply(lw::span<Color> colors) override
    {
        for (auto& color : colors)
        {
            color['R'] = static_cast<uint8_t>(color['R'] + _delta);
        }
        ++applyCount;
    }

    uint32_t applyCount{0};

  private:
    uint8_t _delta;
};

class MultiplyShader : public IShader
{
  public:
    explicit MultiplyShader(uint8_t factor) : _factor(factor) {}

    void apply(lw::span<Color> colors) override
    {
        for (auto& color : colors)
        {
            color['R'] = static_cast<uint8_t>(color['R'] * _factor);
        }
        ++applyCount;
    }

    uint32_t applyCount{0};

  private:
    uint8_t _factor;
};

class AddGreenShader : public IShader
{
  public:
    explicit AddGreenShader(uint8_t delta) : _delta(delta) {}

    void apply(lw::span<Color> colors) override
    {
        for (auto& color : colors)
        {
            color['G'] = static_cast<uint8_t>(color['G'] + _delta);
        }
    }

  private:
    uint8_t _delta;
};

class CountingOwnedShader : public IShader
{
  public:
    ~CountingOwnedShader() override { ++destructorCount; }

    void apply(lw::span<Color>) override {}

    static int destructorCount;
};

int CountingOwnedShader::destructorCount = 0;

std::vector<Color> make_frame(void)
{
    return {Color{2, 3, 4, 0, 0}, Color{5, 6, 7, 0, 0}};
}

void resetCounters(void)
{
    CountingOwnedShader::destructorCount = 0;
}

void test_4_1_1_ordered_shader_application(void)
{
    AggregateShader::SettingsType settings{};
    settings.shaders.emplace_back(std::make_unique<AddShader>(10));
    settings.shaders.emplace_back(std::make_unique<MultiplyShader>(2));

    AggregateShader shader(std::move(settings));

    auto frame = make_frame();
    shader.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_EQUAL_UINT8((2 + 10) * 2, frame[0]['R']);
    TEST_ASSERT_EQUAL_UINT8((5 + 10) * 2, frame[1]['R']);
}

void test_4_1_2_null_shader_handle_skip(void)
{
    auto add3 = std::make_unique<AddShader>(3);
    auto* add3Ptr = add3.get();

    AggregateShader::SettingsType settings{};
    settings.shaders.emplace_back(nullptr);
    settings.shaders.emplace_back(std::move(add3));
    settings.shaders.emplace_back(nullptr);

    AggregateShader shader(std::move(settings));

    auto frame = make_frame();
    shader.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_EQUAL_UINT32(1U, add3Ptr->applyCount);
    TEST_ASSERT_EQUAL_UINT8(5, frame[0]['R']);
    TEST_ASSERT_EQUAL_UINT8(8, frame[1]['R']);
}

void test_4_1_3_empty_shader_list_no_op(void)
{
    AggregateShader::SettingsType settings{};
    AggregateShader shader(std::move(settings));

    auto frame = make_frame();
    const auto original = frame;

    shader.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_TRUE(frame[0] == original[0]);
    TEST_ASSERT_TRUE(frame[1] == original[1]);
}

void test_4_2_1_owning_aggregate_shader_equivalence(void)
{
    AggregateShader::SettingsType settings{};
    settings.shaders.emplace_back(std::make_unique<AddShader>(4));
    settings.shaders.emplace_back(std::make_unique<AddGreenShader>(2));

    AggregateShader aggregate(std::move(settings));
    lw::OwningAggregateShaderT<Color, AddShader, AddGreenShader> owning(AddShader(4), AddGreenShader(2));

    auto frameA = make_frame();
    auto frameB = make_frame();

    aggregate.apply(lw::span<Color>{frameA.data(), frameA.size()});
    owning.apply(lw::span<Color>{frameB.data(), frameB.size()});

    TEST_ASSERT_TRUE(frameA[0] == frameB[0]);
    TEST_ASSERT_TRUE(frameA[1] == frameB[1]);
}

void test_4_2_2_frame_mutation_consistency_across_repeated_calls(void)
{
    AggregateShader::SettingsType settings{};
    settings.shaders.emplace_back(std::make_unique<AddShader>(2));
    settings.shaders.emplace_back(std::make_unique<MultiplyShader>(3));

    AggregateShader shader(std::move(settings));

    const auto baseline = make_frame();

    auto run1 = baseline;
    auto run2 = baseline;
    auto run3 = baseline;

    shader.apply(lw::span<Color>{run1.data(), run1.size()});
    shader.apply(lw::span<Color>{run2.data(), run2.size()});
    shader.apply(lw::span<Color>{run3.data(), run3.size()});

    TEST_ASSERT_TRUE(run1[0] == run2[0]);
    TEST_ASSERT_TRUE(run1[1] == run2[1]);
    TEST_ASSERT_TRUE(run1[0] == run3[0]);
    TEST_ASSERT_TRUE(run1[1] == run3[1]);
}

void test_4_3_1_mixed_null_valid_chain_stability(void)
{
    AggregateShader::SettingsType settings{};
    settings.shaders.emplace_back(std::make_unique<AddShader>(1));
    settings.shaders.emplace_back(nullptr);
    settings.shaders.emplace_back(std::make_unique<MultiplyShader>(2));
    settings.shaders.emplace_back(nullptr);

    AggregateShader shader(std::move(settings));

    auto frame = make_frame();
    shader.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_EQUAL_UINT8((2 + 1) * 2, frame[0]['R']);
    TEST_ASSERT_EQUAL_UINT8((5 + 1) * 2, frame[1]['R']);
}

void test_4_3_2_large_chain_performance_safety_sanity(void)
{
    AggregateShader::SettingsType settings{};
    settings.shaders.reserve(64);

    for (size_t idx = 0; idx < 64; ++idx)
    {
        settings.shaders.emplace_back(std::make_unique<AddShader>(1));
    }

    AggregateShader aggregate(std::move(settings));

    auto frame = make_frame();
    aggregate.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(2 + 64), frame[0]['R']);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(5 + 64), frame[1]['R']);
}

void test_4_3_3_aggregate_shader_deletes_owned_internals(void)
{
    resetCounters();

    {
        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(std::make_unique<CountingOwnedShader>());
        settings.shaders.emplace_back(nullptr);
        settings.shaders.emplace_back(std::make_unique<CountingOwnedShader>());

        AggregateShader shader(std::move(settings));
    }

    TEST_ASSERT_EQUAL_INT(2, CountingOwnedShader::destructorCount);
}

void test_4_4_1_add_and_remove_shader_dynamically(void)
{
    AggregateShader shader;
    auto add2 = std::make_unique<AddShader>(2);
    auto mul3 = std::make_unique<MultiplyShader>(3);
    auto* add2Ptr = add2.get();

    shader.addShader(std::move(add2));
    shader.addShader(std::move(mul3));

    auto removed = shader.removeShader(0);
    TEST_ASSERT_NOT_NULL(removed.get());
    TEST_ASSERT_EQUAL_UINT32(1U, static_cast<uint32_t>(shader.shaderCount()));

    auto frame = make_frame();
    shader.apply(lw::span<Color>{frame.data(), frame.size()});

    TEST_ASSERT_TRUE(add2Ptr->applyCount == 0);
    TEST_ASSERT_EQUAL_UINT8(2 * 3, frame[0]['R']);
    TEST_ASSERT_EQUAL_UINT8(5 * 3, frame[1]['R']);
}

void test_4_4_2_remove_shader_out_of_range_is_safe(void)
{
    AggregateShader shader;

    auto removed = shader.removeShader(4);

    TEST_ASSERT_NULL(removed.get());
    TEST_ASSERT_EQUAL_UINT32(0U, static_cast<uint32_t>(shader.shaderCount()));
}
} // namespace

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_4_1_1_ordered_shader_application);
    RUN_TEST(test_4_1_2_null_shader_handle_skip);
    RUN_TEST(test_4_1_3_empty_shader_list_no_op);
    RUN_TEST(test_4_2_1_owning_aggregate_shader_equivalence);
    RUN_TEST(test_4_2_2_frame_mutation_consistency_across_repeated_calls);
    RUN_TEST(test_4_3_1_mixed_null_valid_chain_stability);
    RUN_TEST(test_4_3_2_large_chain_performance_safety_sanity);
    RUN_TEST(test_4_3_3_aggregate_shader_deletes_owned_internals);
    RUN_TEST(test_4_4_1_add_and_remove_shader_dynamically);
    RUN_TEST(test_4_4_2_remove_shader_out_of_range_is_safe);
    return UNITY_END();
}
