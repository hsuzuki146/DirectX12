#pragma once

#include "common.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <d3dcompiler.h>
#include <wrl/client.h>	// ComPtr.

//#include "../game/model/model.h"

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "dxgi.lib")
#pragma comment( lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;

class D3DX12Manager : public Singleton<D3DX12Manager>
{
public:
	D3DX12Manager();
	~D3DX12Manager();
	bool Create();
	void Destroy();

	ComPtr<ID3D12Device> GetDevice() { return device_; }
	ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return command_list_; }

public:
	void Begin();
	void PreRender();
	void End();
	void ExecuteCommand();
	void WaitFrame();
	void ResetCommand();
	void Present();

private:
	bool createFactory();
	bool createDevice();
	bool createCommandQueue();
	bool createAllocator();
	bool createCommandList();
	bool createSwapChain();
	bool createRenderTarget();
	bool createDepthStencilBuffer();
	bool createRootSignature();
	bool createPipelineStateObject();

	void setupViewport();
	void setupScissor();

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

	// 描画設定関連.
	ComPtr<ID3D12RootSignature>			root_signature_;			// ルートシグネチャ(定数バッファとシェーダーの関連付け).
	ComPtr<ID3D12PipelineState>			pipeline_state_;			// パイプライン設定(シェーダと描画ステート設定).

	// 描画領域.
	D3D12_RECT							scissor_rect_;
	D3D12_VIEWPORT						viewport_;

	// モデル.
	//Model								model_;
};

#define D3D_MGR() (D3DX12Manager::GetInstance())
#define D3D_DEVICE() (D3DX12Manager::GetInstance().GetDevice().Get())
#define D3D_COMMAND_LIST() (D3DX12Manager::GetInstance().GetCommandList().Get())