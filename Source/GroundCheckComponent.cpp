#include "GroundCheckComponent.h"
#include "Factory.h"
#include "Scene.h"

REGISTER_COMPONENT(ComponentID::GroundCheckComponent, GroundCheckComponent)

std::unique_ptr<Component> GroundCheckComponent::Clone() const
{
    return std::unique_ptr<GroundCheckComponent>();
}

void GroundCheckComponent::CheckGround(float elapsedTime)
{
    auto transform = owner->GetComponent<Transform>();

    Ray ray;
    ray.origin = transform->GetWorldPosition() + DirectX::XMFLOAT3(0, offsetY, 0);
    ray.direction = DirectX::XMFLOAT3(0, -1, 0);

    RaycastHit hit;

    if (physics->RayCast(ray, rayLength, hit))
    {
        isGrounded = true;
        groundY = hit.point.y;
        groundNormal = hit.normal;
    }
    else
    {
        isGrounded = false;
    }
}