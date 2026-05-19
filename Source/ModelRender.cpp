#include "ModelRender.h"
#include "Factory.h"
#include "ResourceManager.h"
#include "ImGuiFileDialog.h"
#include "Actor.h"

REGISTER_COMPONENT(ComponentID::ModelRender,ModelRender)

void ModelRender::Render(RenderContext& rc, ModelRenderer* renderer)
{
	if (!owner || !model) return;

	auto t = owner->GetComponent<Transform>();
	if (!t) return;

	// Instance登録だけする（描画はしない）
	renderer->AddInstance(model.get(), owner);
}

void ModelRender::DrawInspector()
{
	if (!modelpath.empty())
	{
		std::string filename = std::filesystem::path(modelpath).filename().string();
		ImGui::Text(filename.c_str());
	}

	if (ImGui::Button("Select Model"))
	{
		std::string full = OpenDialog::OpenLoadFileDialog();

		if (!full.empty())
		{
			modelpath = ToDataPath(full);

			model = std::make_unique<Model>(modelpath.c_str());
		}
	}
}

void ModelRender::PlayAnimation(int index, bool loop)
{
	animationPlaying = true;
	animationLoop = loop;
	animationIndex = index;
	animationSeconds = 0.0f;
}

void ModelRender::PlayAnimation(const char* name, bool loop)
{
	int index = 0;
	const std::vector<ModelResource::Animation>& animations = model->GetResource()->GetAnimations();
	for (const ModelResource::Animation& animation : animations)
	{
		if (animation.name == name)
		{
			PlayAnimation(index, loop);
			return;
		}
		++index;
	}
}

void ModelRender::UpdateAnimation(float elapsedTime)
{
	if (animationPlaying)
	{
		//アニメーション切り替え時のブレンド率を計算
		float blendRate = 1.0f;
		if (animationSeconds < animationBlendSecondsLength)
		{
			blendRate = (animationSeconds / animationBlendSecondsLength);
		}

		std::vector<Model::Node>& nodes = model->GetNodes();

		//アニメーションを取得
		const std::vector<ModelResource::Animation>& animations = model->GetResource()->GetAnimations();
		const ModelResource::Animation& animation = animations.at(animationIndex);

		animationSeconds += elapsedTime;
		blendRate *= blendRate;

		if (animationSeconds >= animation.secondsLength)
		{
			if (animationLoop)
			{
				animationSeconds = 0;
			}
			else
			{
				animationPlaying = false;
			}
		}

		const std::vector<ModelResource::Keyframe>& keyframes = animation.keyframes;
		int keyCount = static_cast<int>(keyframes.size());

		for (int keyIndex = 0; keyIndex < keyCount - 1; ++keyIndex)
		{
			//現在の時間がどのキーフレームかを判定する
			const ModelResource::Keyframe& keyframe0 = keyframes.at(keyIndex);
			const ModelResource::Keyframe& keyframe1 = keyframes.at(keyIndex + 1);
			if (animationSeconds >= keyframe0.seconds && animationSeconds < keyframe1.seconds)
			{
				//再生時間とキーフレームの時間から補完率を算出
				float rate = ((animationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds));

				//すべてのノードの姿勢を計算する
				int nodeCount = static_cast<int>(nodes.size());
				for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
				{
					const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
					const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);

					//ノード取得
					Model::Node& node = nodes[nodeIndex];

					if (blendRate < 1.0f)
					{
						//現在と次の姿勢を補完
						DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&node.scale);
						DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);
						DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&node.rotate);
						DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);
						DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&node.translate);
						DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

						DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, blendRate);
						DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, blendRate);
						DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, blendRate);

						DirectX::XMStoreFloat3(&node.scale, S);
						DirectX::XMStoreFloat4(&node.rotate, R);
						DirectX::XMStoreFloat3(&node.translate, T);
					}
					else
					{
						//前のキーフレームと次のキーフレームの姿勢を補完
						DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&key0.scale);
						DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);
						DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&key0.rotate);
						DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);
						DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&key0.translate);
						DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

						DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, rate);
						DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, rate);
						DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, rate);

						DirectX::XMStoreFloat3(&node.scale, S);
						DirectX::XMStoreFloat4(&node.rotate, R);
						DirectX::XMStoreFloat3(&node.translate, T);
					}
				}
			}
		}
	}
	//行列更新
	model->UpdateTransform();
}

void ModelRender::Flush(const RenderContext& rc,ModelRenderer* renderer)
{
	renderer->FlushAll(rc);
}

std::unique_ptr<Component> ModelRender::Clone() const
{
	auto c = std::make_unique<ModelRender>();
	c->SetModel(std::make_unique<Model>(modelpath.c_str()));
	if (modelpath != "")
	{
		c->SetString(modelpath.c_str());
	}
	return c;
}

void ModelRender::Serialize(nlohmann::json& j) const
{
	std::string safePath = std::string(modelpath.c_str());
	OutputDebugStringA(("modelpath: " + safePath + "\n").c_str());
	j["ModelPath"] = safePath;
}

void ModelRender::Deserialize(nlohmann::json& j)
{
	modelpath = j.value("ModelPath","");

	if (!modelpath.empty())
	{
		model = std::make_unique<Model>(modelpath.c_str());
	}
}