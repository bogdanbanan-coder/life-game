#include <chrono>
#include <utility>

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

} // namespace
