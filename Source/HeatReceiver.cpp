#include "HeatReceiver.h"
#include "Factory.h"

//								Å´Ç…ñºëOì¸ÇÍÇÈ
REGISTER_COMPONENT(ComponentID::HeatReceiver, HeatReceiver)

void HeatReceiver::OnAwake(float elapsedTime)
{
}

void HeatReceiver::Update(float elapsedTime)
{
}

void HeatReceiver::DrawInspector()
{
	ImGui::InputInt("heat", &heatNum);
}

void HeatReceiver::Serialize(nlohmann::json& j) const
{
	j["temperature"] = heatNum;
}

void HeatReceiver::Deserialize(nlohmann::json& j)
{
	auto n = j["temperature"];
	heatNum = n;
}

std::unique_ptr<Component> HeatReceiver::Clone() const
{
	auto h = std::make_unique<HeatReceiver>();
	h->heatNum = this->heatNum;
	return h;
}