#include <domain/session/session-name.hpp>

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <utf8proc.h>

namespace lifeGame::domain {

    namespace {

        using CodePoint = std::uint32_t;

        struct DecodedInput {
            std::vector<CodePoint> codePoints;
            std::vector<std::size_t> byteOffsets;
        };

        [[nodiscard]] auto decodeUtf8(std::string_view input, DecodedInput& decoded) -> bool {
            if (input.size() > static_cast<std::size_t>(
                                  std::numeric_limits<utf8proc_ssize_t>::max())) {
                return false;
            }

            decoded.codePoints.clear();
            decoded.byteOffsets.clear();
            decoded.codePoints.reserve(input.size());
            decoded.byteOffsets.reserve(input.size() + 1);

            for (std::size_t offset = 0; offset < input.size();) {
                utf8proc_int32_t codePoint = 0;
                const auto length = utf8proc_iterate(
                    reinterpret_cast<const utf8proc_uint8_t*>(input.data() + offset),
                    static_cast<utf8proc_ssize_t>(input.size() - offset), &codePoint);
                if (length <= 0) {
                    return false;
                }

                decoded.byteOffsets.push_back(offset);
                decoded.codePoints.push_back(static_cast<CodePoint>(codePoint));
                offset += static_cast<std::size_t>(length);
            }
            decoded.byteOffsets.push_back(input.size());
            return true;
        }

        [[nodiscard]] auto isUnicodeWhitespace(CodePoint codePoint) noexcept -> bool {
            switch (codePoint) {
            case 0x0009U:
            case 0x000AU:
            case 0x000BU:
            case 0x000CU:
            case 0x000DU:
            case 0x0020U:
            case 0x0085U:
            case 0x00A0U:
            case 0x1680U:
            case 0x2028U:
            case 0x2029U:
            case 0x202FU:
            case 0x205FU:
            case 0x3000U:
                return true;
            default:
                return codePoint >= 0x2000U && codePoint <= 0x200AU;
            }
        }

        [[nodiscard]] auto mapUtf8(std::string_view input, utf8proc_option_t options,
                                   std::string& output)
            -> std::optional<foundation::ErrorCode> {
            if (input.size() > static_cast<std::size_t>(
                                  std::numeric_limits<utf8proc_ssize_t>::max())) {
                return foundation::ErrorCode::InvalidArgument;
            }

            utf8proc_uint8_t* mapped = nullptr;
            const auto length = utf8proc_map(
                reinterpret_cast<const utf8proc_uint8_t*>(input.data()),
                static_cast<utf8proc_ssize_t>(input.size()), &mapped, options);
            std::unique_ptr<utf8proc_uint8_t, decltype(&std::free)> mappedOwner{mapped,
                                                                                   &std::free};
            if (length < 0) {
                return length == UTF8PROC_ERROR_NOMEM ? foundation::ErrorCode::InternalFailure
                                                       : foundation::ErrorCode::InvalidArgument;
            }

            if (length == 0) {
                output.clear();
            } else {
                output.assign(reinterpret_cast<const char*>(mapped),
                              static_cast<std::size_t>(length));
            }
            return std::nullopt;
        }

    } // namespace

    SessionName::SessionName(std::string displayName, std::string uniquenessKey,
                             std::size_t codePointCount)
        : displayName_{std::move(displayName)},
          uniquenessKey_{std::move(uniquenessKey)},
          codePointCount_{codePointCount} {}

    auto SessionName::create(std::string_view input)
        -> foundation::Result<SessionName, foundation::ErrorCode> {
        try {
            DecodedInput decoded;
            if (!decodeUtf8(input, decoded)) {
                return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }

            std::size_t firstNonWhitespace = 0;
            while (firstNonWhitespace < decoded.codePoints.size() &&
                   isUnicodeWhitespace(decoded.codePoints[firstNonWhitespace])) {
                ++firstNonWhitespace;
            }

            std::size_t lastNonWhitespace = decoded.codePoints.size();
            while (lastNonWhitespace > firstNonWhitespace &&
                   isUnicodeWhitespace(decoded.codePoints[lastNonWhitespace - 1])) {
                --lastNonWhitespace;
            }
            if (firstNonWhitespace == lastNonWhitespace) {
                return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }

            const auto trimmedStart = decoded.byteOffsets[firstNonWhitespace];
            const auto trimmedEnd = decoded.byteOffsets[lastNonWhitespace];
            const auto trimmed = input.substr(trimmedStart, trimmedEnd - trimmedStart);

            std::string normalized;
            const auto normalizationError = mapUtf8(
                trimmed, static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE),
                normalized);
            if (normalizationError) {
                return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                    *normalizationError);
            }

            DecodedInput normalizedDecoded;
            if (!decodeUtf8(normalized, normalizedDecoded) || normalizedDecoded.codePoints.empty() ||
                normalizedDecoded.codePoints.size() > 64) {
                return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }

            std::string uniquenessKey;
            const auto caseFoldError = mapUtf8(
                normalized,
                static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE |
                                               UTF8PROC_CASEFOLD),
                uniquenessKey);
            if (caseFoldError) {
                return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                    *caseFoldError);
            }

            return foundation::Result<SessionName, foundation::ErrorCode>::success(
                SessionName{std::move(normalized), std::move(uniquenessKey),
                            normalizedDecoded.codePoints.size()});
        } catch (const std::bad_alloc&) {
            return foundation::Result<SessionName, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InternalFailure);
        }
    }

    const std::string& SessionName::value() const noexcept { return displayName_; }

    const std::string& SessionName::displayName() const noexcept { return displayName_; }

    const std::string& SessionName::normalized() const noexcept { return displayName_; }

    const std::string& SessionName::uniquenessKey() const noexcept { return uniquenessKey_; }

    std::size_t SessionName::codePointCount() const noexcept { return codePointCount_; }

    bool SessionName::equalsCaseInsensitive(const SessionName& other) const noexcept {
        return uniquenessKey_ == other.uniquenessKey_;
    }

} // namespace lifeGame::domain
