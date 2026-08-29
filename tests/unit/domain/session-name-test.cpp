#include <string>

#include <catch2/catch_test_macros.hpp>
#include <domain/session/session-name.hpp>
#include <foundation/error-code.hpp>

namespace {

    using lifeGame::domain::SessionName;
    using lifeGame::foundation::ErrorCode;

    TEST_CASE("Session names trim outer whitespace and normalize NFC") {
        const auto name = SessionName::create("  Cafe\xCC\x81  ");

        REQUIRE(name);
        CHECK(name.value().displayName() == "Caf\xC3\xA9");
        CHECK(name.value().value() == "Caf\xC3\xA9");
        CHECK(name.value().codePointCount() == 4);
        CHECK(name.value().uniquenessKey() == "caf\xC3\xA9");
    }

    TEST_CASE("Session name uniqueness preserves case and internal whitespace") {
        const auto first = SessionName::create("  My  Session  ");
        const auto second = SessionName::create("my  session");

        REQUIRE(first);
        REQUIRE(second);
        CHECK(first.value().displayName() == "My  Session");
        CHECK(first.value().uniquenessKey() == second.value().uniquenessKey());
        CHECK(first.value().equalsCaseInsensitive(second.value()));
        CHECK(first.value().displayName() != second.value().displayName());
    }

    TEST_CASE("Session names use complete Unicode NFC normalization and case folding") {
        const auto decomposed = SessionName::create("\xCE\xB1\xCC\x81");
        const auto composed = SessionName::create("\xCE\xAC");
        const auto sharpS = SessionName::create("Stra\xC3\x9F" "e");
        const auto uppercase = SessionName::create("STRASSE");

        REQUIRE(decomposed);
        REQUIRE(composed);
        REQUIRE(sharpS);
        REQUIRE(uppercase);
        CHECK(decomposed.value().displayName() == composed.value().displayName());
        CHECK(decomposed.value().equalsCaseInsensitive(composed.value()));
        CHECK(sharpS.value().equalsCaseInsensitive(uppercase.value()));
    }

    TEST_CASE("Session names count Unicode code points rather than bytes") {
        std::string sixtyFourEmoji;
        for (int index = 0; index < 64; ++index) {
            sixtyFourEmoji += "\xF0\x9F\x8C\x8D";
        }
        const auto valid = SessionName::create(sixtyFourEmoji);
        const auto invalid = SessionName::create(sixtyFourEmoji + "a");

        REQUIRE(valid);
        CHECK(valid.value().codePointCount() == 64);
        CHECK_FALSE(invalid);
        CHECK(invalid.error() == ErrorCode::InvalidArgument);
    }

    TEST_CASE("Session names reject blank, malformed, and overlong values") {
        CHECK_FALSE(SessionName::create(" \t\n "));
        CHECK_FALSE(SessionName::create(std::string{"bad\xC3\x28"}));
        CHECK_FALSE(SessionName::create(std::string(65, 'a')));
    }

} // namespace
