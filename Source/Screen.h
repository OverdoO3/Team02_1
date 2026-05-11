#pragma once
#include <DirectXMath.h>
#include "Camera.h"
#include "System/Graphics.h"

class Screen
{
public:
	static DirectX::XMFLOAT3 GetScreenWorld(const Camera* camera, const DirectX::XMFLOAT3 pos)
	{
		// スクリーンサイズ取得
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		//各行列の取得
		DirectX::XMMATRIX view = camera->GetView();
		DirectX::XMMATRIX View = view;
		DirectX::XMMATRIX proj = camera->GetProjection();
		DirectX::XMMATRIX Projection = proj;
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		//スクリーン座標にの設定
		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		DirectX::XMFLOAT3 screenPosition;
		screenPosition.x = (pos.x);
		screenPosition.y = (pos.y);

		screenPosition.z = pos.z;
		ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);
		//スクリーン座標をワールド座標に変換し、レイの始点を求める
		WorldPosition = DirectX::XMVector3Unproject(
			ScreenPosition,
			0.0f, 0.0f,
			screenWidth, screenHeight,
			0.0f, 1.0f,
			Projection,
			View,
			World
		);

		DirectX::XMFLOAT3 WPos;
		DirectX::XMStoreFloat3(&WPos, WorldPosition);

		return WPos;
	}

	static DirectX::XMFLOAT3 GetScreenCursorWorld(const CameraBase* camera, const float z)
	{
		// スクリーンサイズ取得
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		// マウスカーソル位置の取得
		POINT cursor;
		::GetCursorPos(&cursor);
		::ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		//各行列の取得
		DirectX::XMMATRIX view = camera->GetView();
		DirectX::XMMATRIX View = view;
		DirectX::XMMATRIX proj = camera->GetProjection();
		DirectX::XMMATRIX Projection = proj;
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		//スクリーン座標にの設定
		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		DirectX::XMFLOAT3 screenPosition;
		screenPosition.x = static_cast<float>(cursor.x);
		screenPosition.y = static_cast<float>(cursor.y);

		screenPosition.z = z;
		ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);
		//スクリーン座標をワールド座標に変換し、レイの始点を求める
		WorldPosition = DirectX::XMVector3Unproject(
			ScreenPosition,
			0.0f, 0.0f,
			screenWidth, screenHeight,
			0.0f, 1.0f,
			Projection,
			View,
			World
		);

		DirectX::XMFLOAT3 WPos;
		DirectX::XMStoreFloat3(&WPos, WorldPosition);

		return WPos;
	}

	static DirectX::XMFLOAT2 GetWorldToScreen(const Camera* camera, const DirectX::XMFLOAT3 pos)
	{
		// スクリーンサイズ取得
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		DirectX::XMMATRIX view = camera->GetView();
		DirectX::XMMATRIX View = view;
		DirectX::XMMATRIX proj = camera->GetProjection();
		DirectX::XMMATRIX Projection = proj;
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		//頭上のワールド座標
		DirectX::XMFLOAT3 spritPosition = pos;

		//ワールド座標からスクリーン座標に変換
		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		WorldPosition = DirectX::XMLoadFloat3(&spritPosition);

		ScreenPosition = DirectX::XMVector3Project(
			WorldPosition,
			0.0f, 0.0f,
			screenWidth, screenHeight,
			0.0f, 1.0f,
			Projection,
			View,
			World
		);

		//スクリーン座標
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		return screenPosition;
	}
};