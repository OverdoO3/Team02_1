#include "Stage.h"

//コンストラクタ
Stage::Stage()
{
	//ステージのモデルを読み込む
	model = std::make_unique<Model>("Data/Model/Stage/ExampleStage.mdl");
}

Stage::~Stage()
{
}

//更新処理
void Stage::Update(float elapsedTime)
{

}

//描画処理
void Stage::Render(const RenderContext& rc, ModelRenderer* renderer)
{
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMStoreFloat4x4(&transform, DirectX::XMMatrixIdentity());

	//レンダラに描画させる
	renderer->Render(rc, transform, model.get(), ShaderId::Lambert);
}
