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
    using Color = npb::Rgbcw8Color;
    using IShader = npb::IShader<Color>;
    using AggregateShader = npb::AggregateShader<Color>;

    class AddShader : public IShader
    {
    public:
        explicit AddShader(uint8_t delta)
            : _delta(delta)
        {
        }

        void apply(npb::span<Color> colors) override
        {
            for (auto &color : colors)
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
        explicit MultiplyShader(uint8_t factor)
            : _factor(factor)
        {
        }

        void apply(npb::span<Color> colors) override
        {
            for (auto &color : colors)
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
        explicit AddGreenShader(uint8_t delta)
            : _delta(delta)
        {
        }

        void apply(npb::span<Color> colors) override
        {
            for (auto &color : colors)
            {
                color['G'] = static_cast<uint8_t>(color['G'] + _delta);
            }
        }

    private:
        uint8_t _delta;
    };

    std::vector<Color> make_frame(void)
    {
        return {
            Color{2, 3, 4, 0, 0},
            Color{5, 6, 7, 0, 0}};
    }

    void test_4_1_1_ordered_shader_application(void)
    {
        AddShader add10(10);
        MultiplyShader mul2(2);

        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(&add10);
        settings.shaders.emplace_back(&mul2);

        AggregateShader shader(std::move(settings));

        auto frame = make_frame();
        shader.apply(npb::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8((2 + 10) * 2, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8((5 + 10) * 2, frame[1]['R']);
    }

    void test_4_1_2_null_shader_handle_skip(void)
    {
        AddShader add3(3);

        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(nullptr);
        settings.shaders.emplace_back(&add3);
        settings.shaders.emplace_back(nullptr);

        AggregateShader shader(std::move(settings));

        auto frame = make_frame();
        shader.apply(npb::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT32(1U, add3.applyCount);
        TEST_ASSERT_EQUAL_UINT8(5, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(8, frame[1]['R']);
    }

    void test_4_1_3_empty_shader_list_no_op(void)
    {
        AggregateShader::SettingsType settings{};
        AggregateShader shader(std::move(settings));

        auto frame = make_frame();
        const auto original = frame;

        shader.apply(npb::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_TRUE(frame[0] == original[0]);
        TEST_ASSERT_TRUE(frame[1] == original[1]);
    }

    void test_4_2_1_owning_aggregate_shader_equivalence(void)
    {
        AddShader add4(4);
        AddGreenShader addG2(2);

        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(&add4);
        settings.shaders.emplace_back(&addG2);

        AggregateShader aggregate(std::move(settings));
        npb::OwningAggregateShaderT<Color, AddShader, AddGreenShader> owning(AddShader(4), AddGreenShader(2));

        auto frameA = make_frame();
        auto frameB = make_frame();

        aggregate.apply(npb::span<Color>{frameA.data(), frameA.size()});
        owning.apply(npb::span<Color>{frameB.data(), frameB.size()});

        TEST_ASSERT_TRUE(frameA[0] == frameB[0]);
        TEST_ASSERT_TRUE(frameA[1] == frameB[1]);
    }

    void test_4_2_2_frame_mutation_consistency_across_repeated_calls(void)
    {
        AddShader add2(2);
        MultiplyShader mul3(3);

        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(&add2);
        settings.shaders.emplace_back(&mul3);

        AggregateShader shader(std::move(settings));

        const auto baseline = make_frame();

        auto run1 = baseline;
        auto run2 = baseline;
        auto run3 = baseline;

        shader.apply(npb::span<Color>{run1.data(), run1.size()});
        shader.apply(npb::span<Color>{run2.data(), run2.size()});
        shader.apply(npb::span<Color>{run3.data(), run3.size()});

        TEST_ASSERT_TRUE(run1[0] == run2[0]);
        TEST_ASSERT_TRUE(run1[1] == run2[1]);
        TEST_ASSERT_TRUE(run1[0] == run3[0]);
        TEST_ASSERT_TRUE(run1[1] == run3[1]);
    }

    void test_4_3_1_mixed_null_valid_chain_stability(void)
    {
        AddShader validA(1);
        MultiplyShader validB(2);

        AggregateShader::SettingsType settings{};
        settings.shaders.emplace_back(&validA);
        settings.shaders.emplace_back(nullptr);
        settings.shaders.emplace_back(&validB);
        settings.shaders.emplace_back(nullptr);

        AggregateShader shader(std::move(settings));

        auto frame = make_frame();
        shader.apply(npb::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8((2 + 1) * 2, frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8((5 + 1) * 2, frame[1]['R']);
    }

    void test_4_3_2_large_chain_performance_safety_sanity(void)
    {
        AggregateShader::SettingsType settings{};
        std::vector<std::unique_ptr<AddShader>> ownedShaders{};
        ownedShaders.reserve(64);
        settings.shaders.reserve(64);

        for (size_t idx = 0; idx < 64; ++idx)
        {
            auto shader = std::make_unique<AddShader>(1);
            settings.shaders.emplace_back(shader.get());
            ownedShaders.emplace_back(std::move(shader));
        }

        AggregateShader aggregate(std::move(settings));

        auto frame = make_frame();
        aggregate.apply(npb::span<Color>{frame.data(), frame.size()});

        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(2 + 64), frame[0]['R']);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(5 + 64), frame[1]['R']);
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
    RUN_TEST(test_4_1_1_ordered_shader_application);
    RUN_TEST(test_4_1_2_null_shader_handle_skip);
    RUN_TEST(test_4_1_3_empty_shader_list_no_op);
    RUN_TEST(test_4_2_1_owning_aggregate_shader_equivalence);
    RUN_TEST(test_4_2_2_frame_mutation_consistency_across_repeated_calls);
    RUN_TEST(test_4_3_1_mixed_null_valid_chain_stability);
    RUN_TEST(test_4_3_2_large_chain_performance_safety_sanity);
    return UNITY_END();
}
