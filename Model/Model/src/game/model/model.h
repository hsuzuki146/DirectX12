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
	~Model();
	bool LoadFromFile( const std::string& gltf_name, const std::string& root_path );
	void Draw();

private:
	bool loadglTF(const std::string& gltf_name, const std::string& root_path);
	// 各種バッファの生成.
	bool createBuffer();
	bool createVertexBuffer();
	bool createIndexBuffer();
	bool createConstantBuffer();
	UInt64 getIndexBufferSize() const;
	UInt64 getVertexBufferSize() const;
	UInt64 getConstantBufferSize() const;

	// モデルデータの読み込み処理.
	bool loadModel(const std::string& bin_name, const std::string& root_path); 
	bool setupBuffer(const UInt8* buffer_top, Int32 accessor_no);
	bool setupIndexBuffer( const UInt8* buffer, const glTF::Accessor& accessor);
	bool setupVertexBuffer(const UInt8* buffer, const glTF::Accessor& accessor, Int32 accessor_no);
	void setupIndexBufferView();
	void setupVertexBufferView();

	// メインループ時に行う処理.
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
	ComPtr<ID3D12Resource>		vertex_buffer_;
	ComPtr<ID3D12Resource>		index_buffer_;
	ComPtr<ID3D12Resource>		constant_buffer_;
	D3D12_VERTEX_BUFFER_VIEW	vertex_view_ = {};
	D3D12_INDEX_BUFFER_VIEW		index_view_ = {};

	Int32						vertex_buffer_num_ = { 0 };
	Int32						index_buffer_num_ = { 0 };
};