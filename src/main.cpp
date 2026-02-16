#include "Engine/Engine.h"
#include "Game/GameLayer.h"
#include <memory>

/// <summary>
/// Entry point of the Application.
/// </summary>
int main(int argc, char *argv[])
{
    auto engine = std::make_shared<Engine>();
    engine->Init();

    auto gameLayer = std::make_shared<GameLayer>();
    engine->AttachLayer(gameLayer);

    engine->Run();
    engine->Clean();

    return 0;
}