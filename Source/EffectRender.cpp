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

	DirectX::XMFLOAT4 q = transform->GetWorldRotation();
	DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&q);

	// クォータニオンから回転行列
	DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion(quat);

	// DirectXの関数でオイラー角を取り出す（ジンバルロック回避）
	DirectX::XMVECTOR scale, rotQuat, trans;
	DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, rotMat);

	float rotY;
	// Y軸回転だけ取り出す
	DirectX::XMFLOAT4 rq;
	DirectX::XMStoreFloat4(&rq, rotQuat);
	rotY = atan2f(2.0f * (rq.w * rq.y + rq.x * rq.z),
		1.0f - 2.0f * (rq.y * rq.y + rq.z * rq.z));

	effect->SetRotation(handle, { 0, rotY - DirectX::XM_PIDIV2, 0 });

	DirectX::XMFLOAT3 pos = position + offset;
	// 再生終了チェック
	if (handle != -1)
	{
		effect->SetPosition(handle, pos);
		if (!effect->Exists(handle))
		{
			handle = -1;

			if (loop)
			{
				Play();
			}
		}
	}

	if (!this->enabled)
	{
		Stop();
	}
}

void EffectRender::DrawInspector()
{
	ImGui::Checkbox("loop", &loop);
	ImGui::Checkbox("StartPlay", &playOnStart);
	ImGui::DragFloat3("effoffset", &offset.x);
	if (ImGui::Button("Select"))
	{
		effectPath = OpenDialog::OpenLoadFileDialog();
		effectPath = ToDataPath(effectPath);
		effect = EffectManager::Instance().LoadEffect(effectPath);
	}
	ImGui::DragFloat("Scale", &scale);

	ImGui::Text(effectPath.c_str());
}

void EffectRender::Serialize(nlohmann::json& j) const
{
	j["loop"] = loop;
	j["StartPlay"] = playOnStart;
	j["scale"] = scale;
	j["path"] = effectPath;
	j["offset"] = 
	{ offset.x,offset.y,offset.z };
}

void EffectRender::Deserialize(nlohmann::json& j)
{
	loop = j["loop"];
	scale = j["scale"];
	effectPath = j["path"];
	playOnStart = j.value("StartPlay",false);
	auto p = j["offset"];
	offset.x = p[0];
	offset.x = p[1];
	offset.x = p[2];
	if (effectPath != "")
	{
		effect = EffectManager::Instance().LoadEffect(effectPath.c_str());
	}
}

std::unique_ptr<Component> EffectRender::Clone() const
{
	auto e = std::make_unique<EffectRender>();
	e->loop = loop;
	e->playOnStart = playOnStart;
	e->effectPath = effectPath;
	e->scale = scale;
	e->offset = offset;
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
	position = offset + position;
	if (handle != -1)
	{
		effect->Play(position, scale);
		effect->SetPosition(handle, position);
		effect->SetRotation(handle, rotation);
	}
	else
	{
		handle = effect->Play(position,scale);
		effect->SetPosition(handle, position);
		effect->SetRotation(handle, rotation);
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
