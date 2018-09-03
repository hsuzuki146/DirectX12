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
	// リソースバリア.
	void setResourceBarrier(D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state);
	// 実行したコマンドの終了待ち.
	void waitForPreviousFrame();

private:
	// 生成するレンダーターゲットビューの数.
	static const int RTV_NUM = 2;

private:
	ComPtr<IDXGIFactory4>		factory_;		// DXGIオブジェクト生成インタフェース.
	ComPtr<IDXGISwapChain4>		swap_chain_;	// バックバッファとフロントバッファの入れ替え.

	ComPtr<ID3D12Device>		device_;		// D3Dデバイス.
	
	// コマンド系.
	ComPtr<ID3D12CommandAllocator>		command_allocator_;			// コマンドアロケータ(描画コマンドをためておく場所).
	ComPtr<ID3D12GraphicsCommandList>	command_list_;				// コマンドリスト.
	ComPtr<ID3D12CommandQueue>			command_queue_;				// ドローコールを積み込むQueue.
	
	// レンダーターゲット.
	ComPtr<ID3D12Resource>				render_target_[RTV_NUM];	// レンダーターゲット.
	ComPtr<ID3D12DescriptorHeap>		dh_rtv_;					// レンダーターゲット用ヒープ.
	D3D12_CPU_DESCRIPTOR_HANDLE			rtv_handle_[RTV_NUM];		// レンダーターゲットのハンドル.
	UInt32								buffer_index_ = { 0 };		// カレントバッファのインデックス.

	// 深度バッファ.
	ComPtr<ID3D12Resource>				depth_buffer_;				// 深度バッファ.
	ComPtr<ID3D12DescriptorHeap>		dh_dsv_;					// 深度バッファ用ヒープ.
	D3D12_CPU_DESCRIPTOR_HANDLE			dsv_handle_;				// 深度バッファハンドル.

	// 描画コマンド終了待ち.
	HANDLE								fence_event_ = {};			// 終了イベント用ハンドル.
	ComPtr<ID3D12Fence>					queue_fence_;				// 積んだコマンドが終了待ちを行う.
	UInt64								frames_ = { 0 };

};

#define D3D_MGR() (D3DX12Manager::GetInstance())