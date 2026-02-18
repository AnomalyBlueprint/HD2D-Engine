#pragma once

#include "Engine/UI/BaseScreen.h"
#include <memory>
#include <string>

namespace Game {

    class MainMenuController : public BaseScreen {
    public:
        MainMenuController(const std::string& guid) 
            : BaseScreen("main_menu", guid) {}
        
        ~MainMenuController() = default;

    protected:
        void RegisterEvents() override;
        void OnInitialize(); 

    private:
        void OnStartGame();
        void OnGenerateWorld();
    };

}
