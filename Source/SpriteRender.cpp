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

        float sx = m_srcX;
        float sy = m_srcY;
        float sw = (m_srcW < 0.0f) ? texW : m_srcW;
        float sh = (m_srcH < 0.0f) ? texH : m_srcH;


        spr->Render(
            rc,
            pos.x, pos.y, pos.z,
            width, height,
            sx, sy,
            sw, sh,
            m_editorAngleDeg,
            color.x,color.y,color.z,color.w
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
    j["SrcX"] = m_srcX;
    j["SrcY"] = m_srcY;
    j["SrcW"] = m_srcW;
    j["SrcH"] = m_srcH;
    j["ColorR"] = color.x;
    j["ColorG"] = color.y;
    j["ColorB"] = color.z;
    j["ColorA"] = color.w;
    j["SortOrder"] = sortOrder;
    j["SplitX"] = m_splitX;
    j["SplitY"] = m_splitY;
    j["SpriteIndex"] = m_spriteIndex;
    j["TargetCol"] = m_targetCol;
    j["TargetRow"] = m_targetRow;
}

void SpriteRender::DrawInspector()
{
    ImGui::Checkbox("enabled", &enabled);

    auto tran = owner->GetComponent<Transform>();
    if (!tran) return;

    // 位置調整（これはTransformをいじるべき）
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
    ImGui::Separator();
    ImGui::Text("Sprite Sheet Splitter");

    //分割数
    ImGui::InputInt("Columns (Horizontal)", &m_splitX);
    ImGui::InputInt("Rows (Horizontal)", &m_splitY);

    //移したい場所を
    ImGui::InputInt("Select Column (X)", &m_targetCol);
    ImGui::InputInt("Select Row (Y)", &m_targetRow);


    ImGui::InputInt("Sprite Index", &m_spriteIndex);

    //0以下防止
    if (m_splitX < 1) m_splitX = 1;
    if (m_splitY < 1) m_splitX = 1;
    if (m_spriteIndex < 0)m_spriteIndex = 0;

    m_targetCol = std::clamp(m_targetCol, 0, m_splitX - 1);
    m_targetRow = std::clamp(m_targetRow, 0, m_splitY - 1);

    if (spr && ImGui::Button("Update Crop Area")) {
        float texW = spr->GetTextureWidth();
        float texH = spr->GetTextureHeight();

        // 1枚あたりのサイズ (Width, Height)
        m_srcW = texW / (float)m_splitX;
        m_srcH = texH / (float)m_splitY;

        // 指定された場所の座標 (Left, Top)
        m_srcX = (float)m_targetCol * m_srcW;
        m_srcY = (float)m_targetRow * m_srcH;
    }


    ImGui::Separator();
    ImGui::Text("UV Crop");
    ImGui::DragFloat("Src X", &m_srcX, 1.0f, 0.0f, 4096.0f);
    ImGui::DragFloat("Src Y", &m_srcY, 1.0f, 0.0f, 4096.0f);
    ImGui::DragFloat("Src W", &m_srcW, 1.0f, -1.0f, 4096.0f);
    ImGui::DragFloat("Src H", &m_srcH, 1.0f, -1.0f, 4096.0f);
    ImGui::Text("(-1 = full texture)");

    ImGui::Separator();
    ImGui::Text("Color");
    ImGui::ColorEdit4("Color", &color.x);

    ImGui::Separator();
    ImGui::InputInt("Sort Order", &sortOrder);


}



void SpriteRender::Deserialize(nlohmann::json& j)
{
	texturepath = j["TexturePath"];
    m_srcX = j.value("SrcX", 0.0f);
    m_srcY = j.value("SrcY", 0.0f);
    m_srcW = j.value("SrcW", -1.0f);
    m_srcH = j.value("SrcH", -1.0f);
    color.x = j.value("ColorR", 1.0f);
    color.y = j.value("ColorG", 1.0f);
    color.z = j.value("ColorB", 1.0f);
    color.w = j.value("ColorA", 1.0f);
    sortOrder = j.value("SortOrder", 0);
    m_splitX = j.value("SplitX", 1);
    m_splitY = j.value("SplitY", 1);
    m_targetCol = j.value("TargetCol", 0);
    m_targetRow = j.value("TargetRow", 0);


    m_spriteIndex = j.value("SpriteIndex", 0);

	if (texturepath != "")
	{
		spr = std::make_unique<Sprite>(texturepath.c_str());
	}
}
