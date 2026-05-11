#pragma once


#include <DirectXMath.h>
//#include "Model.h"
#include "System/Model.h"
#include "imgui.h"
#include "CameraBase.h"

// コリジョン
class Hit
{
public:
	// レイキャスト
	static bool RayCast(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		const DirectX::XMFLOAT4X4& worldTransform,
		const Model* model,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal);

	static bool CreateRayFromImGui(
		CameraBase* camera,
		ImVec2 scenePos,
		ImVec2 sceneSize,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal);
};
