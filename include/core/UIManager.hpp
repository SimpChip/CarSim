#ifndef CARSIM_UIMANAGER_HPP
#define CARSIM_UIMANAGER_HPP

#include "ui/RaceHUD.hpp"
#include "ui/MainMenuUI.hpp"
#include "ui/PauseMenuUI.hpp"
#include "ui/MapMakerUI.hpp"
#include "types/gameSnapshots.hpp"
#include "types/mapMakerSnapshots.hpp"
#include "helpers/ResourceManager.hpp"
#include "controllers/Commands.hpp"


// TODO UI very similar interfaces, can i combine two one common class?
class UIManager {
public:
    explicit UIManager(void* windowPtr);

    void startFrame();
    void endFrame();

    void renderCheckPointOverlay(const GameCommands& cmd, const GameSnap& snap);

    void renderMapMakerOverlay(const GameCommands& cmd, const  MapMakerSnap& snap) const;
    void renderMainMenuOverlay(const GameCommands& cmd, const std::unique_ptr<ResourceManager>& resources, int Aicount);
    void renderPauseMenuOverlay(const GameCommands& cmd) const;

    void renderStartTimer(float time);

    ~UIManager();
private:
    ImGuiContext* imguiContext_;
    RaceHUD raceHUD_;
    MapMakerUI mapMakerUI_;
    PauseMenuUI pauseMenuUI_;
    MainMenuUI mainMenuUI_;
};


#endif //CARSIM_UIMANAGER_HPP