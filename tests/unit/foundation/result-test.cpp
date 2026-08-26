#include <catch2/catch_test_macros.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace {

using lifeGame::foundation::ErrorCode;
using lifeGame::foundation::Result;

TEST_CASE("Result exposes a successful value") {
  const auto result = Result<int, ErrorCode>::success(42);

  REQUIRE(result.hasValue());
  CHECK_FALSE(result.hasError());
  CHECK(result.value() == 42);
}

TEST_CASE("Result exposes a typed error without a message") {
  const auto result = Result<int, ErrorCode>::failure(ErrorCode::PersistenceWriteFailed);

  REQUIRE_FALSE(result.hasValue());
  CHECK(result.hasError());
  CHECK(result.error() == ErrorCode::PersistenceWriteFailed);
}

TEST_CASE("void Result represents successful completion") {
  const auto result = Result<void, ErrorCode>::success();

  REQUIRE(result.hasValue());
  CHECK_FALSE(result.hasError());
}

TEST_CASE("void Result represents a typed failure") {
  const auto result = Result<void, ErrorCode>::failure(ErrorCode::InvalidState);

  REQUIRE_FALSE(result.hasValue());
  CHECK(result.hasError());
  CHECK(result.error() == ErrorCode::InvalidState);
}

} // namespace
