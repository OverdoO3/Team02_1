#pragma once

#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "RenderContext.h"

	struct Mesh
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer>	vertexBuffer;
		UINT									vertexCount;
	};

	struct point_lights
	{
		DirectX::XMFLOAT4 position{ 0,0,0,0 };
		DirectX::XMFLOAT4 color{ 1,1,1,1 };
		float range{ 0 };
		DirectX::XMFLOAT3 dummy;
	};

	struct LightConstants {
		DirectX::XMFLOAT4X4 lightViewProjection; // ライト視点の行列
		DirectX::XMFLOAT4   lightDirection;      // ライトの向き
		DirectX::XMFLOAT4   lightColor;          // ライトの色
	};

	struct CbMesh
	{
		DirectX::XMFLOAT4X4		worldViewProjection;
		DirectX::XMFLOAT4		color;

		DirectX::XMFLOAT4 ka;	//環境光係数
		DirectX::XMFLOAT4 kd;	//拡散反射係数(今までのcolor)
		DirectX::XMFLOAT4 ks;	//鏡面反射係数

		float shadow_color;
		float shadow_bias;
		DirectX::XMFLOAT2 padding;
	};

	struct CbScene
	{
		DirectX::XMFLOAT4X4 viewProjection;

		DirectX::XMFLOAT4 cameraPosition;
	};


class ShapeRenderer
{
public:
	ShapeRenderer(ID3D11Device* device);
	~ShapeRenderer() {}

	// 箱描画
	void RenderBox(
		const RenderContext& rc,
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& angle,
		const DirectX::XMFLOAT3& size,
		const DirectX::XMFLOAT4& color) const;

	// 球描画
	void RenderSphere(
		const RenderContext& rc,
		const DirectX::XMFLOAT3& position,
		float radius,
		const DirectX::XMFLOAT4& color) const;

	// 円柱描画
	void RenderCylinder(
		const RenderContext& rc,
		const DirectX::XMFLOAT3& position,
		float radius,
		float height,
		const DirectX::XMFLOAT4& color) const;

	// カプセル描画
	void RenderCapsule(
		const RenderContext& rc,
		const DirectX::XMFLOAT4X4& transform,
		float radius,
		float height,
		const DirectX::XMFLOAT4& color) const;

	void DrawLine(
		const RenderContext& rc,
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		const DirectX::XMFLOAT4& color)const;

	// 定数バッファ自体を取得
	ID3D11Buffer* GetLightConstantBuffer() const { return lightConstantBuffer.Get(); }

	//ポイント配列のゲッター
	point_lights* GetPointLights() { return point_light; }


private:
	// 描画実行
	void Render(const RenderContext& rc, const Mesh& mesh, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT4& color) const;

	// メッシュ生成
	void CreateMesh(ID3D11Device* device, const std::vector<DirectX::XMFLOAT3>& vertices, Mesh& mesh);

	// 箱メッシュ作成
	void CreateBoxMesh(ID3D11Device* device, float width, float height, float depth);

	// 球メッシュ作成
	void CreateSphereMesh(ID3D11Device* device, float radius, int subdivisions);

	// 半球メッシュ作成
	void CreateHalfSphereMesh(ID3D11Device* device, float radius, int subdivisions);

	// 円柱
	void CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int subdivisions);
	
	//線メッシュ作成
	void CreateLineMesh(ID3D11Device* device);
private:
	Mesh										boxMesh;
	Mesh										sphereMesh;
	Mesh										halfSphereMesh;
	Mesh										cylinderMesh;
	Mesh										lineMesh;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	    sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		lightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		meshDataConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		shadowParamsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		light_constant_buffer;

	DirectX::XMFLOAT4 ambient_color{ 0.2f,0.2f,0.2f,0.2f };
	DirectX::XMFLOAT4 directional_light_direction{ 0.0f,-1.0f,1.0f,1.0f };
	DirectX::XMFLOAT4 directional_light_color{ 1.0f,1.0f,1.0f,1.0f };
	point_lights point_light[8];
};
