#pragma once

#include <cstdint>

namespace lifeGame::foundation {

enum class ErrorCode : std::uint8_t {
  InvalidArgument,
  InvalidFieldDimensions = InvalidArgument,
  InvalidSessionName = InvalidArgument,
  InvalidGenerationInterval = InvalidArgument,
  InvalidState,
  NotFound,
  SessionNotFound = NotFound,
  PersistenceOpenFailed,
  PersistenceReadFailed,
  PersistenceWriteFailed,
  InternalFailure,
  AlreadyExists,
  DuplicateName = AlreadyExists,
  DuplicateSessionName = AlreadyExists,
  NameAlreadyExists = AlreadyExists,
  CapacityExceeded,
  SessionLimitReached = CapacityExceeded,
  TooManySessions = CapacityExceeded,
};

} // namespace lifeGame::foundation
