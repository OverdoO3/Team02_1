#pragma once

#include "System//ModelRenderer.h"
#include "System/Sprite.h"
#include <memory>


class Stage
{
public:
	Stage();
	~Stage();

	//XV
	void Update(float elapsedTime);

	//•`‰æ
	void Render(const RenderContext& rc, ModelRenderer* renderer);

private:
	std::unique_ptr<Model>model = nullptr;
};