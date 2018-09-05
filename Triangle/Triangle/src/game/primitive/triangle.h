#pragma once

#include "../../common/common.h"
#include <d3d12.h>
#include <wrl/client.h>	// ComPtr.

using namespace Microsoft::WRL;

class Triangle
{
public:
	Triangle() {}
	~Triangle() {}
	bool Initialize(ID3D12Device* device);
	void Draw(ID3D12GraphicsCommandList* ommand_list);

private:
	ComPtr<ID3D12Resource>	vertex_buffer_;
	ComPtr<ID3D12Resource>	constant_buffer_;
};