#include "D3DX12Manager.h"


D3DX12Manager::D3DX12Manager()
{
}

D3DX12Manager::~D3DX12Manager()
{
}

bool D3DX12Manager::Create()
{
	if(!createFactory()) { return false; }
	if(!createDevice()) { return false; }

	if(!createAllocator()) { return false; }
	if(!createCommandQueue()) { return false; }
	if(!createCommandList()) { return false; }

	if(!createSwapChain()) { return false; }
	if(!createRenderTarget()) { return false; }
	if(!createDepthStencilBuffer()) { return false; }

	return true;
}

void D3DX12Manager::Destroy()
{
}

void D3DX12Manager::Render()
{
	{
		const Float32 clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

		// リソースの状態をプレゼント用からレンダーターゲット用に変更.
		setResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// 深度バッファとレンダーターゲットのクリア.
		command_list_->ClearDepthStencilView(dsv_handle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		command_list_->ClearRenderTargetView(rtv_handle_[buffer_index_], clearColor, 0, nullptr);

		// レンダーターゲットの設定.
		command_list_->OMSetRenderTargets(1, &rtv_handle_[buffer_index_], TRUE, &dsv_handle_);

		// リソースの状態をレンダーターゲット用からプレゼンyと用に変更.
		setResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		HRESULT hr = command_list_->Close();
	}

	// コマンドリストの実行.
	{
		ID3D12CommandList* const command_lists = command_list_.Get();
		command_queue_->ExecuteCommandLists(1, &command_lists);
	}

	// 実行したコマンドの終了待ち.
	{
		waitForPreviousFrame();
	}

	// 各種リセット処理.
	{
		// アロケータ.
		HRESULT hr = command_allocator_->Reset();
		if ((FAILED(hr)))
		{
			ASSERT(false);
			return;
		}

		// コマンドリスト.
		hr = command_list_->Reset(command_allocator_.Get(), nullptr);
		if ((FAILED(hr)))
		{
			ASSERT(false);
			return;
		}
	}

	// フロントバッファとバックバッファの切り替え.
	{
		HRESULT hr = swap_chain_->Present(1, 0);
		if (FAILED(hr))
		{
			ASSERT(false);
			return;
		}
	}

	// カレントのバックバッファのインデックスを取得する.
	buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();
}

bool D3DX12Manager::createFactory()
{
#if defined(_DEBUG)
	// デバッグレイヤーを有効にする.
	{
		ComPtr<ID3D12Debug> debug;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
		if (FAILED(hr))
		{
			ASSERT(false);
			return false;
		}
		debug->EnableDebugLayer();
	}
#endif	// #if defined(_DEBUG)
	UInt32 flag = {};
	HRESULT hr = CreateDXGIFactory2(flag, IID_PPV_ARGS(factory_.GetAddressOf()));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	return true;
}

bool D3DX12Manager::createDevice()
{
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device_));
	if (FAILED(hr))
	{
		ASSERT(false);
	}
	return true;
}

bool D3DX12Manager::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};

	// コマンドキューの生成.
	command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	// レンダリングに関係するコマンドを実行する為のコマンドリスト.
	command_queue_desc.Priority = 0;
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// タイムアウト処理は行わない.
	command_queue_desc.NodeMask = 0;
	HRESULT hr = device_->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(command_queue_.GetAddressOf()));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// コマンドキュー終了監視用のフェンス.
	// フェンスイベントの生成.
	fence_event_ = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (fence_event_ == nullptr)
	{
		ASSERT(false);
		return false;
	}
	// フェンスの生成.
	hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(queue_fence_.GetAddressOf()));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	return true;
}

bool D3DX12Manager::createAllocator()
{
	HRESULT hr = device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,		// レンダリング用途.
		IID_PPV_ARGS(command_allocator_.GetAddressOf()) );
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	return true;
}

bool D3DX12Manager::createCommandList()
{
	// コマンドアロケータとバインドして、コマンドリストを作成する.
	HRESULT hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	return true;
}

bool D3DX12Manager::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc = {};
	ComPtr<IDXGISwapChain> swap_chain = {};

	const SetupParam::Param& param = SetupParam::GetInstance().GetParam();

	desc.BufferDesc.Width					= param.windowSize_.cx;				// 幅.
	desc.BufferDesc.Height					= param.windowSize_.cy;				// 高さ.
	desc.OutputWindow						= Window::GetInstance().GetWnd();	// ウィンドウハンドル.
	desc.Windowed							= TRUE;								// ウィンドウ表示フラグ.
	desc.BufferUsage						= DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 用途(レンダーターゲット).
	desc.BufferCount						= RTV_NUM;							// レンダーターゲットの数.
	desc.SwapEffect							= DXGI_SWAP_EFFECT_FLIP_DISCARD;	// スワップ方法.
	desc.Flags								= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// ウィンドウモードとフルスクリーンを自動で切り替え.
	desc.BufferDesc.RefreshRate.Numerator	= 60;	// リフレッシュデータの分子.
	desc.BufferDesc.RefreshRate.Denominator = 1;	// リフレッシュデータの分母.
	desc.BufferDesc.Format					= DXGI_FORMAT_B8G8R8A8_UNORM;	// テクスチャフォーマット.
	desc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// スワップした時の走査線の順序.
	desc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;	// 特定のモニターの解像度に合わせてイメージを引き延ばす方法を示すフラグ.
	desc.SampleDesc.Count					= 1;	// ピクセル単位のサンプリング数.
	desc.SampleDesc.Quality					= 0;	// イメージの品質レベル.

	// スワップチェインの生成.
	HRESULT hr = factory_->CreateSwapChain(command_queue_.Get(), &desc, swap_chain.GetAddressOf());
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	// スワップチェインの設定をサポートしているか.
	hr = swap_chain->QueryInterface(swap_chain_.GetAddressOf());
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// カレントのバックバッファのインデックスを取得する.
	buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();

	return true;
}

