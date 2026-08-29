#include <presentation/screens/start-screen.hpp>

#include <algorithm>
#include <cstddef>
#include <string>

#include <raygui.h>

namespace lifeGame::presentation {

    namespace {

        constexpr int CONTENT_MARGIN = 32;
        constexpr int BUTTON_WIDTH = 160;
        constexpr int BUTTON_HEIGHT = 40;
        constexpr int CARD_WIDTH = 184;
        constexpr int CARD_HEIGHT = 144;
        constexpr int CARD_GAP = 16;

        [[nodiscard]] auto contains(Rectangle bounds, LogicalPoint point) noexcept -> bool {
            return point.x >= bounds.x && point.x < bounds.x + bounds.width &&
                   point.y >= bounds.y && point.y < bounds.y + bounds.height;
        }

    } // namespace

    StartScreen::StartScreen(application::SettingsService& settingsService,
                             application::SessionService& sessionService)
        : sessionService_{sessionService},
          settingsPanel_{settingsService},
          nameDialog_{sessionService} {}

    auto StartScreen::calculateLayout(int viewportWidth, int viewportHeight) noexcept
        -> StartScreenLayout {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto contentWidth = std::max(safeWidth - 2 * CONTENT_MARGIN, 1);
        const auto top = std::max(safeHeight / 8, CONTENT_MARGIN);
        return StartScreenLayout{
            Rectangle{static_cast<float>(CONTENT_MARGIN), static_cast<float>(top),
                      static_cast<float>(contentWidth), 48.0F},
            Rectangle{static_cast<float>(CONTENT_MARGIN), static_cast<float>(top + 72),
                      static_cast<float>(BUTTON_WIDTH), static_cast<float>(BUTTON_HEIGHT)},
            Rectangle{static_cast<float>(CONTENT_MARGIN + BUTTON_WIDTH + 16),
                      static_cast<float>(top + 72), static_cast<float>(BUTTON_WIDTH),
                      static_cast<float>(BUTTON_HEIGHT)},
            Rectangle{static_cast<float>(CONTENT_MARGIN), static_cast<float>(top + 144),
                      static_cast<float>(contentWidth),
                      static_cast<float>(std::max(safeHeight - top - 176, CARD_HEIGHT))},
            Rectangle{static_cast<float>(CONTENT_MARGIN),
                      static_cast<float>(safeHeight - CONTENT_MARGIN - 28),
                      static_cast<float>(contentWidth), 24.0F}};
    }

