#include "triangle.h"
#include "../../common/d3dx12manager.h"

#include <DirectXMath.h>
using namespace DirectX;
struct Vertex3D 
{
	XMFLOAT3 Position;	//位置
	XMFLOAT3 Normal;	//法線
	XMFLOAT4 Color;	//頂点カラー
};

Triangle::Triangle()
{
}

Triangle::~Triangle()
{
}

bool Triangle::Initialize()
{
	// 物理メモリ設定.
	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type					= D3D12_HEAP_TYPE_UPLOAD;		// GPUへの転送に最適化.
	heap_properties.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// CPU側で読み書き.
	heap_properties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;	// メモリの取り方はお任せ.
	heap_properties.CreationNodeMask		= 0;
	heap_properties.VisibleNodeMask			= 0;

	// バッファ設定.
	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;	//用途(バッファの塊).
	resource_desc.Width					= 256;
	resource_desc.Height				= 1;
	resource_desc.DepthOrArraySize		= 1;
	resource_desc.MipLevels				= 1;
	resource_desc.Format				= DXGI_FORMAT_UNKNOWN;
	resource_desc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resource_desc.SampleDesc.Count		= 1;
	resource_desc.SampleDesc.Quality	= 0;

	// 頂点バッファの作成.
	HRESULT hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// 定数バッファの作成.
	hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constant_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// 頂点データの書き込み.
	Vertex3D* buffer = {};
	hr = vertex_buffer_->Map(0, nullptr, (void**)&buffer);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	buffer[0].Position = {  0.0f,  1.0f, 0.0f };
	buffer[1].Position = {  1.0f, -1.0f, 0.0f };
	buffer[2].Position = { -1.0f, -1.0f, 0.0f };
	buffer[0].Normal = { 0.0f, 0.0f, -1.0f };
	buffer[1].Normal = { 0.0f, 0.0f, -1.0f };
	buffer[2].Normal = { 0.0f, 0.0f, -1.0f };
	buffer[0].Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	buffer[1].Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	buffer[2].Color = { 0.0f, 0.0f, 1.0f, 1.0f };

	vertex_buffer_->Unmap(0, nullptr);
	buffer = nullptr;


	return true;
}

void Triangle::Draw()
{
	// カメラの設定.
	const XMMATRIX view = XMMatrixLookAtLH({ 0.0f,0.0f,-5.0f }, { 0.0f,0.0f,0.0f }, {0.0f,1.0f,0.0f});
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f), 
		static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx) / static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy),
		1.0f, 
		20.0f );

	// ビュープロジェクション行列.
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixTranspose(view * projection));

	XMFLOAT4X4* buffer = {};
	HRESULT hr = constant_buffer_->Map(0, nullptr, (void**)&buffer);
	if (FAILED(hr))
	{
		ASSERT(false);
		return;
	}
	// 行列を定数バッファに書き込み.
	*buffer = Mat;
	constant_buffer_->Unmap(0, nullptr);
	buffer = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
	vertex_view.BufferLocation		= vertex_buffer_->GetGPUVirtualAddress();
	vertex_view.StrideInBytes		= sizeof(Vertex3D);
	vertex_view.SizeInBytes			= sizeof(Vertex3D) * 3;

	// 定数バッファをシェーダのレジスタにセット.
	D3D_COMMAND_LIST()->SetGraphicsRootConstantBufferView( 0, constant_buffer_->GetGPUVirtualAddress());

	// インデックスを使用しないトライアングルストリップで描画.
	D3D_COMMAND_LIST()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D_COMMAND_LIST()->IASetVertexBuffers(0, 1, &vertex_view);

	// 描画.
	D3D_COMMAND_LIST()->DrawInstanced(3, 1, 0, 0);
}