// レンダーターゲットの作成.
bool D3DX12Manager::createRenderTarget()
{
	ASSERT(swap_chain_);

	// レンダーターゲット用のヒープ作成.
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc	= {};
	heap_desc.NumDescriptors				= RTV_NUM;
	heap_desc.Type							= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// 用途(レンダーターゲット).
	heap_desc.Flags							= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// フラグ.
	heap_desc.NodeMask						= 0;
	HRESULT hr = device_->CreateDescriptorHeap( &heap_desc, IID_PPV_ARGS(dh_rtv_.GetAddressOf()) );
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// レンダーターゲットの作成処理.
	const UInt32 size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UInt32 i = 0; i < RTV_NUM; ++i)
	{
		// スワップチェインからバッファを受け取る.
		HRESULT hr = swap_chain_->GetBuffer(i, IID_PPV_ARGS(&render_target_[i]));
		if (FAILED(hr))
		{
			ASSERT(false);
			return false;
		}

		// RenderTargetViewを作成してヒープでスクリプタに登録.
		// DescriptorHeapの開始位置のハンドルを取得.
		rtv_handle_[i] = dh_rtv_->GetCPUDescriptorHandleForHeapStart();
		// そこからのオフセット.
		rtv_handle_[i].ptr += (size * i);
		// レンダーターゲットの生成.
		device_->CreateRenderTargetView(render_target_[i].Get(), nullptr, rtv_handle_[i]);
	}
	return true;
}

bool D3DX12Manager::createDepthStencilBuffer()
{
	// 深度バッファ用のデスクリプタヒープの作成.
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = 1;						// ヒープの数.
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;		// 用途(震度バッファ).		
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// オプションフラグ.
	descriptor_heap_desc.NodeMask = 0;
	HRESULT hr = device_->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&dh_dsv_));

	// 深度バッファの作成.
	// ヒープの設定.
	D3D12_HEAP_PROPERTIES heap_properties	= {};
	heap_properties.Type					= D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask		= 0;
	heap_properties.VisibleNodeMask			= 0;

	// 生成するリソースの詳細.
	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2Dテクスチャ.
	resource_desc.Width = SetupParam::GetInstance().GetParam().windowSize_.cx;		// 幅.
	resource_desc.Height = SetupParam::GetInstance().GetParam().windowSize_.cy;		// 高さ.
	resource_desc.DepthOrArraySize = 1;	// 配列の次元数.
	resource_desc.MipLevels = 0;	// 全てのミップを生成する.
	resource_desc.Format = DXGI_FORMAT_R32_TYPELESS;	// 32bit型なしフォーマット.
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// テクスチャレイアウト.
	resource_desc.SampleDesc.Count = 1;	// マルチサンプリングのサンプリングカウント数.
	resource_desc.SampleDesc.Quality = 0;	// 品質(アンチエイリアス処理を行わない).
	resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// 深度・ステンシル用途.

	// クリアパラメータ.
	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format					= DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth		= 1.0f;
	clear_value.DepthStencil.Stencil	= 0;

	// リソースの確保(深度書き込み用).
	hr = device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&depth_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// 深度バッファビューの作成.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Format			= DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags			= D3D12_DSV_FLAG_NONE;
	// ヒープのハンドルを取得.
	dsv_handle_ = dh_dsv_->GetCPUDescriptorHandleForHeapStart();
	// 深度バッファ生成.
	device_->CreateDepthStencilView(depth_buffer_.Get(), &dsv_desc, dsv_handle_);

	return true;
}

// リソースバリア.
void D3DX12Manager::setResourceBarrier(D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state)
{
	// before_stateから、after_stateまで状態遷移するまで、バリアを設置.
	D3D12_RESOURCE_BARRIER resource_barrier = {};
	resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	// バリアはリソースの状態遷移に対して設置.
	resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resource_barrier.Transition.pResource = render_target_[buffer_index_].Get();	// 対象となるレンダーターゲット.
	resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	
	resource_barrier.Transition.StateBefore = before_state;
	resource_barrier.Transition.StateAfter = after_state;

	command_list_->ResourceBarrier(1, &resource_barrier );
}

// 実行したコマンドの終了待ち.
void D3DX12Manager::waitForPreviousFrame()
{
	const UInt64 fence = frames_;
	HRESULT hr = command_queue_->Signal(queue_fence_.Get(), fence);
	if (FAILED(hr))
	{
		ASSERT(false);
		return;
	}
	++frames_;

	if (queue_fence_->GetCompletedValue() < fence)
	{
		hr = queue_fence_->SetEventOnCompletion(fence, fence_event_);
		if (FAILED(hr))
		{
			ASSERT(false);
			return;
		}
		WaitForSingleObject(fence_event_, INFINITE);
	}
}