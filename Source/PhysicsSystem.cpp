#include "PhysicsSystem.h"
#include "Collision.h"
#include "DirectXCommon.h"

void PhysicsSystem::QueueAdd(Collider* col)
{
    pendingAdd.push_back(col);
}

void PhysicsSystem::DeleteQueueAdd(Collider* col)
{
    pendingRemove.push_back(col);
}


bool PhysicsSystem::RayCast(const Ray& ray, float maxDist, RaycastHit& hit)
{
    float closest = maxDist;
    bool found = false;

    for (auto col : colliders)
    {
        float dist;
        DirectX::XMFLOAT3 normal;

        if (RayVsAABB(ray, col->GetAABB(), dist, normal))
        {
            if (dist < closest)
            {
                closest = dist;
                hit.collider = col;
                hit.distance = dist;
                hit.point = ray.origin + ray.direction * dist;
                hit.normal = normal;
                found = true;
            }
        }
    }

    return found;
}

bool PhysicsSystem::RayVsAABB(const Ray& ray, const AABB& aabb, float& t, DirectX::XMFLOAT3& normal)
{
    float tmin = 0.0f;
    float tmax = FLT_MAX;
    normal = { 0,0,0 };

    for (int i = 0; i < 3; i++)
    {
        float origin = (&ray.origin.x)[i];
        float dir = (&ray.direction.x)[i];
        float minB = (&aabb.min.x)[i];
        float maxB = (&aabb.max.x)[i];

        if (fabs(dir) < 1e-6f)
        {
            if (origin < minB || origin > maxB)
                return false;
        }
        else
        {
            float ood = 1.0f / dir;
            float t1 = (minB - origin) * ood;
            float t2 = (maxB - origin) * ood;

            float sign = -1.0f;
            if (t1 > t2)
            {
                std::swap(t1, t2);
                sign = 1.0f;
            }

            if (t1 > tmin)
            {
                tmin = t1;
                normal = { 0,0,0 };
                (&normal.x)[i] = sign;
            }

            tmax = min(tmax, t2);

            if (tmin > tmax)
                return false;
        }
    }

    t = tmin;
    return true;
}