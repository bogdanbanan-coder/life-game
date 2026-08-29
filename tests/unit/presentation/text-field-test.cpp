#include <string>

#include <catch2/catch_test_macros.hpp>
#include <presentation/ui/text-field.hpp>

namespace {

    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::TextField;
    using lifeGame::presentation::TextInput;

    TEST_CASE("TextField keeps UTF-8 code points whole at its byte limit") {
        const std::string globe = "\xF0\x9F\x8C\x8D";
        TextField field{4};

        field.setText(globe + "a");
        CHECK(field.text() == globe);

        field.setText({});
        TextInput input;
        input.characters = globe + "a";
        CHECK(field.handleInput(Rectangle{0.0F, 0.0F, 100.0F, 24.0F}, LogicalPoint{1.0F, 1.0F},
                                 true, input));
        CHECK(field.text() == globe);
    }

    TEST_CASE("TextField backspace removes one Unicode code point and setText resets focus") {
        const std::string globe = "\xF0\x9F\x8C\x8D";
        TextField field{16};
        const auto bounds = Rectangle{0.0F, 0.0F, 100.0F, 24.0F};

        field.setText(globe + "a");
        CHECK_FALSE(field.handleInput(bounds, LogicalPoint{1.0F, 1.0F}, true, TextInput{}));
        TextInput backspace;
        backspace.backspace = true;
        CHECK(field.handleInput(bounds, LogicalPoint{1.0F, 1.0F}, false, backspace));
        CHECK(field.text() == globe);

        field.setText("new value");
        CHECK_FALSE(field.focused());
    }

} // namespace
