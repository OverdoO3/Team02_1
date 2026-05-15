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

        float sw = (m_srcW < 0.0f) ? texW : m_srcW;
        float sh = (m_srcH < 0.0f) ? texH : m_srcH;

        float width = sw * m_editorScale;
        float height = sh * m_editorScale;

        float sx = m_srcX;
        float sy = m_srcY;

        spr->Render(
            rc,
            pos.x, pos.y, pos.z,
            width, height,
            sx, sy,
            sw, sh,
            m_editorAngleDeg,
            color.x, color.y, color.z, color.w
        );
    }
}

void SpriteRender::Update(float elapsedTime)
{
    if (!m_isLoop || m_animFrameCount <= 1) return;

    m_timer += elapsedTime;

    if (m_timer >= m_frameDuration)
    {
        m_timer = 0.0f;
        m_currentFrame++;

        if (m_currentFrame >= m_animFrameCount)
        {
            m_currentFrame = 0;
        }

        if (spr)
        {
            float texW = spr->GetTextureWidth();
            float singleFrameW = texW / (float)m_splitX;

            m_srcW = singleFrameW;

            int nextCol = (m_targetCol + m_currentFrame) % m_splitX;
            m_srcX = (float)nextCol * m_srcW;
        }
    }
}


std::unique_ptr<Component> SpriteRender::Clone() const
{
	auto c = std::make_unique<SpriteRender>();

    c->texturepath = this->texturepath;
    c->m_srcX = this->m_srcX;
    c->m_srcY = this->m_srcY;
    c->m_srcW = this->m_srcW;
    c->m_srcH = this->m_srcH;
    c->m_splitX = this->m_splitX;
    c->m_splitY = this->m_splitY;
    c->m_targetCol = this->m_targetCol;
    c->m_targetRow = this->m_targetRow;
    c->m_isLoop = this->m_isLoop;
    c->m_frameDuration = this->m_frameDuration;
    c->m_animFrameCount = this->m_animFrameCount;
    c->m_editorScale = this->m_editorScale;
    c->color = this->color;

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
    j["IsLoop"] = m_isLoop;
    j["FrameDuration"] = m_frameDuration;
    j["AnimFrameCount"] = m_animFrameCount;
}

void SpriteRender::DrawInspector()
{
    ImGui::Checkbox("Enabled", &enabled);

    auto tran = owner->GetComponent<Transform>();
    if (!tran) return;

    if (ImGui::CollapsingHeader("Basic Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DirectX::XMFLOAT3 pos = tran->GetLocalPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
            tran->SetLocalPosition(pos);
        }
        ImGui::DragFloat("UI Rotation Z", &m_editorAngleDeg, 1.0f);
        ImGui::DragFloat("UI Scale", &m_editorScale, 0.01f, 0.001f, 100.0f);

        ImGui::Separator();
        ImGui::ColorEdit4("Color", &color.x);
        ImGui::InputInt("Sort Order", &sortOrder);
    }

    if (ImGui::CollapsingHeader("Sprite Resource"))
    {
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

    if (ImGui::CollapsingHeader("Sprite Sheet Splitter"))
    {
        ImGui::InputInt("Columns (Horizontal)", &m_splitX);
        ImGui::InputInt("Rows (Vertical)", &m_splitY);
        ImGui::InputInt("Select Column (X)", &m_targetCol);
        ImGui::InputInt("Select Row (Y)", &m_targetRow);
        ImGui::InputInt("Sprite Index", &m_spriteIndex);

        if (m_splitX < 1) m_splitX = 1;
        if (m_splitY < 1) m_splitY = 1; 
        if (m_spriteIndex < 0) m_spriteIndex = 0;

        m_targetCol = std::clamp(m_targetCol, 0, m_splitX - 1);
        m_targetRow = std::clamp(m_targetRow, 0, m_splitY - 1);

        if (spr && ImGui::Button("Update Crop Area")) {
            float texW = spr->GetTextureWidth();
            float texH = spr->GetTextureHeight();
            m_srcW = texW / (float)m_splitX;
            m_srcH = texH / (float)m_splitY;
            m_srcX = (float)m_targetCol * m_srcW;
            m_srcY = (float)m_targetRow * m_srcH;
        }
    }

    if (ImGui::CollapsingHeader("UV Crop"))
    {
        ImGui::DragFloat("Src X", &m_srcX, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src Y", &m_srcY, 1.0f, 0.0f, 4096.0f);
        ImGui::DragFloat("Src W", &m_srcW, 1.0f, -1.0f, 4096.0f);
        ImGui::DragFloat("Src H", &m_srcH, 1.0f, -1.0f, 4096.0f);
        ImGui::Text("(-1 = full texture)");
    }

    if (ImGui::CollapsingHeader("Animation Settings"))
    {
        ImGui::Checkbox("Loop Animation", &m_isLoop);
        ImGui::DragInt("Frame Count", &m_animFrameCount, 1, 1, m_splitX);
        ImGui::SliderFloat("Frame Duration", &m_frameDuration, 0.01f, 2.0f, "%.2f sec");

        if (ImGui::Button("Reset Animation")) {
            m_currentFrame = 0;
            m_timer = 0.0f;
        }
    }
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
    m_isLoop = j.value("IsLoop", false);
    m_frameDuration = j.value("FrameDuration", 0.1f);
    m_animFrameCount = j.value("AnimFrameCount", 1);
}