    auto StartScreen::sessionCardBounds(const StartScreenLayout& layout, std::size_t index) noexcept
        -> Rectangle {
        const auto x = layout.sessionList.x +
                       static_cast<float>(index * static_cast<std::size_t>(CARD_WIDTH + CARD_GAP));
        const auto y = layout.sessionList.y;
        return Rectangle{x, y, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT)};
    }

    StartScreenRenderPlan StartScreen::calculateRenderPlan() const noexcept {
        return StartScreenRenderPlan{"Life Game", "Create session", "Settings", modalOwnsInput(),
                                     sessionService_.sessions().size()};
    }

    StartScreenAction StartScreen::processInput(int viewportWidth, int viewportHeight,
                                                PointerSample pointer,
                                                const TextInput& textInput) {
        if (errorDialog_.isOpen()) {
            return handleErrorInput(viewportWidth, viewportHeight, pointer);
        }

        if (settingsPanel_.isOpen()) {
            const auto action =
                settingsPanel_.handleInput(viewportWidth, viewportHeight, pointer, textInput);
            if (action == SettingsPanelAction::Saved) {
                status_.showSuccess("Settings saved.");
            } else if (action == SettingsPanelAction::SaveFailed &&
                       settingsPanel_.lastError()) {
                errorDialog_.open(*settingsPanel_.lastError());
            }
            if (textInput.escape) {
                settingsPanel_.cancel();
            }
            return {};
        }

        if (nameDialog_.isOpen()) {
            const auto action =
                nameDialog_.handleInput(viewportWidth, viewportHeight, pointer, textInput);
            if (action == NameDialogAction::Created) {
                const auto sessionId = nameDialog_.takeCreatedSession();
                if (sessionId) {
                    return StartScreenAction{StartScreenActionKind::OpenSession, sessionId};
                }
            }
            return {};
        }

        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        if (!pointer.pressed || !pointer.down) {
            return {};
        }

        if (contains(layout.settings, pointer.position)) {
            settingsPanel_.open();
            return {};
        }
        if (contains(layout.create, pointer.position)) {
            nameDialog_.open();
            return {};
        }
        return openSessionAt(pointer.position, layout);
    }

    void StartScreen::render(int viewportWidth, int viewportHeight) const {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto layout = calculateLayout(safeWidth, safeHeight);
        const auto renderPlan = calculateRenderPlan();

        ClearBackground(Color{32, 32, 32, 255});
        DrawText(renderPlan.title.data(), static_cast<int>(layout.title.x),
                 static_cast<int>(layout.title.y), 32, WHITE);
        GuiButton(layout.create, renderPlan.createLabel.data());
        GuiButton(layout.settings, renderPlan.settingsLabel.data());
        GuiLabel(Rectangle{layout.sessionList.x, layout.sessionList.y - 28.0F,
                           layout.sessionList.width, 24.0F},
                 "Sessions");

        const auto summaries = sessionService_.summaries();
        for (std::size_t index = 0; index < summaries.size(); ++index) {
            const auto card = sessionCardBounds(layout, index);
            GuiPanel(card, nullptr);
            GuiButton(Rectangle{card.x + 8.0F, card.y + 8.0F, card.width - 16.0F, 36.0F},
                      summaries[index].displayName.c_str());
            const auto dimensions = std::to_string(summaries[index].dimensions.width) + " x " +
                                    std::to_string(summaries[index].dimensions.height);
            GuiLabel(Rectangle{card.x + 8.0F, card.y + 56.0F, card.width - 16.0F, 24.0F},
                     dimensions.c_str());
            GuiLabel(Rectangle{card.x + 8.0F, card.y + 88.0F, card.width - 16.0F, 24.0F},
                     "Open session");
        }
        status_.render(layout.status);

        if (settingsPanel_.isOpen() || nameDialog_.isOpen() || errorDialog_.isOpen()) {
            DrawRectangle(0, 0, safeWidth, safeHeight, Color{0, 0, 0, 96});
        }
        settingsPanel_.render(safeWidth, safeHeight);
        nameDialog_.render(safeWidth, safeHeight);
        errorDialog_.render(safeWidth, safeHeight);
    }

    bool StartScreen::modalOwnsInput() const noexcept {
        return settingsPanel_.isOpen() || nameDialog_.isOpen() || errorDialog_.isOpen();
    }

    bool StartScreen::settingsOpen() const noexcept { return settingsPanel_.isOpen(); }

    bool StartScreen::createDialogOpen() const noexcept { return nameDialog_.isOpen(); }

    const SettingsPanel& StartScreen::settingsPanel() const noexcept { return settingsPanel_; }

    SettingsPanel& StartScreen::settingsPanel() noexcept { return settingsPanel_; }

    const NameDialog& StartScreen::nameDialog() const noexcept { return nameDialog_; }

    NameDialog& StartScreen::nameDialog() noexcept { return nameDialog_; }

    const ErrorDialog& StartScreen::errorDialog() const noexcept { return errorDialog_; }

    const StatusMessage& StartScreen::statusMessage() const noexcept { return status_; }

    StartScreenAction StartScreen::openSessionAt(LogicalPoint point,
                                                 const StartScreenLayout& layout) {
        const auto summaries = sessionService_.summaries();
        for (std::size_t index = 0; index < summaries.size(); ++index) {
            if (contains(sessionCardBounds(layout, index), point)) {
                return StartScreenAction{StartScreenActionKind::OpenSession, summaries[index].id};
            }
        }
        return {};
    }

    StartScreenAction StartScreen::handleErrorInput(int viewportWidth, int viewportHeight,
                                                    PointerSample pointer) {
        const auto action = errorDialog_.handleInput(viewportWidth, viewportHeight, pointer);
        if (action == ErrorDialogAction::Retry) {
            const auto result = settingsPanel_.save();
            if (result) {
                errorDialog_.close();
            } else if (settingsPanel_.lastError()) {
                errorDialog_.open(*settingsPanel_.lastError());
            }
        } else if (action == ErrorDialogAction::Cancel) {
            errorDialog_.close();
            settingsPanel_.cancel();
        }
        return {};
    }

} // namespace lifeGame::presentation
