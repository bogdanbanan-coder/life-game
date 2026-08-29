#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

#include <application/session/session-service.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/ui/error-dialog.hpp>
#include <presentation/ui/name-dialog.hpp>
#include <presentation/ui/settings-panel.hpp>
#include <presentation/ui/status-message.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct StartScreenLayout {
        Rectangle title;
        Rectangle create;
        Rectangle settings;
        Rectangle sessionList;
        Rectangle status;
    };

    enum class StartScreenActionKind {
        None,
        OpenSession,
    };

    struct StartScreenAction {
        StartScreenActionKind kind = StartScreenActionKind::None;
        std::optional<domain::SessionId> sessionId;
    };

    struct StartScreenRenderPlan {
        std::string_view title;
        std::string_view createLabel;
        std::string_view settingsLabel;
        bool modalOpen;
        std::size_t sessionCount;
    };

    class StartScreen {
      public:
        StartScreen(application::SettingsService& settingsService,
                    application::SessionService& sessionService);

        [[nodiscard]] static auto calculateLayout(int viewportWidth, int viewportHeight) noexcept
            -> StartScreenLayout;
        [[nodiscard]] static auto sessionCardBounds(const StartScreenLayout& layout,
                                                    std::size_t index) noexcept -> Rectangle;
        [[nodiscard]] StartScreenRenderPlan calculateRenderPlan() const noexcept;

        [[nodiscard]] StartScreenAction processInput(int viewportWidth, int viewportHeight,
                                                      PointerSample pointer,
                                                      const TextInput& textInput = {});
        void render(int viewportWidth, int viewportHeight) const;

        [[nodiscard]] bool modalOwnsInput() const noexcept;
        [[nodiscard]] bool settingsOpen() const noexcept;
        [[nodiscard]] bool createDialogOpen() const noexcept;
        [[nodiscard]] const SettingsPanel& settingsPanel() const noexcept;
        [[nodiscard]] SettingsPanel& settingsPanel() noexcept;
        [[nodiscard]] const NameDialog& nameDialog() const noexcept;
        [[nodiscard]] NameDialog& nameDialog() noexcept;
        [[nodiscard]] const ErrorDialog& errorDialog() const noexcept;
        [[nodiscard]] const StatusMessage& statusMessage() const noexcept;

      private:
        [[nodiscard]] StartScreenAction openSessionAt(LogicalPoint point,
                                                       const StartScreenLayout& layout);
        [[nodiscard]] StartScreenAction handleErrorInput(int viewportWidth, int viewportHeight,
                                                          PointerSample pointer);

        application::SessionService& sessionService_;
        SettingsPanel settingsPanel_;
        NameDialog nameDialog_;
        mutable ErrorDialog errorDialog_;
        mutable StatusMessage status_;
    };

} // namespace lifeGame::presentation
