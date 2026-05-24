#include "snowman.h"
#include "Factory.h"
#include "Actor.h"
#include <EffectRender.h>
#include "System/Audio.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::snowman, snowman)

void snowman::OnAwake(float elapsedTime)
{
}

void snowman::Update(float elapsedTime)
{
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;
	auto th = owner->GetComponent<ThermalBody>();
	if (th->GetHeat() == 1)
	{
		if (death == false)
		{
			death = true;
			// 溶ける時の音（SE_game_reaction_fire.wavなどを指定）
			Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_game_reaction_fire.wav").c_str());
		}		
		timer -= elapsedTime;
	}

	if (death)
	{
		owner->GetComponent<Transform>()->SetLocalScale({ 1,timer,1 });
		eff->Stop();
		if (timer < 0)
		{
			owner->isDead = true;
		}
	}
}

void snowman::DrawInspector()
{
}

void snowman::Serialize(nlohmann::json& j) const
{
}

void snowman::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> snowman::Clone() const
{
	return std::make_unique<snowman>();
}
