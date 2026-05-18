#include "ComponentManager.h"
#include <string>
#include "Factory.h"

ComponentID ComponentRegistry::StringToID(const std::string& name)
{
    for (int i = 0; i < (int)ComponentID::COUNT; ++i)
    {
        std::string n = std::string(ComponentFactory::GetName((ComponentID)i));
        if (name == n)
        {
            return (ComponentID)i;
        }
    }
    return ComponentID::COUNT;
}

const char* ComponentRegistry::IDToString(ComponentID id)
{
    return ComponentFactory::GetName(id);
}
