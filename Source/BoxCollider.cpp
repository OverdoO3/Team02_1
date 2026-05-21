#include "BoxCollider.h"
#include "Actor.h"
#include "Factory.h"
#include "Scene.h"

REGISTER_COMPONENT(ComponentID::BoxCollider,BoxCollider)

AABB BoxCollider::GetAABB() const
{
	auto tran = owner->GetComponent<Transform>();
	DirectX::XMFLOAT3 pos = tran->GetWorldPosition();
	pos = pos + offset;

	return {
		pos - size * 0.5f,
		pos + size * 0.5f
	};
}

void BoxCollider::Serialize(json& j)const
{
	j["size"] = { size.x,size.y,size.z };
	j["offset"] = { offset.x,offset.y,offset.z };
}

void BoxCollider::Deserialize(json& j)
{
	auto s = j["size"];
	if (s != nullptr)
	{ 
		size = { s[0], s[1], s[2] };
	}
	auto o = j["offset"];
	if (o != nullptr)
	{
		offset = { o[0], o[1], o[2] };
	}
}

void BoxCollider::DrawInspector()
{
	ImGui::InputFloat3("size", &size.x);
	ImGui::InputFloat3("offset", &offset.x);
}

void BoxCollider::RenderDebug(RenderContext& rc, ShapeRenderer* shapeRenderer)
{
	DirectX::XMFLOAT3 pos = owner->GetComponent<Transform>()->GetWorldPosition();
	pos = offset + pos;

	shapeRenderer->RenderBox(rc, pos, { 0,0,0 }, { size.x / 2,size.y / 2,size.z / 2 }, { 1,1,1,1 });
}

std::unique_ptr<Component> BoxCollider::Clone() const
{
	auto box = std::make_unique<BoxCollider>();

	box->size = size;
	box->offset = offset;

	return box;
}

