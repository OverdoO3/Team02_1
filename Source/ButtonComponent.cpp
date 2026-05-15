#include "ButtonComponent.h"
#include "Factory.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::ButtonComponent, ButtonComponent)


void ButtonComponent::Update(float elapsedTime)
{
    // マウスがボタンの上にあるか確認
    bool hovering = IsMouseOver();

    auto spr = owner->GetComponent<SpriteRender>();
    if (spr)
    {
        if (hovering)
        {
            // ホバー中：少し明るくするか、特定の色にする（例：黄色っぽく）
            spr->SetColor(DirectX::XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f));

        }
        else
        {
            spr->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
}

void ButtonComponent::DrawInspector()
{
}

void ButtonComponent::Serialize(nlohmann::json& j) const

{
}

void ButtonComponent::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> ButtonComponent::Clone() const
{
    auto c = std::make_unique<ButtonComponent>();

    return c;
}

bool ButtonComponent::IsMouseOver()
{
    if (!owner) return false;
    auto tran = owner->GetComponent<Transform>();
    auto spr = owner->GetComponent<SpriteRender>();
    if (!tran || !spr) return false;

    auto& mouse = Input::Instance().GetMouse();
    float mouseX = static_cast<float>(mouse.GetPositionX());
    float mouseY = static_cast<float>(mouse.GetPositionY());

    auto pos = tran->GetWorldPosition();
    float texW = spr->GetSprite() ? spr->GetSprite()->GetTextureWidth() : 0.0f;
    float texH = spr->GetSprite() ? spr->GetSprite()->GetTextureHeight() : 0.0f;
    float srcW = (spr->GetSrcW() < 0.0f) ? texW : spr->GetSrcW();
    float srcH = (spr->GetSrcH() < 0.0f) ? texH : spr->GetSrcH();
    float width = srcW * spr->GetEditorScale();
    float height = srcH * spr->GetEditorScale();

    // 左上原点のスクリーン座標で判定
    float left = pos.x;
    float top = pos.y;
    float right = pos.x + width;
    float bottom = pos.y + height;

    if (mouseX >= left && mouseX <= right &&
        mouseY >= top && mouseY <= bottom)
    {
        return true;
    }
    return false;
}

void ButtonComponent::OnClick()
{
    if (!owner) return;
    auto spr = owner->GetComponent<SpriteRender>();
    if (spr)
    {
        spr->SetColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
    }
}