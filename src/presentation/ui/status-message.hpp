#pragma once

#include <string>
#include <string_view>

#include <raylib.h>

namespace lifeGame::presentation {

    enum class StatusMessageKind {
        None,
        Information,
        Success,
        Warning,
        Error,
    };

    class StatusMessage {
      public:
        void show(StatusMessageKind kind, std::string message);
        void showInformation(std::string message);
        void showSuccess(std::string message);
        void showWarning(std::string message);
        void showError(std::string message);
        void clear() noexcept;

        [[nodiscard]] bool visible() const noexcept;
        [[nodiscard]] StatusMessageKind kind() const noexcept;
        [[nodiscard]] const std::string& text() const noexcept;

        void render(Rectangle bounds) const;

      private:
        StatusMessageKind kind_ = StatusMessageKind::None;
        std::string text_;
    };

} // namespace lifeGame::presentation
