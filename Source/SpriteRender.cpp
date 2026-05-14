#include "SpriteRender.h"
#include "Factory.h"
#include "Actor.h"

REGISTER_COMPONENT(ComponentID::SpriteRender,SpriteRender)

void SpriteRender::Draw(RenderContext& rc)
{
    auto tran = owner->GetComponent<Transform>();
    if (spr && tran)
    {
        auto pos = tran->GetWorldPosition();

        float texW = spr->GetTextureWidth();
        float texH = spr->GetTextureHeight();

        float width = texW * m_editorScale;
        float height = texH * m_editorScale;

        spr->Render(
            rc,
            pos.x, pos.y, pos.z,
            width, height,
            m_editorAngleDeg,      
            1.0f, 1.0f, 1.0f, 1.0f
        );
    }
}


void SpriteRender::Update(float elapsedTime)
{
	
}

std::unique_ptr<Component> SpriteRender::Clone() const
{
	auto c = std::make_unique<SpriteRender>();

	c->SetSprite(std::make_unique<Sprite>(texturepath.c_str()));
	if (texturepath !="")
	{ 
		c->SetString(texturepath.c_str());
	}


	return c;
}

void SpriteRender::Serialize(nlohmann::json& j) const
{
	j["TexturePath"] = texturepath;
}

void SpriteRender::DrawInspector()
{
    ImGui::Checkbox("enabled", &enabled);

    auto tran = owner->GetComponent<Transform>();
    if (!tran) return;

    // ˆÊ’u’²®i‚±‚ê‚ÍTransform‚ð‚¢‚¶‚é‚×‚«j
    DirectX::XMFLOAT3 pos = tran->GetLocalPosition();
    if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
        tran->SetLocalPosition(pos);
    }

    ImGui::DragFloat("UI Rotation Z", &m_editorAngleDeg, 1.0f);

    ImGui::DragFloat("UI Scale", &m_editorScale, 0.01f, 0.001f, 100.0f);

    ImGui::Separator();

    if (!texturepath.empty()) {
        std::string filename = std::filesystem::path(texturepath).filename().string();
        ImGui::Text("File: %s", filename.c_str());
    }

    if (ImGui::Button("Select Sprite")) {
        std::string full = OpenDialog::OpenLoadFileDialog();
        if (!full.empty()) {
            texturepath = ToDataPath(full);
            spr = std::make_unique<Sprite>(texturepath.c_str());
        }
    }
}



void SpriteRender::Deserialize(nlohmann::json& j)
{
	texturepath = j["TexturePath"];

	if (texturepath != "")
	{
		spr = std::make_unique<Sprite>(texturepath.c_str());
	}
}
