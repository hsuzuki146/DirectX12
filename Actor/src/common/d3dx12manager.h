#pragma once

#include "common.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl/client.h>	// ComPtr.

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "dxgi.lib")

using namespace Microsoft::WRL;

class D3DX12Manager : public Singleton<D3DX12Manager>
{
public:
	D3DX12Manager();
	~D3DX12Manager();
	bool Create();
	void Destroy();
	void Render();

private:
	bool createFactory();
	bool createDevice();
	bool createCommandQueue();
	bool createAllocator();
	bool createCommandList();
	bool createSwapChain();

	bool createRenderTarget();
	bool createDepthStencilBuffer();

private:
	// ���\�[�X�o���A.
	void setResourceBarrier(D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state);
	// ���s�����R�}���h�̏I���҂�.
	void waitForPreviousFrame();

private:
	// �������郌���_�[�^�[�Q�b�g�r���[�̐�.
	static const int RTV_NUM = 2;

private:
	ComPtr<IDXGIFactory4>		factory_;		// DXGI�I�u�W�F�N�g�����C���^�t�F�[�X.
	ComPtr<IDXGISwapChain4>		swap_chain_;	// �o�b�N�o�b�t�@�ƃt�����g�o�b�t�@�̓���ւ�.

	ComPtr<ID3D12Device>		device_;		// D3D�f�o�C�X.
	
	// �R�}���h�n.
	ComPtr<ID3D12CommandAllocator>		command_allocator_;			// �R�}���h�A���P�[�^(�`��R�}���h�����߂Ă����ꏊ).
	ComPtr<ID3D12GraphicsCommandList>	command_list_;				// �R�}���h���X�g.
	ComPtr<ID3D12CommandQueue>			command_queue_;				// �h���[�R�[����ςݍ���Queue.
	
	// �����_�[�^�[�Q�b�g.
	ComPtr<ID3D12Resource>				render_target_[RTV_NUM];	// �����_�[�^�[�Q�b�g.
	ComPtr<ID3D12DescriptorHeap>		dh_rtv_;					// �����_�[�^�[�Q�b�g�p�q�[�v.
	D3D12_CPU_DESCRIPTOR_HANDLE			rtv_handle_[RTV_NUM];		// �����_�[�^�[�Q�b�g�̃n���h��.
	UInt32								buffer_index_ = { 0 };		// �J�����g�o�b�t�@�̃C���f�b�N�X.

	// �[�x�o�b�t�@.
	ComPtr<ID3D12Resource>				depth_buffer_;				// �[�x�o�b�t�@.
	ComPtr<ID3D12DescriptorHeap>		dh_dsv_;					// �[�x�o�b�t�@�p�q�[�v.
	D3D12_CPU_DESCRIPTOR_HANDLE			dsv_handle_;				// �[�x�o�b�t�@�n���h��.

	// �`��R�}���h�I���҂�.
	HANDLE								fence_event_ = {};			// �I���C�x���g�p�n���h��.
	ComPtr<ID3D12Fence>					queue_fence_;				// �ς񂾃R�}���h���I���҂����s��.
	UInt64								frames_ = { 0 };

};

#define D3D_MGR() (D3DX12Manager::GetInstance())