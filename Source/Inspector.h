#pragma once
#include "Actor.h"
#include <memory>
#include "ComponentManager.h"
#include "Input.h"

class Inspector
{
public:
    inline static void DrawActorHeader(Actor* actor)
    {
        ImGui::InputInt("Tag", &actor->tag);

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);

        if (ImGui::Button("...##Actor"))
        {
            ImGui::OpenPopup("ActorMenu");
        }

        if (ImGui::BeginPopup("ActorMenu"))
        {
			if (ImGui::MenuItem("Delete"))
            {
                actor->isDead = true;
            }

            ImGui::EndPopup();
        }
    }

    static bool DrawComponentHeader(Component* comp, Actor* owner)
    {
        ImGui::Separator();

        // 左：名前
        ImGui::Text("%s", ComponentRegistry::IDToString(comp->GetID()));
        ImGui::SameLine();
        ImGui::Checkbox("enabled", &comp->enabled);

        // 右に寄せる
        float lineWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SameLine(lineWidth - 30);

        // ユニークID作成
        std::string popupID = "CompMenu##" + std::to_string((uintptr_t)comp);
        std::string buttonID = "...##" + std::to_string((uintptr_t)comp);

        // ボタン（ID付き）
        if (ImGui::Button(buttonID.c_str()))
        {
            ImGui::OpenPopup(popupID.c_str());
        }

        // メニュー
        if (ImGui::BeginPopup(popupID.c_str()))
        {
            if (ImGui::MenuItem("Delete"))
            {
                ImGui::EndPopup();
                return true; // 削除した
            }

            ImGui::EndPopup();
        }

        return false; // 何もしてない
    }
};
