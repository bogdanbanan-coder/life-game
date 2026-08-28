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
#include <presentation/ui/toolbar.hpp>

namespace {

    using namespace std::chrono_literals;

    using lifeGame::application::PaintMode;
    using lifeGame::application::RunState;
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

    TEST_CASE("A sparse Live drag follows a scheduled generation") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        RaylibApplication application{std::move(field)};

        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 0, 0, true, true, false)));
        CHECK(application.field().isLive({0, 0}));
        static_cast<void>(application.processIteration(
            250ms, inputAt(application.field(), 2, 0, false, true, false)));
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 2, 0, false, false, true)));
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 4, 0, true, true, false)));

        auto expected = std::vector<std::uint8_t>(25, std::uint8_t{0});
        expected[0] = std::uint8_t{1};
        expected[1] = std::uint8_t{1};
        expected[2] = std::uint8_t{1};
        expected[4] = std::uint8_t{1};
        expected[1 * 5 + 1] = std::uint8_t{1};
        expected[2 * 5 + 1] = std::uint8_t{1};
        expected[2 * 5 + 2] = std::uint8_t{1};
        expected[2 * 5 + 3] = std::uint8_t{1};
        CHECK(application.field().cells() == expected);
    }

    TEST_CASE("A paused application accepts a Live drag without advancing") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 2}, true));
        RaylibApplication application{std::move(field)};
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto pauseButton =
            toolbar.controls[lifeGame::presentation::Toolbar::RUN_CONTROL_INDEX];
        const auto pausePoint = LogicalPoint{pauseButton.x + pauseButton.width / 2.0F,
                                             pauseButton.y + pauseButton.height / 2.0F};

        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));
        REQUIRE(application.runState() == RunState::Paused);
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, false, false, true}}));
        const auto before = application.field().cells();

        static_cast<void>(application.processIteration(
            2'000ms, inputAt(application.field(), 0, 0, true, true, false)));
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 3, 0, false, true, false)));
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 4, 0, false, false, true)));

        auto expected = before;
        for (std::size_t x = 0; x <= 4; ++x) {
            expected[x] = std::uint8_t{1};
        }
        CHECK(application.runState() == RunState::Paused);
        CHECK(application.field().cells() == expected);
    }

    TEST_CASE("The application starts Running and pauses without changing the Field") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({4, 4}, true));
        RaylibApplication application{std::move(field)};
        const auto before = application.field().cells();
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto pauseButton =
            toolbar.controls[lifeGame::presentation::Toolbar::RUN_CONTROL_INDEX];
        const auto pausePoint = LogicalPoint{pauseButton.x + pauseButton.width / 2.0F,
                                             pauseButton.y + pauseButton.height / 2.0F};

        CHECK(application.runState() == RunState::Running);
        CHECK(application.paintMode() == PaintMode::Live);
        const auto dieButton =
            toolbar.controls[lifeGame::presentation::Toolbar::DIE_CONTROL_INDEX];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, true, true, false}}));
        CHECK(application.paintMode() == PaintMode::Die);

        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));

        CHECK(application.runState() == RunState::Paused);
        CHECK(application.paintMode() == PaintMode::Die);
        CHECK(application.field().cells() == before);

        static_cast<void>(application.processIteration(
            1'000ms, FrameInput{1280, 720, PointerSample{{0.0F, 0.0F}, false, false, false}}));
        CHECK(application.field().cells() == before);
    }

    TEST_CASE("A pause request follows the same-iteration scheduled generation") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        RaylibApplication application{std::move(field)};
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto pauseButton =
            toolbar.controls[lifeGame::presentation::Toolbar::RUN_CONTROL_INDEX];
        const auto pausePoint = LogicalPoint{pauseButton.x + pauseButton.width / 2.0F,
                                             pauseButton.y + pauseButton.height / 2.0F};

        static_cast<void>(application.processIteration(
            250ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));

        CHECK(application.runState() == RunState::Paused);
        CHECK(application.field().isLive({1, 2}));
        CHECK(application.field().isLive({2, 2}));
        CHECK(application.field().isLive({3, 2}));
        CHECK_FALSE(application.field().isLive({2, 1}));
        CHECK_FALSE(application.field().isLive({2, 3}));
        const auto afterPause = application.field().cells();

        static_cast<void>(application.processIteration(
            2'000ms, FrameInput{1280, 720, PointerSample{{0.0F, 0.0F}, false, false, false}}));
        CHECK(application.field().cells() == afterPause);
    }

    TEST_CASE("Resuming returns to Live and starts a fresh interval") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 2}, true));
        RaylibApplication application{std::move(field)};
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto dieButton =
            toolbar.controls[lifeGame::presentation::Toolbar::DIE_CONTROL_INDEX];
        const auto pauseButton =
            toolbar.controls[lifeGame::presentation::Toolbar::RUN_CONTROL_INDEX];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};
        const auto pausePoint = LogicalPoint{pauseButton.x + pauseButton.width / 2.0F,
                                             pauseButton.y + pauseButton.height / 2.0F};
        const auto idle = FrameInput{1280, 720,
                                     PointerSample{LogicalPoint{0.0F, 0.0F}, false, false, false}};

        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, false, false, true}}));
        static_cast<void>(application.processIteration(125ms, idle));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, false, false, true}}));
        REQUIRE(application.runState() == RunState::Paused);
        REQUIRE(application.paintMode() == PaintMode::Die);
        const auto pausedState = application.field().cells();

        static_cast<void>(application.processIteration(
            2'000ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, false, false, true}}));

        CHECK(application.runState() == RunState::Running);
        CHECK(application.paintMode() == PaintMode::Live);
        CHECK(application.field().cells() == pausedState);

        static_cast<void>(application.processIteration(249ms, idle));
        CHECK(application.field().cells() == pausedState);

        const auto expectedAfterGeneration = std::vector<std::uint8_t>(pausedState.size(), 0);
        static_cast<void>(application.processIteration(1ms, idle));
        CHECK(application.field().cells() == expectedAfterGeneration);
    }

    TEST_CASE("Resuming from Die selects Live before the next paint") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        RaylibApplication application{std::move(field)};
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto dieButton =
            toolbar.controls[lifeGame::presentation::Toolbar::DIE_CONTROL_INDEX];
        const auto pauseButton =
            toolbar.controls[lifeGame::presentation::Toolbar::RUN_CONTROL_INDEX];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};
        const auto pausePoint = LogicalPoint{pauseButton.x + pauseButton.width / 2.0F,
                                             pauseButton.y + pauseButton.height / 2.0F};

        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, false, false, true}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, false, false, true}}));
        REQUIRE(application.runState() == RunState::Paused);
        REQUIRE(application.paintMode() == PaintMode::Die);

        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{pausePoint, false, false, true}}));
        REQUIRE(application.runState() == RunState::Running);
        REQUIRE(application.paintMode() == PaintMode::Live);

        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 5, 5, true, true, false)));
        CHECK(application.field().isLive({5, 5}));
    }

    TEST_CASE("The toolbar selects Die and the application applies it to the Field") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({4, 4}, true));
        RaylibApplication application{std::move(field)};
        CHECK(application.paintMode() == PaintMode::Live);
        const auto beforeDieSelection = application.field().cells();

        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto dieButton =
            toolbar.controls[lifeGame::presentation::Toolbar::DIE_CONTROL_INDEX];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, true, true, false}}));

        CHECK(application.paintMode() == PaintMode::Die);
        const auto afterDieSelection = application.field().cells();
        CHECK(afterDieSelection == beforeDieSelection);
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 4, 4, true, true, false)));
        CHECK_FALSE(application.field().isLive({4, 4}));

        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 4, 4, false, false, true)));
        const auto beforeLiveSelection = application.field().cells();
        const auto liveButton =
            toolbar.controls[lifeGame::presentation::Toolbar::LIVE_CONTROL_INDEX];
        const auto livePoint = LogicalPoint{liveButton.x + liveButton.width / 2.0F,
                                            liveButton.y + liveButton.height / 2.0F};
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{livePoint, true, true, false}}));

        CHECK(application.paintMode() == PaintMode::Live);
        const auto afterLiveSelection = application.field().cells();
        CHECK(afterLiveSelection == beforeLiveSelection);
        static_cast<void>(application.processIteration(
            0ms, inputAt(application.field(), 5, 5, true, true, false)));
        CHECK(application.field().isLive({5, 5}));
    }

    TEST_CASE("A same-iteration Die edit follows the scheduled generation") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        RaylibApplication application{std::move(field)};

        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto dieButton =
            toolbar.controls[lifeGame::presentation::Toolbar::DIE_CONTROL_INDEX];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{diePoint, true, true, false}}));

        static_cast<void>(application.processIteration(
            250ms, inputAt(application.field(), 2, 2, true, true, false)));

        CHECK(application.field().isLive({1, 2}));
        CHECK_FALSE(application.field().isLive({2, 2}));
        CHECK(application.field().isLive({3, 2}));
        CHECK_FALSE(application.field().isLive({2, 1}));
        CHECK_FALSE(application.field().isLive({2, 3}));
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
