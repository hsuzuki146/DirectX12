#pragma once

#include "../../common/common.h"
#include "def_gltf.h"

#include <d3d12.h>
#include <wrl/client.h>	// ComPtr.
using namespace Microsoft::WRL;

class Model
{
public:
	Model() {}
	~Model() { }
	bool LoadFromFile( const std::string& gltf_name, const std::string& root_path );
	void Draw();

private:
	bool loadBinary(const std::string& bin_name, const std::string& root_path);
	bool createBuffer(const UInt8* buffer_top, Int32 accessor_no);
	bool createIndexBuffer( const UInt8* buffer, const glTF::Accessor& accessor);
	bool createVertexBuffer(const UInt8* buffer, const glTF::Accessor& accessor, Int32 accessor_no);
	bool createBuffer();

	void setConstantBuffer();
	void setVertexBuffer();
	void setIndexBuffer();
	void drawInstanced();

private:
	std::vector<glTF::Buffer>		buffers_;
	std::vector<glTF::BufferView>	buffer_views_;
	std::vector<glTF::Accessor>		accessors_;
	std::vector<glTF::Mesh>			meshs_;

private:
	ComPtr<ID3D12Resource>	vertex_buffer_;
	ComPtr<ID3D12Resource>	index_buffer_;
	ComPtr<ID3D12Resource>	constant_buffer_;
	
	Int32					vertex_buffer_num_ = { 0 };
	Int32					index_buffer_num_ = { 0 };
};