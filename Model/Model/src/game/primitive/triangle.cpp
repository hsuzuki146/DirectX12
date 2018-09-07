#include "triangle.h"
#include "../../common/d3dx12manager.h"

#include <DirectXMath.h>
using namespace DirectX;
struct Vertex3D 
{
	XMFLOAT3 Position;	//�ʒu
	XMFLOAT3 Normal;	//�@��
	XMFLOAT4 Color;	//���_�J���[
};

Triangle::Triangle()
{
}

Triangle::~Triangle()
{
}

bool Triangle::Initialize()
{
	// �����������ݒ�.
	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type					= D3D12_HEAP_TYPE_UPLOAD;		// GPU�ւ̓]���ɍœK��.
	heap_properties.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// CPU���œǂݏ���.
	heap_properties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;	// �������̎����͂��C��.
	heap_properties.CreationNodeMask		= 0;
	heap_properties.VisibleNodeMask			= 0;

	// �o�b�t�@�ݒ�.
	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;	//�p�r(�o�b�t�@�̉�).
	resource_desc.Width					= 256;
	resource_desc.Height				= 1;
	resource_desc.DepthOrArraySize		= 1;
	resource_desc.MipLevels				= 1;
	resource_desc.Format				= DXGI_FORMAT_UNKNOWN;
	resource_desc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resource_desc.SampleDesc.Count		= 1;
	resource_desc.SampleDesc.Quality	= 0;

	// ���_�o�b�t�@�̍쐬.
	HRESULT hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �C���f�b�N�X�o�b�t�@�̍쐬.
	hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&index_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �萔�o�b�t�@�̍쐬.
	hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constant_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// ���_�f�[�^�̏�������.
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

	// �C���f�b�N�X�f�[�^�̏�������.

	
	buffer = nullptr;


	return true;
}

void Triangle::Draw()
{
	// �J�����̐ݒ�.
	const XMMATRIX view = XMMatrixLookAtLH({ 0.0f,0.0f,-5.0f }, { 0.0f,0.0f,0.0f }, {0.0f,1.0f,0.0f});
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f), 
		static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx) / static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy),
		1.0f, 
		20.0f );

	// �r���[�v���W�F�N�V�����s��.
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixTranspose(view * projection));

	XMFLOAT4X4* buffer = {};
	HRESULT hr = constant_buffer_->Map(0, nullptr, (void**)&buffer);
	if (FAILED(hr))
	{
		ASSERT(false);
		return;
	}
	// �s���萔�o�b�t�@�ɏ�������.
	*buffer = Mat;
	constant_buffer_->Unmap(0, nullptr);
	buffer = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
	vertex_view.BufferLocation		= vertex_buffer_->GetGPUVirtualAddress();
	vertex_view.StrideInBytes		= sizeof(Vertex3D);
	vertex_view.SizeInBytes			= sizeof(Vertex3D) * 3;

	// �萔�o�b�t�@���V�F�[�_�̃��W�X�^�ɃZ�b�g.
	D3D_COMMAND_LIST()->SetGraphicsRootConstantBufferView( 0, constant_buffer_->GetGPUVirtualAddress());

	// �C���f�b�N�X���g�p���Ȃ��g���C�A���O���X�g���b�v�ŕ`��.
	D3D_COMMAND_LIST()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D_COMMAND_LIST()->IASetVertexBuffers(0, 1, &vertex_view);

	// �`��.
	D3D_COMMAND_LIST()->DrawInstanced(3, 1, 0, 0);
}
