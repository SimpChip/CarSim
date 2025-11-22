//
// Created by Mella on 13.11.2025.
//

#ifndef CARSIM_UIMANAGER_HPP
#define CARSIM_UIMANAGER_HPP

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include "RaceHUD.hpp"

class UIManager {
public:
    RaceHUD raceHUD_;

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

    static void renderOverlay(const float speed, const float boost, const threepp::Vector3 positon, const std::string& mapName) {
        ImGui::Begin("CarSim Debug Overlay");
        ImGui::Text("Map: %s", mapName.c_str());
        ImGui::Text("Speed: %.2f", speed);
        ImGui::Text("Boost factor: %.2f", boost);
        ImGui::Text("x: %.2f", positon.x , "y: %.2f", positon.y , "z: %.2f");
        ImGui::End();
    }

    void renderCheckPointOverlay(GameLogic& gameLogic, const float dt) {
        raceHUD_.update(dt);
        raceHUD_.render(gameLogic);
    }

static void renderStartTimer(float time, const float dt) {
    ImGui::Begin("Start Timer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 windowSize = ImGui::GetWindowSize();

    ImGui::SetWindowPos(ImVec2(
        viewport->WorkPos.x + (viewport->WorkSize.x - windowSize.x) * 0.5f,
        viewport->WorkPos.y + (viewport->WorkSize.y - windowSize.y) * 0.5f
    ));
    const int countdown = static_cast<int>(std::ceil(time));
    constexpr float fontSize = 100.0f;
    ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());

    std::string timerText;
    if (countdown > 0) {
        timerText = std::to_string(countdown);
    } else {
        timerText = "GO!";
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green text
    }
    const ImVec2 textSize = ImGui::CalcTextSize(timerText.c_str());
    const float x_indent = (ImGui::GetWindowWidth() - textSize.x) * 0.5f;
    if (x_indent > 0) {
        ImGui::SameLine(0.0f, x_indent);
    }

    ImGui::Text("%s", timerText.c_str());

    if (countdown <= 0) {
        ImGui::PopStyleColor();
    }
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
}

    static bool renderMainMenu(std::string& selectedMap, std::string& selectedModel) {
        bool startClicked = false;
        ImGui::Begin("Main Menu");

        ImGui::Text("Welcome to CarSim!");
        ImGui::Separator();

        static int currentMap = 0;
        const char* maps[] = { "firstMap", "secondMap", "thirdMap", "fourthMap" };
        ImGui::Combo("Select Map", &currentMap, maps, IM_ARRAYSIZE(maps));
        selectedMap = maps[currentMap];
        ImGui::Separator();

        static int currentModel = 0;
        const char* models[] = { "cooltruck", "redcar", "wheelcar", "policecary"};
        ImGui::Combo("Select Model", &currentModel, models, IM_ARRAYSIZE(models));
        selectedModel = models[currentModel];

        if (ImGui::Button("Start Race")) {
            startClicked = true;
        }

        ImGui::End();
        return startClicked;
    }

    struct PauseResult {
        bool resume = false;
        bool quitToMenu = false;
    };

    static PauseResult renderPauseMenu() {
        PauseResult result;
        ImGui::Begin("Pause Menu");

        ImGui::Text("Game Paused");
        ImGui::Separator();

        if (ImGui::Button("Resume")) {
            result.resume = true;
        }
        if (ImGui::Button("Quit to Menu")) {
            result.quitToMenu = true;
        }

        ImGui::End();
        return result;
    }

    static void endFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    ~UIManager() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

#endif //CARSIM_UIMANAGER_HPP