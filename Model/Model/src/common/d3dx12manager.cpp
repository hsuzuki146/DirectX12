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
	if(!createRootSignature()) { return false; }
	if(!createPipelineStateObject()) { return false; }

	setupViewport();
	setupScissor();

	return true;
}

void D3DX12Manager::Destroy()
{
}

void D3DX12Manager::Begin()
{
	// リソースの状態をプレゼント用からレンダーターゲット用に変更.
	setResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	const Float32 clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	// 深度バッファとレンダーターゲットのクリア.
	command_list_->ClearDepthStencilView(dsv_handle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	command_list_->ClearRenderTargetView(rtv_handle_[buffer_index_], clearColor, 0, nullptr);
}
void D3DX12Manager::PreRender()
{
	// ルートシグネチャとPSOの設定.
	command_list_->SetGraphicsRootSignature(root_signature_.Get());
	command_list_->SetPipelineState(pipeline_state_.Get());

	// ビューポートとシザー矩形の設定.
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_rect_);

	// レンダーターゲットの設定.
	command_list_->OMSetRenderTargets(1, &rtv_handle_[buffer_index_], TRUE, &dsv_handle_);
}
void D3DX12Manager::End()
{
	// リソースの状態をレンダーターゲット用からプレゼント用に変更.
	setResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	HRESULT hr = command_list_->Close();
	if (FAILED(hr))
	{
		ASSERT(false);
	}
}
void D3DX12Manager::ExecuteCommand()
{
	ID3D12CommandList* const command_lists = command_list_.Get();
	command_queue_->ExecuteCommandLists(1, &command_lists);
}
void D3DX12Manager::WaitFrame()
{
	waitForPreviousFrame();
}
void D3DX12Manager::ResetCommand()
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
void D3DX12Manager::Present()
{
	// フロントバッファとバックバッファの切り替え.
	HRESULT hr = swap_chain_->Present(1, 0);
	if (FAILED(hr))
	{
		ASSERT(false);
		return;
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

bool D3DX12Manager::createRootSignature()
{
	// 定数バッファとシェーダーの関連付け.

	D3D12_ROOT_PARAMETER		root_parameters[1] = {};	// パラメータ.
	D3D12_ROOT_SIGNATURE_DESC	root_signature_desc = {};	// 用途.
	ComPtr<ID3DBlob>			blob = {};					// バイナリデータ.

	root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// 定数バッファ.
	root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;		// どこでもアクセス可能.
	root_parameters[0].Descriptor.ShaderRegister = 0;						// シェーダレジスタ0.
	root_parameters[0].Descriptor.RegisterSpace = 0;						// b.

	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	// 頂点バッファに関連付ける.
	root_signature_desc.NumParameters = _countof(root_parameters);								// パラメータ数.
	root_signature_desc.pParameters = root_parameters;											// パラメータ.
	root_signature_desc.NumStaticSamplers = 0;													// サンプリング数.
	root_signature_desc.pStaticSamplers = nullptr;												// サンプリングパラメータ.

																								// RootSignatureを作成するのに必要なバッファを確保し、Tableの情報をシリアライズする.
	HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	// ルートシグネチャの生成.
	hr = device_->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_signature_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	return true;
}

bool D3DX12Manager::createPipelineStateObject()
{
	ASSERT(root_signature_);

	// シェーダと描画ステート周りの設定.
	HRESULT hr = S_OK;

	// シェーダの最適化設定.
#if defined(_DEBUG)
	UINT32 compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT32 compile_flag = 0;
#endif

	// 頂点シェーダのコンパイル.
	ComPtr<ID3DBlob> vertex_shader = {};
	hr = D3DCompileFromFile(L"src/shader/shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compile_flag, 0, vertex_shader.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// ピクセルシェーダのコンパイル.
	ComPtr<ID3DBlob> pixel_shader = {};
	hr = D3DCompileFromFile(L"src/shader/shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compile_flag, 0, pixel_shader.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// 頂点レイアウト.
	D3D12_INPUT_ELEMENT_DESC InputElementDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// グラフィックパイプライン.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};

	// シェーダーの設定.
	pipeline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
	pipeline_state_desc.VS.BytecodeLength = vertex_shader->GetBufferSize();
	pipeline_state_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
	pipeline_state_desc.PS.BytecodeLength = pixel_shader->GetBufferSize();

	// インプットレイアウトの設定.
	pipeline_state_desc.InputLayout.pInputElementDescs = InputElementDesc;
	pipeline_state_desc.InputLayout.NumElements = _countof(InputElementDesc);

	// サンプル系の設定.
	pipeline_state_desc.SampleDesc.Count = 1;
	pipeline_state_desc.SampleDesc.Quality = 0;
	pipeline_state_desc.SampleMask = UINT_MAX;

	// レンダーターゲットの設定.
	pipeline_state_desc.NumRenderTargets = 1;
	pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

	// 描画タイプを三角形に設定.
	pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ルートシグネチャの指定.
	pipeline_state_desc.pRootSignature = root_signature_.Get();

	// ラスタライザーステートの設定(三角形を実際に描画する方法).
	pipeline_state_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipeline_state_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipeline_state_desc.RasterizerState.FrontCounterClockwise = FALSE;	// 右回りで前向き.
	pipeline_state_desc.RasterizerState.DepthBias = 0;
	pipeline_state_desc.RasterizerState.DepthBiasClamp = 0;
	pipeline_state_desc.RasterizerState.SlopeScaledDepthBias = 0;
	pipeline_state_desc.RasterizerState.DepthClipEnable = TRUE;
	pipeline_state_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;	// 少しでもピクセルに掛かったらラスタライズする.
	pipeline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipeline_state_desc.RasterizerState.MultisampleEnable = FALSE;

	// ブレンドステートの設定.
	for (int i = 0; i < _countof(pipeline_state_desc.BlendState.RenderTarget); ++i)
	{
		pipeline_state_desc.BlendState.RenderTarget[i].BlendEnable = FALSE;
		pipeline_state_desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
		pipeline_state_desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
		pipeline_state_desc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		pipeline_state_desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		pipeline_state_desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
		pipeline_state_desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		pipeline_state_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		pipeline_state_desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
		pipeline_state_desc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;
	}
	pipeline_state_desc.BlendState.AlphaToCoverageEnable = FALSE;	// レンダーターゲットを設定するときに、アルファトゥカバレッジをMSテクニックとして使用するか.
	pipeline_state_desc.BlendState.IndependentBlendEnable = FALSE;	// RenderTarget[0]のみをブレンディングする.

	// デプスステンシルステートの設定.
	pipeline_state_desc.DepthStencilState.DepthEnable = TRUE;	// 深度テストあり.
	pipeline_state_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pipeline_state_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipeline_state_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	pipeline_state_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	pipeline_state_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	pipeline_state_desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipeline_state_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	pipeline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// グラフィックスパイプラインの設定.
	hr = device_->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	return true;
}

void D3DX12Manager::setupViewport()
{
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.Width = static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx);
	viewport_.Height = static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}

void D3DX12Manager::setupScissor()
{
	scissor_rect_.top = 0;
	scissor_rect_.left = 0;
	scissor_rect_.right = SetupParam::GetInstance().GetParam().windowSize_.cx;
	scissor_rect_.bottom = SetupParam::GetInstance().GetParam().windowSize_.cy;
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