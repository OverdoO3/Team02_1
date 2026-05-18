#include "Actor.h"
#include "Camera.h"
#include "Scene.h"

Actor::Actor()
{
	auto t = std::make_unique<Transform>();
	transform = t.get();
	components[(int)t->GetID()] = (std::move(t));
	id = nextID++;
}

Actor::~Actor()
{
	for (auto& comp : components)
	{
		if (!comp) continue;

		if (auto* col = dynamic_cast<Collider*>(comp.get()))
		{
			scene->physics.DeleteQueueAdd(col); // êÊÇ…ó\ñÒ
		}
	}
}

void Actor::OnAwake(float elapsedTime)
{
	for (auto& comp : components)
	{
		if (!comp) continue;
		if (comp->enabled && !comp->isFinishAwake)
		{
			comp->OnAwake(elapsedTime);
			comp->isFinishAwake = true;
		}
	}
}

void Actor::Update(float elapsedTime)
{
	for (auto& comp : components)
	{
		if (!comp)continue;
		if (comp->enabled)
		{
			comp->Update(elapsedTime);
		}
	}
	GetComponent<Transform>()->UpdateTransform();

	for (auto& comp : components)
	{
		if (!comp)continue;
		if (comp->isDeleted)
		{
			RemoveComponent(comp->GetID());
		}
	}
}

void Actor::UpdateWithOutPlayed(float elapsedTime)
{
	if (parent&&parent->isDead)
	{
		this->isDead = true;
		return;
	}
	GetComponent<Transform>()->UpdateTransform();

	for (auto& comp : components)
	{
		if (!comp) continue;

		if (comp->isDeleted)
		{
			RemoveComponent(comp->GetID());
		}
	}
}

void Actor::Draw(RenderContext& rc)
{
	for (auto& comp : components)
	{
		if (!comp)continue;
		if (comp->enabled)
		{
			comp->Draw(rc);
		}
	}
}

void Actor::Render(RenderContext& rc, ModelRenderer* renderer)
{
	for (auto& comp : components)
	{
		if (!comp)continue;
		if (comp->enabled)
		{
			comp->Render(rc, renderer);
		}
	}
}

void Actor::RenderDebug(RenderContext& rc, ShapeRenderer* renderer)
{
	for (auto& comp : components)
	{
		if (!comp)continue;
		if (comp->enabled)
		{
			comp->RenderDebug(rc, renderer);
		}
	}
}

std::unique_ptr<Actor> Actor::Clone(bool play) const
{
	auto copy = std::make_unique<Actor>();

	//copy->SetScene(scene);

	if (!play)
	{
		copy->name = name + "copy";
	}
	else
	{
		copy->name = name;
	}
	copy->tag = tag;
	copy->id = id;
	copy->setActive = setActive;

	copy->parentid = parentid;
	copy->SetParent(nullptr);

	for (auto& comp : components)
	{
		if (!comp) continue;

		auto newComp = comp->Clone();
		copy->AddComponent(std::move(newComp));
	}

	copy->transform = copy->GetComponent<Transform>();

	copy->SetScene(scene);

	if (!play)
	{
		for (auto child : children)
		{
			auto childCopy = child->Clone();

			childCopy->SetParent(copy.get());
			scene->actors.emplace_back(std::move(childCopy));
		}
	}

	return copy;
}

bool Actor::IsDescendantOf(Actor* potentialParent)
{
	Actor* current = parent;

	while (current)
	{
		if (current == potentialParent)
			return true;

		current = current->GetParent();
	}

	return false;
}

void Actor::RegisterComp(Component* comp)
{
	if (!scene)return;
	if (auto* col = dynamic_cast<Collider*>(comp))
	{
		scene->GetPhysics()->QueueAdd(col);
	}
}

void Actor::UnRegisterComp(Component* comp)
{
	if (!scene)return;
	if (auto* col = dynamic_cast<Collider*>(comp))
	{
		scene->physics.DeleteQueueAdd(col); // Ç±Ç±èdóv
	}
}

void Actor::AddComponent(std::unique_ptr<Component> comp)
{
	comp->SetOwner(this);

	std::string na = (std::string)ComponentRegistry::IDToString(comp->GetID()) + " to " + name;

	LogManager::Instance().AddLog(
		LogCategory::component,
		LogEvent::Add,
		na
	);
	ComponentID id = comp->GetID();

	if (id == ComponentID::BoxCollider)
	{
		RegisterComp(comp.get());
	}

	components[(int)id] = std::move(comp);
}

Component* Actor::AddComponentByID(ComponentID id)
{
	auto comp = ComponentFactory::Create(id);
	if (!comp) return nullptr;

	comp->SetOwner(this);

	Component* ptr = comp.get();

	RegisterComp(ptr);

	components[(int)id] = std::move(comp);

	return ptr;
}

void Actor::SetScene(Scene* s)
{
	scene = s;

	if (!s) return;

	// SpriteRenderÇ…SceneManagerÇìnÇ∑
	if (auto* sr = GetComponent<SpriteRender>())
	{
		sr->SetSceneManager(s->sceneManager);
	}

}
