#pragma once
#include "DirectXMath.h"

template<typename T>
inline T Lerp(const T& a, const T& b, float t)
{
	return a + (b - a) * t;
}

inline DirectX::XMFLOAT3 Lerp(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t)
{
	return {
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t
	};
}