#include "collision.h"
#include <algorithm>
using namespace DirectX;

bool Collision::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, float radiusA, const DirectX::XMFLOAT3& positionB, float radiusB, DirectX::XMFLOAT3& outPositionB)
{
    //A→Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = PositionB - PositionA;
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    //距離判定
    float range = radiusA + radiusB;
    if (lengthSq >= range)
    {
        return false;
    }
    DirectX::XMVECTOR unitVec = DirectX::XMVector3Normalize(Vec);

    DirectX::XMVECTOR newPosB = PositionA + unitVec * (radiusA + radiusB);

    DirectX::XMStoreFloat3(&outPositionB, newPosB);

    return true;
}

bool Collision::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, float radiusA, const DirectX::XMFLOAT3& positionB, float radiusB)
{
    //A→Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = PositionB - PositionA;
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
    float lengthSq = DirectX::XMVectorGetX(LengthSq);

    //距離判定
    float range = radiusA + radiusB;
    return lengthSq <= range * range;
}

bool Collision::IntersectCylinderVsCylinder(const DirectX::XMFLOAT3& positionA, float radiusA, float heightA, const DirectX::XMFLOAT3& positionB, float radiusB, float heightB, DirectX::XMFLOAT3& outPositionB)
{
    //Aの足元がBの頭より上なら当たっていない
    if (positionA.y > heightB+positionB.y)
    {
        return false;
    }
    //Ａの頭がＢの足元より↓なら当たっていない
    if (positionB.y > heightA+positionA.y)
    {
        return false;
    }
    //XZ平面での範囲チェック
    float vx = positionB.x - positionA.x;
    float vz = positionB.z - positionA.z;
    float range = radiusA + radiusB;
    float distXZ = sqrtf(vx * vx + vz * vz);
    if (distXZ > range)
    {
        return false;
    }
    //単位ベクトル
    float overlap = radiusA + radiusB - distXZ;
    outPositionB.x = positionB.x + (vx / distXZ) * overlap;
    outPositionB.y = positionB.y;
    outPositionB.z = positionB.z + (vz / distXZ) * overlap;
    return true;
}

bool Collision::IntersectSphereVsCylinder(const DirectX::XMFLOAT3& spherePosition, float sphereRadius,
    const DirectX::XMFLOAT3& cylinderPosition, float cylinderRadius, float cylinderHeight, DirectX::XMFLOAT3& outCylinderPosition)
{
    //球の底が円柱の頭より上にある
    if (spherePosition.y - sphereRadius > cylinderHeight+cylinderPosition.y)
    {
        return false;
    }
    //円柱の底が球の上よりも上にある
    if (cylinderPosition.y > spherePosition.y + sphereRadius)
    {
        return false;
    }
    //XZ平面
    float vx = spherePosition.x - cylinderPosition.x;
    float vz = spherePosition.z - cylinderPosition.z;
    float range = sphereRadius + cylinderRadius;
    float length = sqrtf(vx * vx + vz * vz);
    if (length > range)
    {
        return false;
    }
    //単位ベクトル
    float overlap = sphereRadius + cylinderRadius - length;
    outCylinderPosition.x = cylinderPosition.x + (vx / length) * overlap;
    outCylinderPosition.y = cylinderPosition.y;
    outCylinderPosition.z = cylinderPosition.z + (vz / length) * overlap;
    return true;
}

//四角と四角の当たり判定
bool Collision::IntersectBoxVsBox(
    const DirectX::XMFLOAT3& posA,
    const DirectX::XMFLOAT3& lengthA,
    const DirectX::XMFLOAT3& posB,
    const DirectX::XMFLOAT3& lengthB,
    DirectX::XMFLOAT3& outPosition)
{
    DirectX::XMFLOAT3 StartPosA;
    DirectX::XMFLOAT3 StartPosB;

    StartPosA.x = posA.x - (lengthA.x * 0.5f);
    StartPosA.z = posA.z - (lengthA.z * 0.5f);
    StartPosA.y = posA.y;

    StartPosB.x = posB.x - (lengthB.x * 0.5f);
    StartPosB.z = posB.z - (lengthB.z * 0.5f);
    StartPosB.y = posB.y;

    return !(StartPosA.x + lengthA.x < StartPosB.x || //BがAの外にあるか
        StartPosB.x + lengthB.x < StartPosA.x || //AがBの外にあるか
        StartPosA.y + lengthA.y < StartPosB.y || //BがAの外にあるか
        StartPosB.y + lengthB.y < StartPosA.y || //AがBの外にあるか
        StartPosA.z + lengthA.z < StartPosB.z || //BがAの外にあるか
        StartPosB.z + lengthB.z < StartPosA.z	  //AがBの外にあるか
        );
}

//四角と円柱の当たり判定
bool Collision::IntersectBoxVsCylinder(
    const DirectX::XMFLOAT3& posA,
    const DirectX::XMFLOAT3& lengthA,
    const DirectX::XMFLOAT3& cylinderP,
    float cylinderR,
    float cylinderH,
    DirectX::XMFLOAT3& outPosition
)
{

    if (isnan(outPosition.x))
    {
        int a = 0;
    }

    if (posA.y + lengthA.y < cylinderP.y) return false;
    if (cylinderP.y + cylinderH < posA.y) return false;

    {
        //四角の始まりから終わりの場所まで
        float boxMinX = posA.x - lengthA.x;
        float boxMaxX = posA.x + lengthA.x;
        float boxMinZ = posA.z - lengthA.z;
        float boxMaxZ = posA.z + lengthA.z;

        //円の中心から四角の最近接点を求める
        float closestX = std::max(boxMinX, std::min(cylinderP.x, boxMaxX));
        float closestZ = std::max(boxMinZ, std::min(cylinderP.z, boxMaxZ));

        //円の中心から四角の最接近点までのベクトルを取る
        float dx = cylinderP.x - closestX;
        float dz = cylinderP.z - closestZ;

        //円と四角形の距離が半径内なら衝突
        if ((dx * dx + dz * dz) <= (cylinderR * cylinderR))
        {
            float dist = { sqrtf(dx * dx + dz * dz) };
            dx /= dist;
            dz /= dist;
            float push = cylinderR - dist;

            outPosition.x = cylinderP.x + dx * push;
            outPosition.y = cylinderP.y;
            outPosition.z = cylinderP.z + dz * push;

            return true;
        }
        return false;
    }
}

