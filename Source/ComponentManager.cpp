#include "ComponentManager.h"
#include <string>
#include "Factory.h"

ComponentID ComponentRegistry::StringToID(const std::string& name)
{
    for (int i = 0; i < (int)ComponentID::COUNT; ++i)
    {
        if (name == std::string(ComponentFactory::GetName((ComponentID)i)))
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
