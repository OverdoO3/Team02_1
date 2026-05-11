#pragma once
#include "Collider.h"
#include "DirectXCommon.h"

class BoxCollider : public Collider
{
public:
	COMPONENT_ID(BoxCollider)
	DirectX::XMFLOAT3 size{};
	DirectX::XMFLOAT3 offset{};

	BoxCollider() {};
	~BoxCollider() {};

	AABB GetAABB() const override;

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(nlohmann::json& j) override;

	void DrawInspector() override;

	void RenderDebug(RenderContext& rc, ShapeRenderer* shapeRenderer) override;

	std::unique_ptr<Component> Clone() const override;
};