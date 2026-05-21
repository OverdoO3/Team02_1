#include "Coin.h"
#include "Factory.h"
#include "Actor.h"
#include "Collision.h"
#include "Scene.h"
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::Coin, Coin)

void Coin::OnAwake(float elapsedTime)
{
}

void Coin::Update(float elapsedTime)
{
	auto playerModelChanger = owner->GetScene()->FindByTag(1);
	auto transform = owner->GetComponent<Transform>();

	auto ro = transform->GetEulerRotation();
	ro.y += elapsedTime * 4;
	transform->SetRotationEuler(ro.x,ro.y,ro.z);

	if (Collision::IntersectSphereVsSphere(transform->GetWorldPosition(), 1, playerModelChanger->GetComponent<Transform>()->GetWorldPosition(), 10))
	{
		owner->GetComponent<ModelRender>()->enabled = false;
	}
}

void Coin::DrawInspector()
{
}

void Coin::Serialize(nlohmann::json& j) const
{
}

void Coin::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> Coin::Clone() const
{
	return std::make_unique<Coin>();
}
