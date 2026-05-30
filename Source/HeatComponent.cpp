#include "HeatComponent.h"
#include "Actor.h"
#include "Factory.h"
#include "Scene.h"
#include "Collision.h"
#include "StateEffect.h"
#include "ThermalBody.h"
#include <EffectRender.h>
#include "System/Audio.h"
#include "firewood.h"

REGISTER_COMPONENT(ComponentID::HeatTransfer, HeatTransfer)

void HeatTransfer::OnAwake(float elapsedTime)
{

}

void HeatTransfer::Update(float elapsedTime)
{
	auto thermal = owner->GetComponent<ThermalBody>();
	auto scene = owner->GetScene();
	if (!thermal) return;

	m_insideActors.clear();


	for (auto& actor : scene->actors)
	{
		auto toThermal = actor->GetComponent<ThermalBody>();
		if (!toThermal) continue;
		if (toThermal == thermal) continue;
		auto toTransform = actor->GetComponent<Transform>();
		auto transform = owner->GetComponent<Transform>();

		if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
			thermal->GetRadius(),
			toTransform->GetWorldPosition(),
			toThermal->GetRadius()))
		{
			if(thermal->GetHeat() != 0)
			toThermal->SetHeat(thermal->GetHeat());
			m_insideActors.push_back(actor.get());
		} 
	}
	m_canAbsorb = false;
	m_targetHeat = 0;

	Actor* priorityTarget = nullptr;

	for (auto& actor : scene->actors)
	{
		auto Receiver = actor->GetComponent<HeatReceiver>();
		if (!Receiver) continue;
		auto toTransform = actor->GetComponent<Transform>();
		auto transform = owner->GetComponent<Transform>();

		if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(),
			thermal->GetRadius(),
			toTransform->GetWorldPosition(),
			Receiver->GetRadius()))
		{
			auto fw = actor->GetComponent<firewood>();
			// ‰Š‚ª‚ ‚èA‚©‚Â”R‚¦‚Ä‚¢‚é‚È‚çÅ—Dæ‚ÅŠm’èI
			if (fw && fw->GetFire()) {
				m_canAbsorb = true;
				m_targetHeat = Receiver->GetHeatNum();
				break;
			}

			m_canAbsorb = true;
			m_targetHeat = Receiver->GetHeatNum();

			break; // ˆê‚Â‚Å‚à”ÍˆÍ“à‚È‚çUIƒtƒ‰ƒOON‚ÅOK
		}
	}

	
	if (InputC::KeyPressed(VK_LBUTTON))
	{
		bool success = false;
		Actor* bestActor = nullptr;

		// 1. ‚Ü‚¸‰Š‚ª‚ ‚é‚©‚¾‚¯‚ð’T‚·
		for (auto& actor : scene->actors) {
			auto Receiver = actor->GetComponent<HeatReceiver>();
			if (!Receiver) continue;
			auto fw = actor->GetComponent<firewood>();
			if (fw && fw->GetFire() && Collision::IntersectSphereVsSphere(owner->GetComponent<Transform>()->GetWorldPosition(), thermal->GetRadius(), actor->GetComponent<Transform>()->GetWorldPosition(), Receiver->GetRadius())) {
				bestActor = actor.get();
				break; // ‰Š—Dæ
			}
		}

		// 2. ‰Š‚ª‚È‚©‚Á‚½‚çA”ÍˆÍ“à‚Ì‰½‚©‚ð’T‚·
		if (!bestActor) {
			for (auto& actor : scene->actors) {
				auto Receiver = actor->GetComponent<HeatReceiver>();
				if (!Receiver) continue;
				if (Collision::IntersectSphereVsSphere(owner->GetComponent<Transform>()->GetWorldPosition(), thermal->GetRadius(), actor->GetComponent<Transform>()->GetWorldPosition(), Receiver->GetRadius())) {
					bestActor = actor.get();
					break;
				}
			}
		}

		if (bestActor) {
			auto Receiver = bestActor->GetComponent<HeatReceiver>();
			thermal->SetHeat(Receiver->GetHeatNum());
			auto th = bestActor->GetComponent<ThermalBody>();
			if (th && th->GetHeat() == 1) {
				th->SetHeat(0);
				auto a = bestActor->GetComponent<EffectRender>();
				if (a) a->Stop();
			}
			success = true;
		}
			if (!success && thermal->GetHeat() != 0)
		{
			owner->GetComponent<ModelRender>()->PlayAnimation("in_out", false);

			switch (thermal->GetHeat())
			{
			case -2:
			{
				owner->GetChildren().front()->GetComponent<StateEffect>()->SetState("icebreath");
				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_ice.wav");
				Audio::Instance().PlaySE(myPath.c_str());
			}
				break;
			case -1:
			{
				owner->GetChildren().front()->GetComponent<StateEffect>()->SetState("waterbreath");
				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_water.wav");
				Audio::Instance().PlaySE(myPath.c_str());
				break;
			}
			case 1:
			{
				owner->GetChildren().front()->GetComponent<StateEffect>()->SetState("hotbreath");
				std::string myPath = ToDataPath("Data/Sound/SE_game_breath_fire.wav");
				Audio::Instance().PlaySE(myPath.c_str());
			}
			break;

			default:
				break;
			}

			thermal->SetHeat(0);
		}
		
	}


	auto effectstate = owner->GetComponent<StateEffect>();
	if (!effectstate) return;

	if (m_prevHeat != thermal->GetHeat())
	{
		switch (thermal->GetHeat())
		{
		case 1:
		{
			effectstate->SetState("hot");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_fire.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case -1:
		{
			effectstate->SetState("water");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_water.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case -2:
		{
			effectstate->SetState("cold");
			std::string myPath = ToDataPath("Data/Sound/SE_game_reaction_ice.wav");
			Audio::Instance().PlaySE(myPath.c_str());
		}
		break;
		case 0:
		{
			effectstate->SetState("normal");
		}
		break;
		}

		// ÅŒã‚ÉŒ»Ý‚Ìó‘Ô‚ð•Û‘¶‚µ‚ÄAŽŸƒtƒŒ[ƒ€‚Æ”äŠr‚Å‚«‚é‚æ‚¤‚É‚·‚é
		m_prevHeat = thermal->GetHeat();
	}
}

void HeatTransfer::DrawInspector()
{

}

void HeatTransfer::Serialize(nlohmann::json& j) const
{

}

void HeatTransfer::Deserialize(nlohmann::json& j)
{

}

std::unique_ptr<Component> HeatTransfer::Clone() const
{
	return std::make_unique<HeatTransfer>();
}

bool HeatTransfer::IsStatusActive() const
{
	auto thermal = owner->GetComponent<ThermalBody>();
	if (!thermal) return false;

	return (thermal->GetHeat() != 0);
}