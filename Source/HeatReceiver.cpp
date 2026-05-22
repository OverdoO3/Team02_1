#include "HeatReceiver.h"
#include "Factory.h"
#include "Scene.h"
#include "Actor.h"
#include "ThermalBody.h"
#include "Collision.h"

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
}

void HeatReceiver::Serialize(nlohmann::json& j) const
{
	j["temperature"] = heatNum;
}

void HeatReceiver::Deserialize(nlohmann::json& j)
{
	auto n = j["temperature"];
	heatNum = n;
}

std::unique_ptr<Component> HeatReceiver::Clone() const
{
	auto h = std::make_unique<HeatReceiver>();
	h->heatNum = this->heatNum;
	return h;
}