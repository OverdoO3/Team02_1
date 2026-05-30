#include "justDance.h"
#include "Factory.h"
#include "Actor.h"
#include "Input.h"
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::justDance, justDance)

void justDance::OnAwake(float elapsedTime)
{
	owner->GetComponent<ModelRender>()->PlayAnimation("goal", true);
}

void justDance::Update(float elapsedTime)
{
	DirectX::XMFLOAT3 ow = owner->transform->GetEulerRotation();
	if (InputC::KeyDown(VK_LBUTTON))
	{
		owner->transform->SetRotationEuler(ow.z, ow.y + elapsedTime * 20.0f, ow.z);
	}
}

void justDance::DrawInspector()
{
}

void justDance::Serialize(nlohmann::json& j) const
{
}

void justDance::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> justDance::Clone() const
{
	return std::make_unique<justDance>();
}
