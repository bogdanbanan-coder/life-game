#pragma once

#include <string_view>

#include <foundation/error-code.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/ui/text-field.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    enum class ErrorDialogAction {
        None,
        Retry,
        Cancel,
    };

    struct ErrorDialogLayout {
        Rectangle panel;
        Rectangle message;
        Rectangle retry;
        Rectangle cancel;
    };

    class ErrorDialog {
      public:
        [[nodiscard]] static auto calculateLayout(int viewportWidth, int viewportHeight) noexcept
            -> ErrorDialogLayout;
        [[nodiscard]] static auto messageFor(foundation::ErrorCode error) noexcept
            -> std::string_view;

        void open(foundation::ErrorCode error) noexcept;
        void close() noexcept;
        [[nodiscard]] bool isOpen() const noexcept;
        [[nodiscard]] foundation::ErrorCode error() const noexcept;

        [[nodiscard]] ErrorDialogAction handleInput(int viewportWidth, int viewportHeight,
                                                    PointerSample pointer) noexcept;
        [[nodiscard]] ErrorDialogAction consumeAction() noexcept;

        void render(int viewportWidth, int viewportHeight) const;

      private:
        foundation::ErrorCode error_ = foundation::ErrorCode::InternalFailure;
        bool open_ = false;
        ErrorDialogAction pendingAction_ = ErrorDialogAction::None;
    };

} // namespace lifeGame::presentation
