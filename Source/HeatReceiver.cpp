#include "HeatReceiver.h"
#include "Factory.h"
#include "Scene.h"
#include "Actor.h"
#include "ThermalBody.h"
#include "Collision.h"
#include "System/Audio.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::HeatReceiver, HeatReceiver)

void HeatReceiver::OnAwake(float elapsedTime)
{
}

void HeatReceiver::Update(float elapsedTime)
{
    // 1. プレイヤーをまだ持っていないなら検索する
    if (!m_playerThermal)
    {
        auto scene = owner->GetScene();
        if (scene)
        {
            for (auto& actor : scene->actors)
            {
                if (actor->tag == 1) // プレイヤーのタグ
                {
                    m_playerThermal = actor->GetComponent<ThermalBody>();
                    break;
                }
            }
        }
    }

    // 2. プレイヤーが見つかっているなら距離判定
    m_isPlayerNear = false;
    if (m_playerThermal)
    {
        auto myPos = owner->GetComponent<Transform>()->GetWorldPosition();
        auto playerPos = m_playerThermal->GetOwner()->GetComponent<Transform>()->GetWorldPosition();
        if (Collision::IntersectSphereVsSphere(myPos, this->radius, playerPos, m_playerThermal->GetRadius()))
        {
            m_isPlayerNear = true;
        }
        else
        {
            m_isPlayerNear = false; // これが重要！範囲外の時はちゃんと false に戻す
        }
    }
}

void HeatReceiver::DrawInspector()
{
    ImGui::InputInt("heat", &heatNum);
    ImGui::SliderInt("Linked Light Index", &m_linkedLightIndex, -1, 7); // 追加
}

void HeatReceiver::Serialize(nlohmann::json& j) const
{
    j["heat"] = heatNum;
    j["linkedLightIndex"] = m_linkedLightIndex; 
}

void HeatReceiver::Deserialize(nlohmann::json& j)
{
	auto n = j["temperature"];
    heatNum = j.value("heat", 0);
    m_linkedLightIndex = j.value("linkedLightIndex", -1);
}

std::unique_ptr<Component> HeatReceiver::Clone() const
{
	auto h = std::make_unique<HeatReceiver>();
	h->heatNum = this->heatNum;
    h->m_linkedLightIndex = this->m_linkedLightIndex;
	return h;
}

void HeatReceiver::SetHeatNum(int n)
{
    int oldHeat = heatNum;
    heatNum = n;

    if (oldHeat != heatNum)
    {
        auto* renderer = Graphics::Instance().GetModelRenderer();
        if (renderer && m_linkedLightIndex >= 0)
        {
            renderer->SetLightEnabled(m_linkedLightIndex, (heatNum > 0));
        }

        //std::string soundPath;
        //if (heatNum == 1)      soundPath =  ToDataPath("Data/Sound/SE_game_reaction_fire.wav");  // 炎の時の音
        //else if (heatNum == -1) soundPath = ToDataPath("Data/Sound/SE_game_reaction_water.wav"); // 水（通常）の音
        //else if (heatNum == -2) soundPath = ToDataPath("Data/Sound/SE_game_reaction_ice.wav");   // 氷の時の音

        //if (!soundPath.empty())
        //{
        //    Audio::Instance().PlaySE(soundPath.c_str());
        //}
    }
}