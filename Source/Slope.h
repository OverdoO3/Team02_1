#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class Slope : public Component
{
public:
    Slope() = default;
    ~Slope() override = default;

    void OnAwake(float elapsedTime) override;
    void Update(float elapsedTime) override;

    void DrawInspector() override;

    void Serialize(nlohmann::json& j) const override;
    void Deserialize(nlohmann::json& j) override;

    std::unique_ptr<Component> Clone() const override;

    float GetSurfaceY(float worldX, float worldZ, DirectX::XMFLOAT3 origin) const
    {
        // ワールド→スロープローカル
        float dx = worldX - origin.x;
        float dz = worldZ - origin.z;
        float cosY = cosf(-dirYaw), sinY = sinf(-dirYaw);
        float lx = cosY * dx + sinY * dz;
        float lz = -sinY * dx + cosY * dz;

        // 坂の底面Y（originは中心なのでsize.y/2を引く）
        float baseY = origin.y - (size.y * 0.5f);

        // 傾きに沿ったY（lzが-size.z/2のとき底面、+size.z/2のとき頂上）
        float surfaceY = baseY + (-lz + size.z * 0.5f) * tanf(angle);

        return surfaceY;
    }

    DirectX::XMFLOAT3 size = { 10,10,10 };
    float             angle = 0.785f;   // X軸回転（ラジアン）
    float             dirYaw = 0;   // 坂の向き（Y軸回転）

    COMPONENT_ID(Slope)

private:
    
};