#include "Coin.h"
#include "Factory.h"
#include "Actor.h"
#include "Collision.h"
#include "Scene.h"
#include "SceneManager.h"
//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::Coin, Coin)

void Coin::OnAwake(float elapsedTime)
{
	// (仮)ステージ読み込み時、すでに獲得済みなら3Dモデルを最初から非表示にする
	auto* scene = owner->GetScene();
	if (scene && scene->sceneManager)
	{
		if (scene->sceneManager->IsCoinCollected(m_stageIndex, m_coinIndex))
		{
			if (auto* mr = owner->GetComponent<ModelRender>())
			{
				mr->enabled = false;
			}
		}
	}
}

void Coin::Update(float elapsedTime)
{
	auto playerModelChanger = owner->GetScene()->FindByTag(1);
	auto transform = owner->GetComponent<Transform>();

	// コインをくるくる回す処理
	auto ro = transform->GetEulerRotation();
	ro.y += elapsedTime * 4;
	transform->SetRotationEuler(ro.x, ro.y, ro.z);

	if (m_isCollected) return;

	// もしくは、モデルが非表示（mr が nullptr か、enabled が false）なら判定しない
	auto* mr = owner->GetComponent<ModelRender>();

	// プレイヤーとの衝突判定
	if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(), 1, playerModelChanger->GetComponent<Transform>()->GetWorldPosition(), 10))
	{
		auto* scene = owner->GetScene();
		if (scene && scene->sceneManager)
		{
			scene->sceneManager->CollectCoin(m_stageIndex, m_coinIndex);
		}
		// 触れたら非表示にする
		if (mr)
		{
			mr->enabled = false;
		}

		m_isCollected = true;

	}
}


void Coin::DrawInspector()
{
	//コイン用
	ImGui::InputInt("Stage Index(0 - 3)", &m_stageIndex);
	ImGui::InputInt("Coin Index (0 - 2)", &m_coinIndex);
}

void Coin::Serialize(nlohmann::json& j) const
{
	j["StageIndex"] = m_stageIndex;
	j["CoinIndex"] = m_coinIndex;
}

void Coin::Deserialize(nlohmann::json& j)
{
	if (j.contains("StageIndex") && j["StageIndex"].is_number_integer())
	{
		m_stageIndex = j["StageIndex"].get<int>();
	}
	else
	{
		m_stageIndex = 0;
	}

	if (j.contains("CoinIndex") && j["CoinIndex"].is_number_integer())
	{
		m_coinIndex = j["CoinIndex"].get<int>();
	}
	else
	{
		m_coinIndex = 0;
	}
}

std::unique_ptr<Component> Coin::Clone() const
{
	auto c = std::make_unique<Coin>();
	c->m_stageIndex = this->m_stageIndex;
	c->m_coinIndex = this->m_coinIndex;
	return c;
}
