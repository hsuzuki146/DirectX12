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

	// ���f���̃��[�h.
	model_.LoadFromFile("Duck.gltf", "data/");
	//model_.LoadFromFile("Box.gltf", "data/");

	return true;
}

void D3DX12Manager::Destroy()
{
}

void D3DX12Manager::Render()
{
	{
		const Float32 clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

		// ���\�[�X�̏�Ԃ��v���[���g�p���烌���_�[�^�[�Q�b�g�p�ɕύX.
		setResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �[�x�o�b�t�@�ƃ����_�[�^�[�Q�b�g�̃N���A.
		command_list_->ClearDepthStencilView(dsv_handle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		command_list_->ClearRenderTargetView(rtv_handle_[buffer_index_], clearColor, 0, nullptr);

		// ���[�g�V�O�l�`����PSO�̐ݒ�.
		command_list_->SetGraphicsRootSignature(root_signature_.Get());
		command_list_->SetPipelineState(pipeline_state_.Get());

		// �r���[�|�[�g�ƃV�U�[��`�̐ݒ�.
		command_list_->RSSetViewports(1, &viewport_);
		command_list_->RSSetScissorRects(1, &scissor_rect_);

		// �����_�[�^�[�Q�b�g�̐ݒ�.
		command_list_->OMSetRenderTargets(1, &rtv_handle_[buffer_index_], TRUE, &dsv_handle_);

		// ���f���̕`��.
		model_.Draw();

		// ���\�[�X�̏�Ԃ������_�[�^�[�Q�b�g�p����v���[��y�Ɨp�ɕύX.
		setResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		HRESULT hr = command_list_->Close();
	}

	// �R�}���h���X�g�̎��s.
	{
		ID3D12CommandList* const command_lists = command_list_.Get();
		command_queue_->ExecuteCommandLists(1, &command_lists);
	}

	// ���s�����R�}���h�̏I���҂�.
	{
		waitForPreviousFrame();
	}

	// �e�탊�Z�b�g����.
	{
		// �A���P�[�^.
		HRESULT hr = command_allocator_->Reset();
		if ((FAILED(hr)))
		{
			ASSERT(false);
			return;
		}

		// �R�}���h���X�g.
		hr = command_list_->Reset(command_allocator_.Get(), nullptr);
		if ((FAILED(hr)))
		{
			ASSERT(false);
			return;
		}
	}

	// �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@�̐؂�ւ�.
	{
		HRESULT hr = swap_chain_->Present(1, 0);
		if (FAILED(hr))
		{
			ASSERT(false);
			return;
		}
	}

	// �J�����g�̃o�b�N�o�b�t�@�̃C���f�b�N�X���擾����.
	buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();
}

bool D3DX12Manager::createFactory()
{
#if defined(_DEBUG)
	// �f�o�b�O���C���[��L���ɂ���.
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

	// �R�}���h�L���[�̐���.
	command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	// �����_�����O�Ɋ֌W����R�}���h�����s����ׂ̃R�}���h���X�g.
	command_queue_desc.Priority = 0;
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// �^�C���A�E�g�����͍s��Ȃ�.
	command_queue_desc.NodeMask = 0;
	HRESULT hr = device_->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(command_queue_.GetAddressOf()));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �R�}���h�L���[�I���Ď��p�̃t�F���X.
	// �t�F���X�C�x���g�̐���.
	fence_event_ = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (fence_event_ == nullptr)
	{
		ASSERT(false);
		return false;
	}
	// �t�F���X�̐���.
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
		D3D12_COMMAND_LIST_TYPE_DIRECT,		// �����_�����O�p�r.
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
	// �R�}���h�A���P�[�^�ƃo�C���h���āA�R�}���h���X�g���쐬����.
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

	desc.BufferDesc.Width					= param.windowSize_.cx;				// ��.
	desc.BufferDesc.Height					= param.windowSize_.cy;				// ����.
	desc.OutputWindow						= Window::GetInstance().GetWnd();	// �E�B���h�E�n���h��.
	desc.Windowed							= TRUE;								// �E�B���h�E�\���t���O.
	desc.BufferUsage						= DXGI_USAGE_RENDER_TARGET_OUTPUT;	// �p�r(�����_�[�^�[�Q�b�g).
	desc.BufferCount						= RTV_NUM;							// �����_�[�^�[�Q�b�g�̐�.
	desc.SwapEffect							= DXGI_SWAP_EFFECT_FLIP_DISCARD;	// �X���b�v���@.
	desc.Flags								= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// �E�B���h�E���[�h�ƃt���X�N���[���������Ő؂�ւ�.
	desc.BufferDesc.RefreshRate.Numerator	= 60;	// ���t���b�V���f�[�^�̕��q.
	desc.BufferDesc.RefreshRate.Denominator = 1;	// ���t���b�V���f�[�^�̕���.
	desc.BufferDesc.Format					= DXGI_FORMAT_B8G8R8A8_UNORM;	// �e�N�X�`���t�H�[�}�b�g.
	desc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// �X���b�v�������̑������̏���.
	desc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;	// ����̃��j�^�[�̉𑜓x�ɍ��킹�ăC���[�W���������΂����@�������t���O.
	desc.SampleDesc.Count					= 1;	// �s�N�Z���P�ʂ̃T���v�����O��.
	desc.SampleDesc.Quality					= 0;	// �C���[�W�̕i�����x��.

	// �X���b�v�`�F�C���̐���.
	HRESULT hr = factory_->CreateSwapChain(command_queue_.Get(), &desc, swap_chain.GetAddressOf());
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	// �X���b�v�`�F�C���̐ݒ���T�|�[�g���Ă��邩.
	hr = swap_chain->QueryInterface(swap_chain_.GetAddressOf());
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �J�����g�̃o�b�N�o�b�t�@�̃C���f�b�N�X���擾����.
	buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();

	return true;
}

// �����_�[�^�[�Q�b�g�̍쐬.
bool D3DX12Manager::createRenderTarget()
{
	ASSERT(swap_chain_);

	// �����_�[�^�[�Q�b�g�p�̃q�[�v�쐬.
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc	= {};
	heap_desc.NumDescriptors				= RTV_NUM;
	heap_desc.Type							= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// �p�r(�����_�[�^�[�Q�b�g).
	heap_desc.Flags							= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// �t���O.
	heap_desc.NodeMask						= 0;
	HRESULT hr = device_->CreateDescriptorHeap( &heap_desc, IID_PPV_ARGS(dh_rtv_.GetAddressOf()) );
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �����_�[�^�[�Q�b�g�̍쐬����.
	const UInt32 size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UInt32 i = 0; i < RTV_NUM; ++i)
	{
		// �X���b�v�`�F�C������o�b�t�@���󂯎��.
		HRESULT hr = swap_chain_->GetBuffer(i, IID_PPV_ARGS(&render_target_[i]));
		if (FAILED(hr))
		{
			ASSERT(false);
			return false;
		}

		// RenderTargetView���쐬���ăq�[�v�ŃX�N���v�^�ɓo�^.
		// DescriptorHeap�̊J�n�ʒu�̃n���h�����擾.
		rtv_handle_[i] = dh_rtv_->GetCPUDescriptorHandleForHeapStart();
		// ��������̃I�t�Z�b�g.
		rtv_handle_[i].ptr += (size * i);
		// �����_�[�^�[�Q�b�g�̐���.
		device_->CreateRenderTargetView(render_target_[i].Get(), nullptr, rtv_handle_[i]);
	}
	return true;
}

bool D3DX12Manager::createDepthStencilBuffer()
{
	// �[�x�o�b�t�@�p�̃f�X�N���v�^�q�[�v�̍쐬.
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = 1;						// �q�[�v�̐�.
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;		// �p�r(�k�x�o�b�t�@).		
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// �I�v�V�����t���O.
	descriptor_heap_desc.NodeMask = 0;
	HRESULT hr = device_->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&dh_dsv_));

	// �[�x�o�b�t�@�̍쐬.
	// �q�[�v�̐ݒ�.
	D3D12_HEAP_PROPERTIES heap_properties	= {};
	heap_properties.Type					= D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask		= 0;
	heap_properties.VisibleNodeMask			= 0;

	// �������郊�\�[�X�̏ڍ�.
	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2D�e�N�X�`��.
	resource_desc.Width = SetupParam::GetInstance().GetParam().windowSize_.cx;		// ��.
	resource_desc.Height = SetupParam::GetInstance().GetParam().windowSize_.cy;		// ����.
	resource_desc.DepthOrArraySize = 1;	// �z��̎�����.
	resource_desc.MipLevels = 0;	// �S�Ẵ~�b�v�𐶐�����.
	resource_desc.Format = DXGI_FORMAT_R32_TYPELESS;	// 32bit�^�Ȃ��t�H�[�}�b�g.
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// �e�N�X�`�����C�A�E�g.
	resource_desc.SampleDesc.Count = 1;	// �}���`�T���v�����O�̃T���v�����O�J�E���g��.
	resource_desc.SampleDesc.Quality = 0;	// �i��(�A���`�G�C���A�X�������s��Ȃ�).
	resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// �[�x�E�X�e���V���p�r.

	// �N���A�p�����[�^.
	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format					= DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth		= 1.0f;
	clear_value.DepthStencil.Stencil	= 0;

	// ���\�[�X�̊m��(�[�x�������ݗp).
	hr = device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&depth_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �[�x�o�b�t�@�r���[�̍쐬.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Format			= DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension	= D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags			= D3D12_DSV_FLAG_NONE;
	// �q�[�v�̃n���h�����擾.
	dsv_handle_ = dh_dsv_->GetCPUDescriptorHandleForHeapStart();
	// �[�x�o�b�t�@����.
	device_->CreateDepthStencilView(depth_buffer_.Get(), &dsv_desc, dsv_handle_);

	return true;
}

bool D3DX12Manager::createRootSignature()
{
	// �萔�o�b�t�@�ƃV�F�[�_�[�̊֘A�t��.

	D3D12_ROOT_PARAMETER		root_parameters[1] = {};	// �p�����[�^.
	D3D12_ROOT_SIGNATURE_DESC	root_signature_desc = {};	// �p�r.
	ComPtr<ID3DBlob>			blob = {};					// �o�C�i���f�[�^.

	root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// �萔�o�b�t�@.
	root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;		// �ǂ��ł��A�N�Z�X�\.
	root_parameters[0].Descriptor.ShaderRegister = 0;						// �V�F�[�_���W�X�^0.
	root_parameters[0].Descriptor.RegisterSpace = 0;						// b.

	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;	// ���_�o�b�t�@�Ɋ֘A�t����.
	root_signature_desc.NumParameters = _countof(root_parameters);								// �p�����[�^��.
	root_signature_desc.pParameters = root_parameters;											// �p�����[�^.
	root_signature_desc.NumStaticSamplers = 0;													// �T���v�����O��.
	root_signature_desc.pStaticSamplers = nullptr;												// �T���v�����O�p�����[�^.

																								// RootSignature���쐬����̂ɕK�v�ȃo�b�t�@���m�ۂ��ATable�̏����V���A���C�Y����.
	HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	// ���[�g�V�O�l�`���̐���.
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

	// �V�F�[�_�ƕ`��X�e�[�g����̐ݒ�.
	HRESULT hr = S_OK;

	// �V�F�[�_�̍œK���ݒ�.
#if defined(_DEBUG)
	UINT32 compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT32 compile_flag = 0;
#endif

	// ���_�V�F�[�_�̃R���p�C��.
	ComPtr<ID3DBlob> vertex_shader = {};
	hr = D3DCompileFromFile(L"src/shader/shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compile_flag, 0, vertex_shader.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// �s�N�Z���V�F�[�_�̃R���p�C��.
	ComPtr<ID3DBlob> pixel_shader = {};
	hr = D3DCompileFromFile(L"src/shader/shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compile_flag, 0, pixel_shader.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// ���_���C�A�E�g.
	D3D12_INPUT_ELEMENT_DESC InputElementDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// �O���t�B�b�N�p�C�v���C��.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};

	// �V�F�[�_�[�̐ݒ�.
	pipeline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
	pipeline_state_desc.VS.BytecodeLength = vertex_shader->GetBufferSize();
	pipeline_state_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
	pipeline_state_desc.PS.BytecodeLength = pixel_shader->GetBufferSize();

	// �C���v�b�g���C�A�E�g�̐ݒ�.
	pipeline_state_desc.InputLayout.pInputElementDescs = InputElementDesc;
	pipeline_state_desc.InputLayout.NumElements = _countof(InputElementDesc);

	// �T���v���n�̐ݒ�.
	pipeline_state_desc.SampleDesc.Count = 1;
	pipeline_state_desc.SampleDesc.Quality = 0;
	pipeline_state_desc.SampleMask = UINT_MAX;

	// �����_�[�^�[�Q�b�g�̐ݒ�.
	pipeline_state_desc.NumRenderTargets = 1;
	pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

	// �`��^�C�v���O�p�`�ɐݒ�.
	pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���[�g�V�O�l�`���̎w��.
	pipeline_state_desc.pRootSignature = root_signature_.Get();

	// ���X�^���C�U�[�X�e�[�g�̐ݒ�(�O�p�`�����ۂɕ`�悷����@).
	pipeline_state_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipeline_state_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipeline_state_desc.RasterizerState.FrontCounterClockwise = FALSE;	// �E���őO����.
	pipeline_state_desc.RasterizerState.DepthBias = 0;
	pipeline_state_desc.RasterizerState.DepthBiasClamp = 0;
	pipeline_state_desc.RasterizerState.SlopeScaledDepthBias = 0;
	pipeline_state_desc.RasterizerState.DepthClipEnable = TRUE;
	pipeline_state_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;	// �����ł��s�N�Z���Ɋ|�������烉�X�^���C�Y����.
	pipeline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipeline_state_desc.RasterizerState.MultisampleEnable = FALSE;

	// �u�����h�X�e�[�g�̐ݒ�.
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
	pipeline_state_desc.BlendState.AlphaToCoverageEnable = FALSE;	// �����_�[�^�[�Q�b�g��ݒ肷��Ƃ��ɁA�A���t�@�g�D�J�o���b�W��MS�e�N�j�b�N�Ƃ��Ďg�p���邩.
	pipeline_state_desc.BlendState.IndependentBlendEnable = FALSE;	// RenderTarget[0]�݂̂��u�����f�B���O����.

	// �f�v�X�X�e���V���X�e�[�g�̐ݒ�.
	pipeline_state_desc.DepthStencilState.DepthEnable = TRUE;	// �[�x�e�X�g����.
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

	// �O���t�B�b�N�X�p�C�v���C���̐ݒ�.
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

// ���\�[�X�o���A.
void D3DX12Manager::setResourceBarrier(D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state)
{
	// before_state����Aafter_state�܂ŏ�ԑJ�ڂ���܂ŁA�o���A��ݒu.
	D3D12_RESOURCE_BARRIER resource_barrier = {};
	resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	// �o���A�̓��\�[�X�̏�ԑJ�ڂɑ΂��Đݒu.
	resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resource_barrier.Transition.pResource = render_target_[buffer_index_].Get();	// �ΏۂƂȂ郌���_�[�^�[�Q�b�g.
	resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	
	resource_barrier.Transition.StateBefore = before_state;
	resource_barrier.Transition.StateAfter = after_state;

	command_list_->ResourceBarrier(1, &resource_barrier );
}

// ���s�����R�}���h�̏I���҂�.
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