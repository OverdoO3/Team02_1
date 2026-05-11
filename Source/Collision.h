#pragma once
#include<DirectXMath.h>

class Collision
{
public:
	//‹…‚Æ‹…‚ÌŒğ·”»’è
	static bool IntersectSphereVsSphere(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		DirectX::XMFLOAT3& outPositionB
	);

	//‹…
	static bool IntersectSphereVsSphere(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB
	);

	//‰~’Œ“¯m‚ÌŒğ·
	static bool IntersectCylinderVsCylinder(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		float heightA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		float heightB,
		DirectX::XMFLOAT3& outPositionB
	);
	//‹…‚Æ‰~’Œ‚ÌŒğ·
	static bool IntersectSphereVsCylinder(
		const DirectX::XMFLOAT3& spherePosition,
		float sphereRadius,
		const DirectX::XMFLOAT3& cylinderPosition,
		float cylinderRadius,
		float cylinderHeight,
		DirectX::XMFLOAT3& outCylinderPosition
	);
	//lŠp‚ÆlŠp
	static bool IntersectBoxVsBox(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& posB,
		const DirectX::XMFLOAT3& lengthB,
		DirectX::XMFLOAT3& outPosition
	);

	//lŠp‚Æ‰~’Œ
	static bool IntersectBoxVsCylinder(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& cylinderP,
		float cylinderR,
		float cylinderH,
		DirectX::XMFLOAT3& outPosition
	);
};