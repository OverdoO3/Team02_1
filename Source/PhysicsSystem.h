#pragma once
#include <vector>
#include <Collider.h>

class PhysicsSystem
{
public:
	PhysicsSystem() 
	{
		Flush();
	};
	~PhysicsSystem() {};

	void Flush()
	{
		for (auto* c : pendingAdd)
			colliders.push_back(c);

		for (auto* c : pendingRemove)
			Unregister(c);

		pendingAdd.clear();
		pendingRemove.clear();
	}

	void QueueAdd(Collider* col);
	void DeleteQueueAdd(Collider* col);

	void DeleteAllQueue() { pendingAdd.clear(); }

	void Register(Collider* col)
	{
		colliders.push_back(col);
	}

	void Unregister(Collider* col)
	{
		colliders.erase(
			std::remove(colliders.begin(), colliders.end(), col),
			colliders.end());
	}

	bool RayCast(const Ray& ray, float maxDist,RaycastHit& hit);

	bool RayVsAABB(const Ray& ray, const AABB& aabb, float& t, DirectX::XMFLOAT3& normal);

	std::vector<Collider*> GetColliders() { return colliders; }
private:
	std::vector<Collider*> colliders;
	std::vector<Collider*> pendingAdd;
	std::vector<Collider*> pendingRemove;
};