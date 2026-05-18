#pragma once
#include <array>
#include "ComponentManager.h"
#include "Component.h"

#define REGISTER_COMPONENT(id, type) \
    namespace { \
        const bool registered_##type = [] { \
            ComponentFactory::Register( \
                id, \
                [](){ return std::make_unique<type>(); }, \
                #type \
            ); \
            return true; \
        }(); \
    }

class ComponentFactory
{
private:
    using Creator = std::function<std::unique_ptr<Component>()>;

    struct ComponentInfo
    {
        Creator creator;
        const char* name;
    };

    static std::array<ComponentInfo, (int)ComponentID::COUNT>& GetTable()
    {
        static std::array<ComponentInfo, (int)ComponentID::COUNT> table{};
        return table;
    }

public:

    static void Register(ComponentID id, Creator c, const char* name)
    {
        GetTable()[(int)id] = { c, name };
    }

    static std::unique_ptr<Component> Create(ComponentID id)
    {
        auto& table = GetTable();
        if (!table[(int)id].creator) return nullptr;
        return table[(int)id].creator();
    }

    static const char* GetName(ComponentID id)
    {
        auto ta = GetTable();
        return ta[(int)id].name;
    }

    static std::vector<ComponentID> GetRegistered()
    {
        std::vector<ComponentID> list;

        auto& table = GetTable();

        for (int i = 0; i < (int)ComponentID::COUNT; i++)
        {
            if (table[i].creator)
            {
                list.push_back((ComponentID)i);
            }
        }

        return list;
    }
};