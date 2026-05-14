#pragma once

#include <DirectXMath.h>
#include "RenderState.h"

struct CbShadowParams
{
	float shadow_color;
	float shadow_bias;
	DirectX::XMFLOAT2 padding;
};

struct CbLightParams
{
	float lightIntensity;   //ñæÇÈÇ≥í≤êÆ
	float contrastPower;    //ïWèÄ1.0 
	float pointLightIntensity;
	float padding1;
};

struct CbOutlineParams
{
	float outlineThickness;
	DirectX::XMFLOAT3 outlinePadding;
};

struct  CbOutlineColor
{
	DirectX::XMFLOAT4 outlineColor;
};

struct RenderContext
{
	ID3D11DeviceContext* deviceContext;
	const RenderState* renderState = nullptr;

	DirectX::XMFLOAT4X4		view;
	DirectX::XMFLOAT4X4		projection;

	DirectX::XMFLOAT3 cameraPosition;

	DirectX::XMFLOAT3		lightDirection = { 0, -1, 0 };
	DirectX::XMFLOAT4		lightColor;
	DirectX::XMFLOAT4		ambientColor;
	DirectX::XMFLOAT4X4		lightViewProjection;


	ID3D11ShaderResourceView* shadowMap = nullptr;
	ID3D11SamplerState* shadowSampler = nullptr;
	CbShadowParams shadowParams;
	CbLightParams lightParams;
	CbOutlineParams outlineParams;
	CbOutlineColor outlineColor;
};