#include "Game/UI/MainMenuController.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/LoggerService.h"
#include "Engine/Services/UI/IUIService.h"
#include "Engine/Services/UI/UIService.h" 

namespace Game {

    void MainMenuController::RegisterEvents() {
        auto btnStart = FindElement("btn_start");
        auto btnGen = FindElement("btn_gen_world");

        if (btnStart) {
            ServiceLocator::Get().GetService<ILoggerService>()->Log("[MainMenu] Registered event for Start Game");
        }
    }

    void MainMenuController::OnStartGame() {
        ServiceLocator::Get().GetService<ILoggerService>()->Log("[MainMenu] START GAME Clicked");
    }

    void MainMenuController::OnGenerateWorld() {
        ServiceLocator::Get().GetService<ILoggerService>()->Log("[MainMenu] GENERATE WORLD Clicked");

        PopupRequest req;
        req.id = "confirm_gen";
        req.priority = PopupPriority::Warning;
        req.title = "Overwrite?";
        req.message = "This will overwrite your world.";
        req.onConfirm = [](){ 
             ServiceLocator::Get().GetService<ILoggerService>()->Log("World Generation Confirmed");
        };

        auto ui = std::dynamic_pointer_cast<IUIService>(ServiceLocator::Get().GetService<IUIService>());
        if(ui) ui->ShowPopup(req);
    }

}
