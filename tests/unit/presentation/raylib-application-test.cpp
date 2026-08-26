#include <chrono>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/field.hpp>
#include <presentation/application/raylib-application.hpp>

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

} // namespace
