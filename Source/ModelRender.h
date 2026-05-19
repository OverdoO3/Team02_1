#pragma once
#include "System/Model.h"
#include "Component.h"
#include <nlohmann/json.hpp>
#include "OpenDialog.h"
using json = nlohmann::json;

class ModelRender : public Component
{
public:
	COMPONENT_ID(ModelRender)
	ModelRender() {};
	~ModelRender() {};

	void Render(RenderContext& rc, ModelRenderer* renderer) override;
	void Update(float elasedTime) override;

	void DrawInspector() override;
	Model* GetModel() 
	{
		if (model != nullptr)
		{
			return model.get();
		}
		return nullptr;
	}

	void SetModel(std::unique_ptr<Model> mdl)
	{
 		model = std::move(mdl);
	}

	void SetString(std::string str)
	{
		modelpath = str;
	}

	std::string ToDataPath(const std::string& fullPath)
	{
		std::filesystem::path base = std::filesystem::absolute("Data");
		std::filesystem::path target = std::filesystem::absolute(fullPath);

		std::filesystem::path relative = std::filesystem::relative(target, base);

		std::filesystem::path normalized = relative.lexically_normal();

		return "Data/" + normalized.generic_string();
	}

	void PlayAnimation(int index, bool loop);
	void PlayAnimation(const char* name, bool loop);
	void UpdateAnimation(float elapsedTime);

	static void Flush(const RenderContext& rc, ModelRenderer* renderer);

	std::unique_ptr<Component> Clone() const override;

	void Serialize(nlohmann::json& j)const override;
	void Deserialize(nlohmann::json& j) override;

	void SetShaderId(ShaderId id) { shaderId = id; }
	ShaderId GetShaderId() const { return shaderId; }
private:
	std::unique_ptr<Model> model;
	std::string modelpath;
	ShaderId shaderId = ShaderId::Lambert;

	int	animationIndex = -1;
	float animationSeconds = 0.0f;
	bool animationLoop = false;
	bool animationPlaying = false;
	float animationBlendSecondsLength = 0.2f;
};