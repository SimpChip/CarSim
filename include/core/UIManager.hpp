#ifndef CARSIM_UIMANAGER_HPP
#define CARSIM_UIMANAGER_HPP

#include "ui/raceHUD.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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

    static void startFrame();

    static void renderOverlay(const float speed, const float boost, const threepp::Vector3 positon, const std::string& mapName) {
        ImGui::Begin("CarSim Debug Overlay");
        ImGui::Text("Map: %s", mapName.c_str());
        ImGui::Text("Speed: %.2f", speed);
        ImGui::Text("Boost factor: %.2f", boost);
        ImGui::Text("x: %.2f", positon.x , "y: %.2f", positon.y , "z: %.2f");
        ImGui::End();
    }

    void renderCheckPointOverlay(const GameState& state, const float dt) {
        raceHUD_.update(dt);
        raceHUD_.render(state.lapManagerData, state.playerCarId);
    }

static void renderStartTimer(const float time, const float dt) {
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
    struct MainMenuResult {
        bool startGame = false;
        bool startMapMaker = false;
        int aiCount = 0;
    };

    static MainMenuResult renderMainMenu(std::string& selectedMap, std::string& selectedModel, const int currentAiCount) {
        MainMenuResult result;
        ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);

        ImGui::SetWindowFontScale(40 / ImGui::GetFontSize());
        ImGui::Text("Welcome to CarSim!");
        ImGui::Separator();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 windowSize = ImGui::GetWindowSize();

        ImGui::SetWindowPos(ImVec2(
            viewport->WorkPos.x + (viewport->WorkSize.x - windowSize.x) * 0.5f,
            viewport->WorkPos.y + (viewport->WorkSize.y - windowSize.y) * 0.5f
        ));

        ImGui::SetNextWindowSize(windowSize);

        constexpr float fontSize = 50.0f;
        ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());
        static int currentMap = 0;
        const char* maps[] = { "firstMap", "secondMap", "thirdMap", "fourthMap" };
        ImGui::Combo("Select Map", &currentMap, maps, IM_ARRAYSIZE(maps));
        selectedMap = maps[currentMap];
        ImGui::Separator();
        static int currentModel = 0;
        const char* models[] = { "redcar", "cooltruck", "wheelcar", "policecary"};
        ImGui::Combo("Select Model", &currentModel, models, IM_ARRAYSIZE(models));
        selectedModel = models[currentModel];

        static int newAiCount = currentAiCount;
        ImGui::SliderInt("Bots", &newAiCount, 0, 120);
        result.aiCount = newAiCount;

        if (ImGui::Button("Start Race")) {
            result.startGame = true;
        }
        if (ImGui::Button("Start Map Maker")) {
            result.startMapMaker = true;
        }

        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
        return result;
    }

    struct MakerMenuResult {
        bool save = false;
        bool undo = false;
        bool clear = false;
        bool goback = false;
        bool looped = false;
        int newMapSize = 100;
    };

    static MakerMenuResult renderMakerMenu(const std::vector<threepp::Vector3> &points, const int currentMapSize, const bool looped) {
        MakerMenuResult result;
        result.looped = looped;
        ImGui::Begin("Map Maker""Race HUD", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowPos(ImVec2(0, 0));
        if (ImGui::Button("Save")) {
             result.save = true;
        }
        ImGui::Spacing();
        if (ImGui::Button("Undo Last")) {
            result.undo = true;
        }
        ImGui::Spacing();
        if (ImGui::Button("Clear All")) {
            result.clear = true;
        }
        ImGui::Spacing();
        if (ImGui::Button("Go Back To Menu")) {
            result.goback = true;
        }
        ImGui::Checkbox("Looped", &result.looped);
        ImGui::Spacing();
        ImGui::Separator();
        static int newSize = currentMapSize;
        ImGui::SliderInt("Map Size", &newSize, 50, 10000);
        ImGui::Text("%.1f",newSize);// range 50–1000
        result.newMapSize = newSize;
        ImGui::Separator();
        ImGui::Spacing();
        for (const auto& point : points) {
            ImGui::Text("X: %.2f , Y: %.2f", point.x, point.y);

        }
        ImGui::End();
        return result;
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