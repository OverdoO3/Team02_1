#include "Move.h"
#include "Scene.h"
#include "Lerp.h"
#include <RayCast.h>
#include "Slope.h"
#include "Factory.h"

REGISTER_COMPONENT(ComponentID::Move,Move)

void Move::OnAwake(float elapsedTime)
{
    auto model = owner->GetComponent<ModelRender>();
}

void Move::Update(float elapsedTime)
{
    if (owner->GetScene()->isClear)
    {
        owner->GetComponent<Transform>()->LookAt(owner->GetScene()->GetCamera()->GetComponent<Transform>()->GetLocalPosition());
        return;
    }
    auto transform = owner->GetComponent<Transform>();
    transform->UpdateTransform();
    auto cam = owner->GetScene()->GetCamera()->GetComponent<Transform>();
    cam->UpdateTransform();
    auto pos = transform->GetWorldPosition();
    auto model = owner->GetComponent<ModelRender>();

    //=========================
    // 入力
    //=========================
    float ax = 0, ay = 0;
    {  
        if (InputC::KeyDown('W')) ay += 1;
        if (InputC::KeyDown('S')) ay -= 1;
        if (InputC::KeyDown('D')) ax += 1;
        if (InputC::KeyDown('A')) ax -= 1;
    }

    //=========================
    // カメラ基準方向
    //=========================
    auto f = cam->GetForward();
    auto r = cam->GetRight();

    float fl = sqrtf(f.x * f.x + f.z * f.z);
    float rl = sqrtf(r.x * r.x + r.z * r.z);

    if (fl > 0.0001f) { f.x /= fl; f.z /= fl; }
    if (rl > 0.0001f) { r.x /= rl; r.z /= rl; }

    DirectX::XMFLOAT3 moveDir = {
        f.x * ay + r.x * ax,
        0,
        f.z * ay + r.z * ax
    };

    float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

    if (len > 0.0001f)
    {
        moveDir.x /= len;
        moveDir.z /= len;


        float targetYaw = atan2f(moveDir.x, moveDir.z);

        float currentYaw = transform->GetEulerRotation().y;

        // 補間（スムーズ回転）
        float diff = targetYaw - currentYaw;

        while (diff > DirectX::XM_PI) diff -= DirectX::XM_2PI;
        while (diff < -DirectX::XM_PI) diff += DirectX::XM_2PI;

        float newYaw = currentYaw + diff * min(1.0f, speed * elapsedTime);

        transform->SetRotationEulerYaw(newYaw);

        nextState = walk;
    }
    else
    {
        nextState = idle;
    }


    //=========================
    // 速度
    //=========================
    DirectX::XMFLOAT3 targetVel = {
        moveDir.x * speed,
        0,
        moveDir.z * speed
    };

    Velocity.x = Lerp(Velocity.x, targetVel.x, 0.2f);
    Velocity.z = Lerp(Velocity.z, targetVel.z, 0.2f);

    //=========================
    // nextPos
    //=========================
    DirectX::XMFLOAT3 nextPos = pos;

    nextPos.x += Velocity.x * elapsedTime;
    nextPos.z += Velocity.z * elapsedTime;

//=========================
// 地面Ray（AABB版）
//=========================
    bool onGround = false;
    RaycastHit hit;

    for (auto& actor : owner->GetScene()->actors)
    {
        if (actor.get() == owner) continue;
        if (actor->tag != 2) continue;

        auto oPos = actor->GetComponent<Transform>()->GetWorldPosition();

        //=========================
        // Slope優先チェック
        //=========================
        auto slope = actor->GetComponent<Slope>();
        if (slope)
        {
            auto model = actor->GetComponent<ModelRender>();
            if (!model || !model->GetModel()) goto next_actor;

            actor->GetComponent<Transform>()->UpdateTransform();

            DirectX::XMFLOAT3 start = { nextPos.x, nextPos.y + height, nextPos.z };
            DirectX::XMFLOAT3 end = { nextPos.x, nextPos.y - 5.0f, nextPos.z };

            DirectX::XMFLOAT3 tmpPos, tmpNormal;

            if (Hit::RayCast(
                start,
                end,
                actor->GetComponent<Transform>()->GetWorldMatrix(),
                model->GetModel(),
                tmpPos,
                tmpNormal))
            {
                hit.point = tmpPos;
                hit.normal = tmpNormal;
                onGround = true;
                break;
            }
            goto next_actor;
        }

        //=========================
        // 通常床チェック（BoxColliderのみ）
        //=========================
        {
            auto col = actor->GetComponent<BoxCollider>();
            if (!col) goto next_actor;

            auto size = col->size;

            float boxTop = oPos.y + size.y * 0.5f;

            if (nextPos.x < oPos.x - size.x * 0.5f || nextPos.x > oPos.x + size.x * 0.5f) goto next_actor;
            if (nextPos.z < oPos.z - size.z * 0.5f || nextPos.z > oPos.z + size.z * 0.5f) goto next_actor;
             
            // 上面をまたいでいるか
            if (nextPos.y + height < boxTop) goto next_actor; // 完全に下
            if (nextPos.y - height > boxTop) goto next_actor; // 完全に上（空中）

            hit.point = { nextPos.x, boxTop, nextPos.z };
            hit.normal = { 0, 1, 0 };
            onGround = true;
            break;
        }

    next_actor:;
    }

    if (!onGround) nextState = Move::fall;

    //=========================
    // 地面処理
    //=========================
    if (onGround && hit.normal.y > 0.5f)
    {
        float dot = Velocity.x * hit.normal.x
            + Velocity.y * hit.normal.y
            + Velocity.z * hit.normal.z;

        Velocity.x -= hit.normal.x * dot;
        Velocity.y -= hit.normal.y * dot;  // ← Yも法線に沿って補正
        Velocity.z -= hit.normal.z * dot;

        nextPos.y = hit.point.y + radius;
    }
    else
    {
        // 空中慣性
        Velocity.x *= 0.01f;
        Velocity.z *= 0.01f;

        // 重力
        Velocity.y -= grav * elapsedTime;

        nextPos.x += Velocity.x * elapsedTime;
        nextPos.z += Velocity.z * elapsedTime;
        nextPos.y += Velocity.y * elapsedTime;
    }

    //=========================
    // 壁衝突（XZ）
    //=========================
    for (auto& other : owner->GetScene()->actors)
    {
        if (other.get() == owner) continue;

        auto col = other->GetComponent<BoxCollider>();
        if (!col) continue;

        auto oPos = other->GetComponent<Transform>()->GetWorldPosition();
        auto size = col->size;

        float top = nextPos.y + height;
        float bottom = nextPos.y;

        float otherTop = oPos.y + size.y * 0.5f;
        float otherBottom = oPos.y - size.y * 0.5f;

        if (top < otherBottom || bottom > otherTop)
            continue;

        float halfX = size.x * 0.5f;
        float halfZ = size.z * 0.5f;

        float closestX = std::clamp(nextPos.x, oPos.x - halfX, oPos.x + halfX);
        float closestZ = std::clamp(nextPos.z, oPos.z - halfZ, oPos.z + halfZ);

        float dx = nextPos.x - closestX;
        float dz = nextPos.z - closestZ;

        float distSq = dx * dx + dz * dz;

        float r = radius + 0.05f;

        if (distSq < r * r)
        {
            float dist = sqrtf(distSq);

            if (other->tag == 3)
            {
                owner->GetScene()->isClear = true;
                nextState = Move::goal;
            }

            if (dist > 0.0001f)
            {
                // ベベル・床除外
                float surfaceY = oPos.y + size.y * 0.5f;
                if (nextPos.y >= surfaceY - 0.1f) continue;

                float push = r - dist;

                dx /= dist;
                dz /= dist;

                nextPos.x += dx * push;
                nextPos.z += dz * push;

                float dot = Velocity.x * dx + Velocity.z * dz;

                Velocity.x -= dx * dot;
                Velocity.z -= dz * dot;
            }
        }
    }

    //=========================
    // 確定
    //=========================
    transform->SetWorldPosition(nextPos);

    if (currentState != nextState)
    {
        switch (nextState)
        {
        case Move::idle:
            model->PlayAnimation("idle", true);
            break;
        case Move::walk:
            model->PlayAnimation("walk", true);
            break;
        case Move::in_out:
            model->PlayAnimation("in_out", true);
            break;
        case Move::land:
            model->PlayAnimation("land", true);
            break;
        case Move::fall:
            model->PlayAnimation("fall", true);
            break;
        case Move::goal:
            model->PlayAnimation("goal", true);
            break;
        default:
            break;
        }
    }

    currentState = nextState;
}

void Move::DrawInspector()
{
	ImGui::InputFloat("speed", &speed);
	ImGui::InputFloat("turnSpeed", &turnSpeed);
	ImGui::InputFloat("gravity", &grav);
    ImGui::InputFloat("radius", &radius);
}

void Move::Serialize(nlohmann::json& j) const
{
    j["grav"] = grav;
    j["turn"] = turnSpeed;
    j["radius"] = radius;
    j["spped"] = speed;
}

void Move::Deserialize(nlohmann::json& j)
{
    /*grav = j["grav"];
    turnSpeed = j["turn"];
    radius = j["radius"];
    speed =  j["spped"];*/
}

std::unique_ptr<Component> Move::Clone() const
{
	return std::make_unique<Move>(*this);
}

void Move::RenderDebug(RenderContext& rc, ShapeRenderer* shapeRenderer)
{
    DirectX::XMFLOAT4X4 tran = owner->GetComponent<Transform>()->GetWorldMatrix();
    shapeRenderer->RenderCapsule(rc, tran, radius, height, { 1,1,1,1 });
}