#ifndef CARSIM_UIMANAGER_HPP
#define CARSIM_UIMANAGER_HPP

#include "ui/RaceHUD.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "ResourceManager.hpp"
#include "ui/MainMenuUI.hpp"
#include "ui/PauseMenuUI.hpp"
#include "ui/MapMakerUI.hpp"
#include "types/SnapShots.hpp"
#include "controllers/Commands.hpp"

class UIManager {
public:
    explicit UIManager(void* windowPtr) {
        auto* window = static_cast<GLFWwindow*>(windowPtr);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150");
    }

    static void startFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    static void endFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    static void renderCheckPointOverlay(const GameCommands& cmd, const GameSnapShot& snap) {
            if (snap.lapManagerData.startTimer > 0) {
                renderStartTimer(snap.lapManagerData.startTimer);
            }
            RaceHUD::render(cmd, snap);
    }
    void renderMapMakerOverlay(const GameCommands& cmd, const  MapMakerSnapshot& snap) const {
        mapMakerUI_.render(cmd, snap);
    }
    void renderMainMenuOverlay(const GameCommands& cmd, const std::unique_ptr<ResourceManager>& resources, const int Aicount) const {
        mainMenuUI_.render(cmd, resources, Aicount);
    }
    void renderPauseMenuOverlay(const GameCommands& cmd) const {
        pauseMenuUI_.render(cmd);
    }

    static void renderStartTimer(const float time) {
        ImGui::Begin("Start Timer", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2(
            viewport->WorkPos.x + (viewport->WorkSize.x - windowSize.x) * 0.5f,
            viewport->WorkPos.y + (viewport->WorkSize.y - windowSize.y) * 0.5f
        ));

        const int countdown = static_cast<int>(std::ceil(time));
        constexpr float fontSize = 100.0f;
        ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());

        std::string timerText = (countdown > 0) ? std::to_string(countdown) : "GO!";
        if (countdown <= 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        const ImVec2 textSize = ImGui::CalcTextSize(timerText.c_str());
        const float x_indent = (ImGui::GetWindowWidth() - textSize.x) * 0.5f;
        if (x_indent > 0) ImGui::SameLine(0.0f, x_indent);
        ImGui::Text("%s", timerText.c_str());
        if (countdown <= 0) ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }

    ~UIManager() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    MainMenuUI& mainMenu() { return mainMenuUI_; }
    PauseMenuUI& pauseMenu() { return pauseMenuUI_; }
    MapMakerUI& mapMaker() { return mapMakerUI_; }
private:
    RaceHUD raceHUD_;
    MapMakerUI mapMakerUI_;
    PauseMenuUI pauseMenuUI_;
    MainMenuUI mainMenuUI_;


};


#endif //CARSIM_UIMANAGER_HPP