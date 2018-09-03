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
		const float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

		// ���\�[�X�̏�Ԃ��v���[���g�p���烌���_�[�^�[�Q�b�g�p�ɕύX.
		setResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �[�x�o�b�t�@�ƃ����_�[�^�[�Q�b�g�̃N���A.
		command_list_->ClearDepthStencilView(dsv_handle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		command_list_->ClearRenderTargetView(rtv_handle_[buffer_index_], clearColor, 0, nullptr);

		// �����_�[�^�[�Q�b�g�̐ݒ�.
		command_list_->OMSetRenderTargets(1, &rtv_handle_[buffer_index_], TRUE, &dsv_handle_);

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
	UINT flag = {};
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
	const UINT size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < RTV_NUM; ++i)
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
	const UINT64 fence = frames_;
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