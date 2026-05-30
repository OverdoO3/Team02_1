#pragma once
#include <string>
#include <unordered_map>
#include <array>

//ここから↓触らないで！
// enumは残す
enum class ComponentID
{
    Transform,
    ModelRender,
    Camera,
    SpriteRender,
    Move,
    BoxCollider,
    GroundCheckComponent,
    CameraController,
    ThermalBody,
    HeatTransfer,
    wood,
    HeatReceiver,
    EffectRender,
    StateEffect,
    water,
    snowman,
    PlayerUIRotator,
    ButtonComponent,
    Slope,
    tent,
    flower,
    Bridge,
    needles,
    Coin,
    playerModelChanger,
    CoinUIComponent,
    StageButtonComponent,
    Audio,
    firewood,
    justDance,
    breath,
    COUNT
};

// 変換（残す）
class ComponentRegistry
{
public:
    static ComponentID StringToID(const std::string& name);
    static const char* IDToString(ComponentID id);
};

// IDマクロ（残す）
#define COMPONENT_ID(name) \
    static constexpr ComponentID StaticID = ComponentID::name; \
    ComponentID GetID() const override { return StaticID; }