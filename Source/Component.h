#pragma once
#include "DirectXMath.h"
#include "System/Graphics.h"
#include "LogManager.h" 
#include <filesystem>
#include "ComponentManager.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Actor;

class Component
{
public:
	virtual ~Component() = default;

    virtual void OnAwake(float elapsedTime) {};
	virtual void Update(float elapsedTime) {};
	virtual void Draw(RenderContext& rc) {};
	virtual void Render(RenderContext& rc, ModelRenderer* renderer) {};
	virtual void RenderDebug(RenderContext& rc, ShapeRenderer* renderer) {};

	virtual void DrawInspector() {};

    virtual std::unique_ptr<Component> Clone() const = 0;

	void SetOwner(Actor* owner) { this->owner = owner; }

	virtual void Serialize(nlohmann::json& j)const {};
	virtual void Deserialize(nlohmann::json& j){};

    virtual ComponentID GetID() const = 0;

	bool enabled = true;

    bool isDeleted = false;

	bool isFinishAwake = false;
protected:
	Actor* owner = nullptr;
};