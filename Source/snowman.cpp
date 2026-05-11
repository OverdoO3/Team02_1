#include "snowman.h"
#include "Factory.h"

//								Å´Ç…ñºëOì¸ÇÍÇÈ
REGISTER_COMPONENT(ComponentID::snowman, snowman)

void snowman::OnAwake(float elapsedTime)
{
}

void snowman::Update(float elapsedTime)
{
}

void snowman::DrawInspector()
{
}

void snowman::Serialize(nlohmann::json& j) const
{
}

void snowman::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> snowman::Clone() const
{

	return std::unique_ptr<snowman>();
}
