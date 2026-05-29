#include "PlayerUIController.h"
#include "Actor.h"
#include "Scene.h"
#include "SpriteRender.h"
#include "ThermalBody.h" 


REGISTER_COMPONENT(ComponentID::PlayerUIController, PlayerUIController)

void PlayerUIController::Update(float elapsedTime)
{
    if (!m_playerThermal)
    {
        auto scene = owner->GetScene();
        if (scene)
        {
            for (auto& actor : scene->actors)
            {
                if (actor->tag == 1) 
                {
                    m_playerThermal = actor->GetComponent<ThermalBody>();
                    break;
                }
            }
        }
    }

    
    if (m_playerThermal)
    {
        auto playerActor = m_playerThermal->GetOwner();
        auto heatTransfer = playerActor->GetComponent<HeatTransfer>();


        if (heatTransfer)
        {
            int targetHeat = heatTransfer->GetTargetHeat();

            if (targetHeat != m_lastHeat)
            {
                m_lastHeat = targetHeat;

                auto sprite = owner->GetComponent<SpriteRender>();
                if (sprite)
                {
                    switch (targetHeat)
                    {
                    case -2: // •X
                        sprite->SetTargetRow(2);
                        sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                        break;
                    case -1: // …
                        sprite->SetTargetRow(1);
                        sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                        break;
                    case 0:  // –³
                        sprite->SetColor({1.0f, 1.0f, 1.0f, 0.0f});

                        break;
                    case 1:  // ‰Î
                        sprite->SetTargetRow(0);
                        sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void PlayerUIController::DrawInspector()
{
    ImGui::Text("Last Heat: %d", m_lastHeat);
    if (m_playerThermal) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Player Connected");
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Player Not Found!");
    }
}

std::unique_ptr<Component> PlayerUIController::Clone() const
{
    auto c = std::make_unique<PlayerUIController>();
    c->m_lastHeat = m_lastHeat;
    return c;
}

void PlayerUIController::Serialize(nlohmann::json& j) const
{
    j["LastHeat"] = m_lastHeat;
}

void PlayerUIController::Deserialize(nlohmann::json& j)
{
    m_lastHeat = j.value("LastHeat", 999);
}