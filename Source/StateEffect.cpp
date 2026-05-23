#include "StateEffect.h"
#include "Factory.h"
#include "Actor.h"
#include <OpenDialog.h>
#include <EffectManager.h>
#include "EffectRender.h"
//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::StateEffect, StateEffect)

void StateEffect::OnAwake(float elapsedTime)
{
    for (auto& [name, data] : states)
    {
        if (data.isFirstState == true)
        {
            SetState(name);
        }
    }
}

void StateEffect::Update(float elapsedTime)
{
    
}

void StateEffect::DrawInspector()
{
    ImGui::Checkbox("effectloop", &loop);

    ImGui::Text("currentState :", currentState.c_str());

    if (ImGui::Button("Add State"))
    {
        states["NewState"] = StateData{};
    }

    // 名前編集用バッファ（UI用の一時保存）
    static std::unordered_map<std::string, std::array<char, 64>> nameBuffers;

    std::vector<std::string> toErase;
    std::vector<std::pair<std::string, std::string>> toRename;

    for (auto& [name, data] : states)
    {
        ImGui::PushID(name.c_str());
       
        ImGui::Text("State: %s", name.c_str());
        ImGui::SameLine();
        ImGui::Checkbox("IsFirstState", &data.isFirstState);

        //=========================
        // 名前編集
        //=========================
        auto& buf = nameBuffers[name];
        if (buf[0] == '\0')
        {
            std::string src = name;
            size_t len = min(src.size(), buf.size() - 1);
            memcpy(buf.data(), src.data(), len);
            buf[len] = '\0';
        }

        if (ImGui::InputText(("Name##" + name).c_str(), buf.data(), buf.size()))
        {
            // 編集中
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            std::string newName = buf.data();

            if (!newName.empty() && newName != name)
            {
                toRename.emplace_back(name, newName);
            }
        }

        //=========================
        // Effect選択
        //=========================
        if (ImGui::Button(("Select Effect##" + name).c_str()))
        {
            std::string path = OpenDialog::OpenLoadFileDialog();

            if (!path.empty())
            {
                data.effectPath = ToDataPath(path);
                data.effect = EffectManager::Instance().LoadEffect(data.effectPath);
            }
        }

        //=========================
        // 削除ボタン
        //=========================
        ImGui::SameLine();
        if (ImGui::Button(("Delete##" + name).c_str()))
        {
            toErase.push_back(name);
        }

        ImGui::PopID();
    }

    //=========================
    // rename適用
    //=========================
    for (auto& [oldName, newName] : toRename)
    {
        if (states.find(newName) != states.end())
            continue; // 既存名ならスキップ（安全対策）

        states[newName] = std::move(states[oldName]);

        if (currentState == oldName)
            currentState = newName;

        states.erase(oldName);
        nameBuffers.erase(oldName);
    }

    //=========================
    // delete適用
    //=========================
    for (auto& name : toErase)
    {
        states.erase(name);
        nameBuffers.erase(name);
    }
}

void StateEffect::Serialize(nlohmann::json& j) const
{
     for (auto& [name, data] : states)
    {
        j["States"][name] = data.effectPath;
    }
    
    j["CurrentState"] = currentState;
    j["loop"] = loop;
}

void StateEffect::Deserialize(nlohmann::json& j)
{
    states.clear();

    if (j.contains("States"))
    {
        for (auto& [name, path] : j["States"].items())
        {
            StateData data;
            data.effectPath = path;
            data.effect =
                EffectManager::Instance()
                .LoadEffect(path);

            states[name] = data;
        }
    }

    currentState = j.value("CurrentState", "");
    loop = j.value("loop",false);
}

void StateEffect::SetState(const std::string& state)
{
    if (currentState == state && loop) return;

    // 今のエフェクト停止
    if (handle != -1)
    {
        if (states[currentState].effect)
            states[currentState].effect->Stop(handle);

        handle = -1;
    }

    currentState = state;

    // 新しいエフェクト再生
    auto it = states.find(currentState);
    if (it == states.end()) return;

    auto eff = owner->GetComponent<EffectRender>();

    eff->SetEffect(it->second.effect);
    eff->Play();
    eff->SetScale(3);
}

std::unique_ptr<Component> StateEffect::Clone() const
{
    auto s = std::make_unique<StateEffect>();
    s->states = states;
    s->loop = loop;
    return s;
}
