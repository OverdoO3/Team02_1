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

    //// 追加：ゲーム開始直後（例えば1秒以内）は判定をスキップする
    //static float timeElapsed = 0.0f;
    //timeElapsed += elapsedTime;
    //if (timeElapsed < 1.0f) return;
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

    bool isNowInside = false;
    if (m_playerThermal)
    {
        auto myPos = owner->GetComponent<Transform>()->GetWorldPosition();
        auto playerPos = m_playerThermal->GetOwner()->GetComponent<Transform>()->GetWorldPosition();
        if (Collision::IntersectSphereVsSphere(myPos, this->radius, playerPos, m_playerThermal->GetRadius()))
        {
            isNowInside = true;
        }
    }

    // 範囲内の時
    if (isNowInside && !m_wasInside)
    {
        Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_game_object_aim.wav").c_str());
    }
    m_wasInside = isNowInside;
    m_isPlayerNear = isNowInside;   
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