#include <chrono>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/field.hpp>
#include <presentation/application/raylib-application.hpp>
#include <presentation/rendering/field-renderer.hpp>

namespace {

    using namespace std::chrono_literals;

    using lifeGame::domain::Field;
    using lifeGame::presentation::FrameInput;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::PointerSample;
    using lifeGame::presentation::RaylibApplication;

    constexpr std::size_t TRACE_FIELD_SIZE = 7;

    FrameInput inputAt(const Field& field, std::size_t x, std::size_t y, bool pressed, bool down,
                       bool released) {
        REQUIRE(x < field.width());
        REQUIRE(y < field.height());
        const auto plan = lifeGame::presentation::FieldRenderer::calculateRenderPlan(
            field, 1280, 720);
        const auto cellSize = static_cast<float>(plan.cellSize);
        return FrameInput{
            1280,
            720,
            PointerSample{
                LogicalPoint{plan.fieldRectangle.x + (static_cast<float>(x) + 0.5F) * cellSize,
                             plan.fieldRectangle.y + (static_cast<float>(y) + 0.5F) * cellSize},
                pressed,
                down,
                released,
            },
        };
    }

    std::vector<std::uint8_t> traceStateWithLive(
        std::initializer_list<std::pair<std::size_t, std::size_t>> cells) {
        auto state = std::vector<std::uint8_t>(
            TRACE_FIELD_SIZE * TRACE_FIELD_SIZE, std::uint8_t{0});
        for (const auto [x, y] : cells) {
            REQUIRE(x < TRACE_FIELD_SIZE);
            REQUIRE(y < TRACE_FIELD_SIZE);
            state[y * TRACE_FIELD_SIZE + x] = std::uint8_t{1};
        }
        return state;
    }

    TEST_CASE("A scheduled generation completes before same-iteration input") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        RaylibApplication application{std::move(field)};

        static_cast<void>(application.processIteration(
            250ms,
            FrameInput{
                1280,
                720,
                PointerSample{
                    LogicalPoint{33.0F, 1.0F},
                    true,
                    true,
                    false,
                },
            }));

        const auto& result = application.field();
        CHECK(result.isLive({0, 0}));
        CHECK(result.isLive({1, 2}));
        CHECK(result.isLive({2, 2}));
        CHECK(result.isLive({3, 2}));
        CHECK_FALSE(result.isLive({2, 1}));
        CHECK_FALSE(result.isLive({2, 3}));
    }

    TEST_CASE("An application iteration cannot edit gray out-of-field space") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        RaylibApplication application{std::move(field)};
        const auto plan = lifeGame::presentation::FieldRenderer::calculateRenderPlan(
            application.field(), 1280, 720);
        const auto outside = LogicalPoint{
            plan.fieldRectangle.x + plan.fieldRectangle.width + 1.0F,
            plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F};

        const auto processOutsideSample = [&](PointerSample pointer) {
            const auto before = application.field().cells();
            static_cast<void>(application.processIteration(
                std::chrono::milliseconds{0},
                FrameInput{1280, 720, pointer}));
            CHECK(application.field().cells() == before);
        };

        processOutsideSample(PointerSample{outside, true, true, false});
        processOutsideSample(PointerSample{outside, false, true, false});
        processOutsideSample(PointerSample{outside, false, false, true});
    }

    TEST_CASE("Identical controlled clock and input traces produce byte-identical states") {
        const auto expectedInitial = traceStateWithLive({{3, 2}, {3, 3}, {3, 4}});
        const auto expectedAfterFirst = expectedInitial;
        const auto expectedAfterSecond =
            traceStateWithLive({{0, 0}, {2, 3}, {3, 3}, {4, 3}});
        const auto expectedAfterThird =
            traceStateWithLive({{0, 0}, {1, 0}, {3, 2}, {3, 3}, {3, 4}});
        const auto expectedAfterFourth = traceStateWithLive({{2, 3}, {3, 3}, {4, 3}});

        auto firstFieldResult = Field::create(TRACE_FIELD_SIZE, TRACE_FIELD_SIZE);
        auto secondFieldResult = Field::create(TRACE_FIELD_SIZE, TRACE_FIELD_SIZE);
        REQUIRE(firstFieldResult);
        REQUIRE(secondFieldResult);
        for (auto* field : {&firstFieldResult.value(), &secondFieldResult.value()}) {
            REQUIRE(field->setLive({3, 2}, true));
            REQUIRE(field->setLive({3, 3}, true));
            REQUIRE(field->setLive({3, 4}, true));
        }

        RaylibApplication first{std::move(firstFieldResult.value())};
        RaylibApplication second{std::move(secondFieldResult.value())};
        CHECK(first.field().cells() == expectedInitial);
        CHECK(second.field().cells() == expectedInitial);

        const auto processSameStep = [&](std::chrono::milliseconds elapsed,
                                         const FrameInput& input,
                                         const std::vector<std::uint8_t>& expected) {
            static_cast<void>(first.processIteration(elapsed, input));
            static_cast<void>(second.processIteration(elapsed, input));
            CHECK(first.field().cells() == expected);
            CHECK(second.field().cells() == expected);
            CHECK(first.field().cells() == second.field().cells());
        };

        processSameStep(249ms, FrameInput{1280, 720, PointerSample{{0.0F, 0.0F}, false, false,
                                                                     false}}, expectedAfterFirst);
        processSameStep(1ms, inputAt(first.field(), 0, 0, true, true, false),
                        expectedAfterSecond);
        processSameStep(499ms, inputAt(first.field(), 1, 0, false, true, false),
                        expectedAfterThird);
        processSameStep(1ms, inputAt(first.field(), 1, 0, false, false, true),
                        expectedAfterFourth);

        auto expectedAfterReleaseAndNewPress = expectedAfterFourth;
        expectedAfterReleaseAndNewPress[6 * TRACE_FIELD_SIZE + 6] = std::uint8_t{1};
        processSameStep(0ms, inputAt(first.field(), 6, 6, true, true, false),
                        expectedAfterReleaseAndNewPress);
    }

} // namespace
