#include <presentation/ui/text-field.hpp>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include <raygui.h>

namespace lifeGame::presentation {

    namespace {

        [[nodiscard]] auto contains(Rectangle bounds, LogicalPoint point) noexcept -> bool {
            return point.x >= bounds.x && point.x < bounds.x + bounds.width &&
                   point.y >= bounds.y && point.y < bounds.y + bounds.height;
        }

        [[nodiscard]] auto isContinuationByte(unsigned char byte) noexcept -> bool {
            return (byte & 0xC0U) == 0x80U;
        }

        [[nodiscard]] auto expectedCodePointBytes(unsigned char byte) noexcept -> std::size_t {
            if ((byte & 0x80U) == 0U) {
                return 1;
            }
            if ((byte & 0xE0U) == 0xC0U) {
                return 2;
            }
            if ((byte & 0xF0U) == 0xE0U) {
                return 3;
            }
            if ((byte & 0xF8U) == 0xF0U) {
                return 4;
            }
            return 1;
        }

        [[nodiscard]] auto completeUtf8PrefixSize(std::string_view text,
                                                   std::size_t maximumBytes) noexcept
            -> std::size_t {
            auto end = std::min(text.size(), maximumBytes);
            while (end > 0) {
                auto start = end - 1;
                while (start > 0 &&
                       isContinuationByte(static_cast<unsigned char>(text[start]))) {
                    --start;
                }
                if (expectedCodePointBytes(static_cast<unsigned char>(text[start])) <=
                    end - start) {
                    return end;
                }
                end = start;
            }
            return 0;
        }

        void removeLastCodePoint(std::string& text) {
            if (text.empty()) {
                return;
            }

            auto index = text.size() - 1;
            while (index > 0 &&
                   isContinuationByte(static_cast<unsigned char>(text[index]))) {
                --index;
            }
            text.erase(index);
        }

    } // namespace

    TextField::TextField(std::size_t maximumBytes) : maximumBytes_{maximumBytes} {}

    void TextField::setText(std::string text) {
        const auto size = completeUtf8PrefixSize(text, maximumBytes_);
        text_.assign(text.data(), size);
        validationMessage_.clear();
        focused_ = false;
        suppressGuiEditing_ = false;
    }

    const std::string& TextField::text() const noexcept { return text_; }

    void TextField::setValidationMessage(std::string message) {
        validationMessage_ = std::move(message);
    }

    void TextField::clearValidationMessage() { validationMessage_.clear(); }

    const std::string& TextField::validationMessage() const noexcept {
        return validationMessage_;
    }

    bool TextField::isValid() const noexcept { return validationMessage_.empty(); }

    bool TextField::focused() const noexcept { return focused_; }

    bool TextField::handleInput(Rectangle bounds, LogicalPoint pointer, bool pressed,
                                const TextInput& input) {
        suppressGuiEditing_ = false;
        if (pressed) {
            focused_ = contains(bounds, pointer);
        }

        if (!focused_) {
            return false;
        }

        auto changed = false;
        if (input.backspace && !text_.empty()) {
            removeLastCodePoint(text_);
            changed = true;
        }

        if (!input.characters.empty() && text_.size() < maximumBytes_) {
            const auto availableBytes = maximumBytes_ - text_.size();
            const auto appendedBytes = completeUtf8PrefixSize(input.characters, availableBytes);
            if (appendedBytes > 0) {
                text_.append(input.characters, 0, appendedBytes);
                changed = true;
            }
        }

        if (changed) {
            validationMessage_.clear();
        }
        suppressGuiEditing_ = changed;
        return changed;
    }

    void TextField::render(Rectangle bounds) const {
        std::vector<char> buffer(maximumBytes_ + 1, '\0');
        std::copy(text_.begin(), text_.end(), buffer.begin());
        const auto textSize = static_cast<int>(buffer.size());
        const auto editMode = focused_ && !suppressGuiEditing_;
        static_cast<void>(GuiTextBox(bounds, buffer.data(), textSize, editMode));
        const auto size = completeUtf8PrefixSize(buffer.data(), maximumBytes_);
        text_.assign(buffer.data(), size);
        if (suppressGuiEditing_) {
            suppressGuiEditing_ = false;
        }
    }

} // namespace lifeGame::presentation
