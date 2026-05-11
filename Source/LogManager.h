#pragma once
#include <imgui.h>
#include <string>
#include <vector>

enum class LogCategory
{
	system,
	scene,
	actor,
	component,
	asset,
	input,
};

enum class LogEvent
{
    Initialize,
    Finalize,
    Scene_Transition,
    Create,
    Destroy,
    Add,
    Remove,
    Load,
    Unload,
    None,
};

class LogManager
{
public:
    static LogManager& Instance()
    {
        static LogManager Instance;
        return Instance;
    }

    void Initialize()
    {
        LogHistory.clear();
    }

    void Finalize()
    {
        LogHistory.clear();
    }

    void DrawLogWindow()
    {
        ImGuiIO& io = ImGui::GetIO();

        const float windowPadding = 10.0f;

        ImVec2 windowPos = ImVec2(
            io.DisplaySize.x - windowPadding,
            windowPadding
        );

        ImVec2 windowPivot = ImVec2(1.0f, 0.0f);
        // (1,0) = âEè„äÓèÄ

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, windowPivot);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);

        ImGui::Begin("Log", nullptr);

        for (auto& log : LogHistory)
        {
            ImGui::Text(log.c_str());
        }

        ImGui::End();
    }

    void AddLog(LogCategory category,LogEvent eventType, std::string text)
    {
        LogHistory.push_back("[" + ToString(category) + "]" + "[" + ToString(eventType) + "]"  + text);
    }
private:
    std::vector<std::string> LogHistory;

    std::string ToString(LogCategory name)
    {
        switch (name)
        {
        case LogCategory::system:    return "System";
        case LogCategory::scene:     return "Scene";
        case LogCategory::actor:     return "Actor";
        case LogCategory::component: return "Component";
        case LogCategory::asset:     return "Asset";
        case LogCategory::input:     return "Input";
        default: return "Unknown";
        }
    }

    std::string ToString(LogEvent name)
    {
        switch (name)
        {
        case LogEvent::Initialize:  return "Initialize";
        case LogEvent::Finalize:    return "Finalize";
        case LogEvent::Scene_Transition: return "Scene_Transition";
        case LogEvent::Create:      return "Create";
        case LogEvent::Destroy:     return "Destroy";
        case LogEvent::Add:         return "Add";
        case LogEvent::Load:        return "Load";
        case LogEvent::Unload:      return "Unload";
        case LogEvent::None:        return "Unknown";
        default: return "Unknown";
        }
    };
};