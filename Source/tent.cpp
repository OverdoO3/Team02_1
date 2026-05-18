#include "tent.h"
#include "Actor.h"
#include "Factory.h"

//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::tent, tent)

void tent::OnAwake(float elapsedTime)
{
	auto model = owner->GetComponent<ModelRender>();
	model->PlayAnimation("hata", true);
}

void tent::Update(float elapsedTime)
{
	auto model = owner->GetComponent<ModelRender>();

	model->UpdateAnimation(elapsedTime);
}

void tent::DrawInspector()
{
}

void tent::Serialize(nlohmann::json& j) const
{
}

void tent::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> tent::Clone() const
{
	return std::make_unique<tent>();
}
