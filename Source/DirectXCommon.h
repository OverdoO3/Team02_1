#pragma once

#include <DirectXMath.h>

class DirectXCommon
{
public:
    //ベクトルを作る
    static DirectX::XMVECTOR CreateVector(DirectX::XMFLOAT3 s, DirectX::XMFLOAT3 e)
    {
        DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&s);
        DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&e);
        DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
        return SE;
    }
    //ノーマライズされたベクトルを作る
    static DirectX::XMVECTOR CreateNormalVector(DirectX::XMFLOAT3 s, DirectX::XMFLOAT3 e)
    {
        DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&s);
        DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&e);
        DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
        SE = DirectX::XMVector3Normalize(SE);
        return SE;
    }
    //見ている方向ベクトルをさらにロール・ピッチ・ヨーに変換する関数
    static DirectX::XMFLOAT3 GetLookAtAngles(const DirectX::XMFLOAT3 from, const DirectX::XMFLOAT3 to)
    {
        using namespace DirectX;
        XMFLOAT3 dirFloat;
        DirectX::XMStoreFloat3(&dirFloat, CreateNormalVector(from, to));

        // ピッチ（X軸回転）：Y成分の角度
        float pitch = asinf(-dirFloat.y);  // 上下方向

        // ヨー（Y軸回転）：XZ平面での向き
        float yaw = atan2f(dirFloat.x, dirFloat.z);  // 左右方向

        // ロール（Z軸回転）：通常は 0
        float roll = 0.0f;

        return XMFLOAT3(pitch, yaw, roll);
    }

    //行列作成
    static DirectX::XMMATRIX CreateXMMATRIX(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& s, const DirectX::XMFLOAT3& a)
    {
        //スケール行列制作
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(s.x, s.y, s.z);

        //回転行列を制作
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(a.x, a.y, a.z);

        //位置行列を作成
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(p.x, p.y, p.z);

        //三つの行列を組み合わせてワールド行列を作成
        return S * R * T;
    }

    static DirectX::XMFLOAT4X4 WorldTransformConvert(const DirectX::XMFLOAT4X4& parentTransform, const DirectX::XMFLOAT4X4& childTransform)
    {
        using namespace DirectX;

        XMMATRIX P = XMLoadFloat4x4(&parentTransform);
        XMMATRIX C = XMLoadFloat4x4(&childTransform);
        XMMATRIX W = XMMatrixMultiply(C, P);

        XMFLOAT4X4 w;
        XMStoreFloat4x4(&w, W);
        return w;
    }

    static DirectX::XMFLOAT3 ReadTransFormPosition(const DirectX::XMFLOAT4X4& t)
    {
        return { t._41,t._42,t._43 };
    }

    static void ReadTransFormAll(const DirectX::XMFLOAT4X4& t, DirectX::XMFLOAT3& p, DirectX::XMFLOAT3& s, DirectX::XMFLOAT3& a)
    {

    }
    static void ReadTransFormAll(const DirectX::XMFLOAT4X4& t, DirectX::XMFLOAT3& p, DirectX::XMFLOAT3& s, DirectX::XMFLOAT4& r)
    {
        DirectX::XMVECTOR P{}, S{}, R{};
        DirectX::XMMATRIX W{};
        DirectX::XMMatrixDecompose(&S, &R, &P, W);

        DirectX::XMStoreFloat3(&p, P);
        DirectX::XMStoreFloat3(&s, S);
        DirectX::XMStoreFloat4(&r, R);
    }

    static inline float Dot(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
    {
        return { (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) };
    }

    static inline DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
    {
        DirectX::XMFLOAT3 cross = {};

        cross.x = v1.y * v2.z - v1.z * v2.y;
        cross.y = v1.z * v2.x - v1.x * v2.z;
        cross.z = v1.x * v2.y - v1.y * v2.x;

        return cross;
    }

    static inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v1)
    {
        DirectX::XMFLOAT3 n;
        float L = sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

        n.x = v1.x / L;
        n.y = v1.y / L;
        n.z = v1.z / L;

        return n;
    }

    static inline float GetLength(const DirectX::XMFLOAT3& v1)
    {
        return sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    }

    static inline DirectX::XMFLOAT3 GetForward(const DirectX::XMFLOAT4X4& world)
    {
        using namespace DirectX;

        XMFLOAT3 forward =
        {
            world._31,
            world._32,
            world._33
        };

        XMStoreFloat3(
            &forward,
            XMVector3Normalize(XMLoadFloat3(&forward))
        );

        return forward;
    }
};

static DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return{ v1.x - v2.x,v1.y - v2.y,v1.z - v2.z };
}

static DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return{ v1.x + v2.x,v1.y + v2.y,v1.z + v2.z };
}

static DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return{ v1.x * v2.x,v1.y * v2.y,v1.z * v2.z };
}

static DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& v1, const float& v2)
{
    return{ v1.x * v2,v1.y * v2,v1.z * v2 };
}

static DirectX::XMFLOAT3 operator*(const float& v1, const DirectX::XMFLOAT3& v2)
{
    return v2 * v1;
}

