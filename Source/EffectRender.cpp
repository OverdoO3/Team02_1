#include "EffectRender.h"
#include "Actor.h"
#include "Factory.h"
#include "OpenDialog.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::EffectRender, EffectRender)

void EffectRender::OnAwake(float elapsedTime)
{
	isFinishAwake = true;
	if (playOnStart)
	{
		Play();
	}
}

void EffectRender::Update(float elapsedTime)
{
	auto transform = owner->GetComponent<Transform>();

	position = transform->GetWorldPosition();

	// 再生終了チェック
	if (handle != -1)
	{
		effect->SetPosition(handle, position);
		if (!effect->Exists(handle))
		{
			handle = -1;

			if (loop)
			{
				Play();
			}
		}
	}
}

void EffectRender::DrawInspector()
{
	ImGui::Checkbox("loop", &loop);
	ImGui::Checkbox("StartPlay", &playOnStart);

	ImGui::Text("effectpath", effectPath);
}

void EffectRender::Serialize(nlohmann::json& j) const
{
	j["loop"] = loop;
}

void EffectRender::Deserialize(nlohmann::json& j)
{
	loop = j["loop"];
}

std::unique_ptr<Component> EffectRender::Clone() const
{
	auto e = std::make_unique<EffectRender>();
	e->loop = loop;
	e->playOnStart = playOnStart;
	e->effectPath = effectPath;
	if (!effectPath.empty())
	{
		e->effect = std::make_unique<Effect>();
		e->effect->Load(effectPath.c_str());
	}
	return e;
}

void EffectRender::Play()
{
	if (!effect) return;

	auto transform = owner->GetComponent<Transform>();
	position = transform->GetWorldPosition();
	
	if (handle != -1)
	{
		effect->Play(position, scale);
		effect->SetPosition(handle, position);
	}
	else
	{
		handle = effect->Play(position,scale);
		effect->SetPosition(handle, position);
	}

}

void EffectRender::Stop()
{
	if (!effect) return;

	if (handle != -1)
	{
		effect->Stop(handle);
		handle = -1;
	}
}

void EffectRender::OnDestroy()
{
	Stop();
}

void EffectRender::SetEffect(const std::shared_ptr<Effect>& eff)
{
	effect = eff;
}

void EffectRender::SetScale(float sca)
{
	scale = sca;
}
