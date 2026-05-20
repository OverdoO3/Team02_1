#include "flower.h"
#include "Factory.h"
#include "Actor.h"
//								«‚É–¼‘O“ü‚ê‚é
REGISTER_COMPONENT(ComponentID::flower, flower)

void flower::OnAwake(float elapsedTime)
{
}

void flower::Update(float elapsedTime)
{
	auto temp = owner->GetComponent<ThermalBody>();
	if (!temp)return;
	auto boxCollider = owner->GetComponent<BoxCollider>();
	if (!boxCollider)return;
	auto model = owner->GetComponent<ModelRender>();
	if (!model)return;

	switch (temp->GetHeat())
	{
	case -1:
		boxCollider->size.y = openY;
		model->SetString(openPath);
		break;
	case 1:
		boxCollider->size.y = deathY;
		death = true;
		break;
	default:
		break;
	}

	if (death)
	{
		if (timer > 0)
		{
			timer -= elapsedTime;
			auto tran = owner->GetComponent<Transform>();
			tran->SetLocalScale({1,timer,1});
		}
	}

	if (deathY == 0)
	{
		boxCollider->enabled = false;
	}
}

void flower::DrawInspector()
{
	ImGui::InputFloat("openY", &openY);
	ImGui::InputFloat("closeY", &deathY);
}

void flower::Serialize(nlohmann::json& j) const
{
	j["openY"] = openY;
	j["closeY"] = deathY;
}

void flower::Deserialize(nlohmann::json& j)
{
	openY = j["openY"];
	deathY = j["closeY"];
}

std::unique_ptr<Component> flower::Clone() const
{
	auto c = std::make_unique<flower>();
	c->openY = openY;
	c->deathY = deathY;
	return c;
}
