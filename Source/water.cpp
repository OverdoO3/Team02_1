#include "water.h"
#include "Actor.h"
#include "Factory.h"
#include "StateEffect.h"
#include <EffectRender.h>
#include "System/Audio.h"

//								Å´Ç…ñºëOì¸ÇÍÇÈ
REGISTER_COMPONENT(ComponentID::water, water)

void water::OnAwake(float elapsedTime)
{
	auto eff = owner->GetComponent<EffectRender>();
	if (!eff)return;

	eff->SetEffect(EffectManager::Instance().LoadEffect("./Data/Effects/ice_box.efk"));
}

void water::Update(float elapsedTime)
{
    auto temp = owner->GetComponent<ThermalBody>();
    if (!temp) return;

    // 1. Ç‹Ç∏îMÇéÊìæ
    int heat = temp->GetHeat();

    // 2. ïœâªÇ™Ç»ÇØÇÍÇŒÇ±Ç±Ç≈èIóπÅiÇ±ÇÍÇ≈ÉãÅ[ÉvÇñhé~Åj
    if (heat == currentTemp) return;

    // 3. ïœâªÇ™Ç†Ç¡ÇΩÇÃÇ≈ÅAÇ‹Ç∏ÇÕílÇämíËÇ≥ÇπÇÈ
    currentTemp = heat;

    // 4. ïœâªÇµÇΩéûÇæÇØÇÃèàóù
    auto modelrender = owner->GetComponent<ModelRender>();
    auto eff = owner->GetComponent<EffectRender>();
    auto col = owner->GetComponent<BoxCollider>();
    auto rec = owner->GetComponent<HeatReceiver>();

    if (modelrender && eff && col && rec)
    {
        switch (heat)
        {
        case -2: // ïXÇ…Ç»Ç¡ÇΩèuä‘
        {
            auto model = std::make_unique<Model>(icepath.c_str());
            col->size.y = 10;
            modelrender->SetModel(std::move(model));
            eff->Play();
            rec->SetHeatNum(-2);
            // ämé¿Ç…1âÒÇæÇØçƒê∂Ç≥ÇÍÇÈ
            Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_game_reaction_ice.wav").c_str());
        }
        break;
        case 1: // êÖÇ…ñﬂÇ¡ÇΩèuä‘
        {
            auto model = std::make_unique<Model>(waterpath.c_str());
            col->size.y = 50;
            modelrender->SetModel(std::move(model));
            rec->SetHeatNum(-1);
            eff->Stop();
            Audio::Instance().PlaySE(ToDataPath("Data/Sound/SE_game_reaction_water.wav").c_str());
        }
        break;
        }
    }

    auto effectstate = owner->GetComponent<StateEffect>();
    if (effectstate)
    {
        if (heat == -2) {
            effectstate->enabled = true;
            effectstate->SetState("ice");
        }
        else if (heat == 1) {
            effectstate->enabled = false;
        }
    }
}


void water::DrawInspector()
{
}

void water::Serialize(nlohmann::json& j) const
{
}

void water::Deserialize(nlohmann::json& j)
{
}

std::unique_ptr<Component> water::Clone() const
{
	auto h = std::make_unique<water>();
	h->currentTemp = this->currentTemp; 
	return h;
}
