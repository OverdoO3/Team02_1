#include "Button.h"
#include "Factory.h"
#include "System/Input.h"
#include "System/GamePad.h"
#include "Transform.h"

//								↓に名前入れる
REGISTER_COMPONENT(ComponentID::Button, Button)

void Button::OnAwake(float elapsedTime)
{
}

void Button::Update(float elapsedTime)
{
    //auto spr = owner->GetComponent<SpriteRender>();
    //if (!spr) return;

    //if (IsMouseOver())
    //{
    //    spr->SetColor(DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));

    //    MouseButton downFlags = Input::Instance().GetMouse().GetButtonDown();
    //    if ((downFlags & Mouse::BTN_LEFT) != 0)
    //    {
    //        OnClick();
    //    }
    //}
    //else
    //{
    //    // 重なっていないときは「白（元の色）」に戻す
    //    spr->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
    //}
}


void Button::DrawInspector()
{
}

void Button::Serialize(nlohmann::json& j) const
{
    //j["NextSceneName"] = m_nextSceneName;
}

void Button::Deserialize(nlohmann::json& j)
{
    //if (j.contains("NextSceneName"))
    //{
    //    m_nextSceneName = j["NextSceneName"];
    //}
}

std::unique_ptr<Component> Button::Clone() const
{
	return std::unique_ptr<Button>();
}

bool Button::IsMouseOver()
{
    //if (!owner) return false;
    //auto tran = owner->GetComponent<Transform>();
    //auto spr = owner->GetComponent<SpriteRender>();
    //if (!tran || !spr) return false;

    //auto& mouse = Input::Instance().GetMouse();
    //float mouseX = static_cast<float>(mouse.GetPositionX());
    //float mouseY = static_cast<float>(mouse.GetPositionY());

    //auto pos = tran->GetWorldPosition();
    //float texW = spr->GetSprite() ? spr->GetSprite()->GetTextureWidth() : 0.0f;
    //float texH = spr->GetSprite() ? spr->GetSprite()->GetTextureHeight() : 0.0f;
    //float srcW = (spr->GetSrcW() < 0.0f) ? texW : spr->GetSrcW();
    //float srcH = (spr->GetSrcH() < 0.0f) ? texH : spr->GetSrcH();
    //float width = srcW * spr->GetEditorScale();
    //float height = srcH * spr->GetEditorScale();

    //// 左上原点のスクリーン座標で判定
    //float left = pos.x;
    //float top = pos.y;
    //float right = pos.x + width;
    //float bottom = pos.y + height;

    //if (mouseX >= left && mouseX <= right &&
    //    mouseY >= top && mouseY <= bottom)
    //{
    //    return true;
    //}
    return false;
}

void Button::OnClick()
{
    //if (!owner) return;
    //auto spr = owner->GetComponent<SpriteRender>();
    //if (spr)
    //{
    //    spr->SetColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
    //}
}