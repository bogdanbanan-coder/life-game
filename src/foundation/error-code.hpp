#pragma once

#include <cstdint>

namespace lifeGame::foundation {

enum class ErrorCode : std::uint8_t {
    InvalidArgument,
    InvalidState,
    NotFound,
    PersistenceOpenFailed,
    PersistenceReadFailed,
    PersistenceWriteFailed,
    InternalFailure,
};

} // namespace lifeGame::foundation
