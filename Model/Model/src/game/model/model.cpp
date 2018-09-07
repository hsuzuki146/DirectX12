#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../lib/json11.hpp"

#include "../../common/d3dx12manager.h"
using namespace DirectX;
struct Vertex3D
{
	XMFLOAT3 Position;	//位置
	XMFLOAT3 Normal;	//法線
};

bool Model::LoadFromFile(const std::string& gltf_name, const std::string& bin_name, const std::string& root_path)
{
	std::ifstream fin((root_path + gltf_name).c_str());
	if (!fin)
	{
		return false;
	}

	std::stringstream strstream;
	strstream << fin.rdbuf();
	fin.close();

	std::string data(strstream.str());
	std::string err;
	auto json = json11::Json::parse(data, err);

	// バイナリファイル情報.
	for (auto& item : json["buffers"].array_items())
	{
		glTF::Buffer buffer;
		buffer.filesize_ = item["byteLength"].int_value();
		buffer.type_ = item["type"].string_value();
		buffer.filename_ = item["uri"].string_value();
		buffers_.push_back(buffer);
	}
	
	// バッファビュー情報.
	for (auto& item : json["bufferViews"].array_items())
	{
		glTF::BufferView buffer_view;
		buffer_view.buffer_			= item["buffer"].string_value();
		buffer_view.byte_length_	= item["byteLength"].int_value();
		buffer_view.byte_offset_	= item["byteOffset"].int_value();
		buffer_view.target_			= static_cast<glTF::Type>(item["target"].int_value());
		buffer_views_.push_back(buffer_view);
	}

	// アクセサ.
	for (auto& item : json["accessors"].array_items())
	{
		glTF::Accessor accessor;
		accessor.buffer_view_		= item["bufferView"].int_value();
		accessor.byte_offset_		= item["byteOffset"].int_value();
		accessor.component_type_	= static_cast<glTF::Type>(item["componentType"].int_value());
		accessor.count_				= item["count"].int_value();
		accessor.type_				= item["type"].string_value();
		accessors_.push_back(accessor);
	}

	// メッシュ.
	for (auto& item : json["meshes"].array_items())
	{
		glTF::Mesh mesh;
			
		// 矩形.
		for (auto& primitive : item["primitives"].array_items())
		{
			glTF::Mesh::Primitive prim;
					
			// 属性.
			for (auto& object: primitive["attributes"].object_items()) 
			{
				prim.attributes_.insert( std::pair<const std::string, Int32>(object.first, object.second.int_value() ) );
			}

			prim.indices_	= primitive["indices"].int_value();
			prim.mode_		= static_cast<glTF::Type>(primitive["mode"].int_value());
			prim.material_	= primitive["material"].int_value();
			mesh.primitives_.push_back(prim);
		}
		// メッシュ名.
		mesh.name_ = item["name"].string_value();
		meshs_.push_back(mesh);
	}

	// バッファの生成.
	createBuffer();

	// バイナリファイル読み込み.
	loadBinary( bin_name, root_path );

	return true;
}
void Model::Draw()
{
	setConstantBuffer();
	setVertexBuffer();
	setIndexBuffer();
	drawInstanced();
}

bool Model::loadBinary(const std::string& bin_name, const std::string& root_path)
{
	// バイナリファイルの読み込み.
	std::ifstream fin( (root_path + bin_name), std::ios::in | std::ios::binary );
	if (!fin)
	{
		ASSERT(false);
		return false;
	}

	// メモリ確保.
	const UInt8* file_buffer = NEW_ARRAY(UInt8, buffers_[0].filesize_);
	// 読み込み.
	fin.read((char*)file_buffer, buffers_[0].filesize_);

	// バッファ生成.
	for (int i = 0; i < accessors_.size(); ++i)
	{
		createBuffer(file_buffer, i);
	}

	SAFE_DELETE_ARRAY(file_buffer);

	fin.close();

	return true;
}

bool Model::createBuffer(const UInt8* buffer_top, Int32 accessor_no)
{
	const glTF::Accessor& accessor = accessors_[accessor_no];
	const glTF::BufferView buffer_view = buffer_views_[accessor.buffer_view_];
	const UInt8* buffer = buffer_top + buffer_view.byte_offset_;
	
	// ビューの読み込み.
	switch (buffer_view.target_)
	{
		case glTF::ELEMENT_ARRAY_BUFFER:	// インデックスバッファ.
			createIndexBuffer(buffer, accessor);
			break;
		case glTF::ARRAY_BUFFER:			// 頂点バッファ.
			createVertexBuffer(buffer, accessor);
			break;
		default:
			ASSERT(false);
			break;
	}
	return true;
}

bool Model::createIndexBuffer(const UInt8* buffer, const glTF::Accessor& accessor)
{
	switch (accessor.component_type_)
	{
	case glTF::UNSIGNED_SHORT:
		{
			UInt16* map_buffer = {};
			HRESULT hr = index_buffer_->Map(0, nullptr, (void**)&map_buffer);
			if (FAILED(hr))
			{
				ASSERT(false);
				return true;
			}
			const UInt16* index_buffer = reinterpret_cast<const UInt16*>(buffer);
			for (Int32 i = 0; i < accessor.count_; ++i)
			{
				map_buffer[i] = index_buffer[i];
			}

			index_buffer_->Unmap(0, nullptr);
			buffer = NULL;
			index_buffer_num_ = accessor.count_;
		}
		break;
	default:
		ASSERT(false);
		break;
	}
	return true;
}
bool Model::createVertexBuffer(const UInt8* buffer, const glTF::Accessor& accessor)
{
	switch (accessor.component_type_)
	{
		case glTF::FLOAT:
		{
			if (accessor.type_ == "VEC3")
			{
				Vertex3D* map_buffer = {};
				HRESULT hr = vertex_buffer_->Map(0, nullptr, (void**)&map_buffer);
				if (FAILED(hr))
				{
					ASSERT(false);
					return false;
				}
				if (accessor.buffer_view_ == 1)
				{
					// 法線.
					const DirectX::XMFLOAT3* normal_buffer = reinterpret_cast<const DirectX::XMFLOAT3*>(buffer);
					for (Int32 i = 0; i < accessor.count_; ++i)
					{
						map_buffer[i].Normal =normal_buffer[i];
					}
				}
				else if (accessor.buffer_view_ == 2)
				{
					// 座標.
					const DirectX::XMFLOAT3* vertex_buffer = reinterpret_cast<const DirectX::XMFLOAT3*>(buffer);
					for (Int32 i = 0; i < accessor.count_; ++i)
					{
						map_buffer[i].Position = vertex_buffer[i];
					}
					vertex_buffer_num_ = accessor.count_;
				}

				vertex_buffer_->Unmap(0, nullptr);
			}
			else if (accessor.type_ == "VEC2")
			{
				const DirectX::XMFLOAT2* vertex_buffer = reinterpret_cast<const DirectX::XMFLOAT2*>(buffer);
				for (Int32 i = 0; i < accessor.count_; ++i)
				{
					//std::cout << "[" << vertex_buffer[i].x << "," << vertex_buffer[i].y << "]" << std::endl;
				}
			}
			else
			{
				ASSERT(false);
			}
		}
		break;
	default:
		ASSERT(false);
		break;
	}
	return true;
}

bool Model::createBuffer()
{
	// 物理メモリ設定.
	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;		// GPUへの転送に最適化.
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// CPU側で読み書き.
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;	// メモリの取り方はお任せ.
	heap_properties.CreationNodeMask = 0;
	heap_properties.VisibleNodeMask = 0;

	// バッファ設定.
	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;	//用途(バッファの塊).
	resource_desc.Width = 256;
	resource_desc.Height = 1;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.Format = DXGI_FORMAT_UNKNOWN;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.SampleDesc.Quality = 0;

	// 頂点バッファの作成.
	HRESULT hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// インデックスバッファの作成.
	hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&index_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}

	// 定数バッファの作成.
	hr = D3D_DEVICE()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constant_buffer_));
	if (FAILED(hr))
	{
		ASSERT(false);
		return false;
	}
	return true;
}

void Model::setConstantBuffer()
{
	// カメラの設定.
	const XMMATRIX view = XMMatrixLookAtLH({ 0.0f,0.0f,-5.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f });
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx) / static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy),
		1.0f,
		20.0f);

	// ビュープロジェクション行列.
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixTranspose(view * projection));

	XMFLOAT4X4* buffer = {};
	HRESULT hr = constant_buffer_->Map(0, nullptr, (void**)&buffer);
	if (FAILED(hr))
	{
		ASSERT(false);
		return;
	}
	// 行列を定数バッファに書き込み.
	*buffer = Mat;
	constant_buffer_->Unmap(0, nullptr);
	buffer = nullptr;

	// 定数バッファをシェーダのレジスタにセット.
	D3D_COMMAND_LIST()->SetGraphicsRootConstantBufferView(0, constant_buffer_->GetGPUVirtualAddress());
}

void Model::setVertexBuffer()
{
	D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
	vertex_view.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
	vertex_view.StrideInBytes = sizeof(Vertex3D);
	vertex_view.SizeInBytes = sizeof(Vertex3D) * vertex_buffer_num_;
	D3D_COMMAND_LIST()->IASetVertexBuffers(0, 1, &vertex_view);
}

void Model::setIndexBuffer()
{
	D3D12_INDEX_BUFFER_VIEW index_view = {};
	index_view.BufferLocation = index_buffer_->GetGPUVirtualAddress();
	index_view.SizeInBytes = sizeof(UInt16) * index_buffer_num_;
	index_view.Format = DXGI_FORMAT_R16_UINT; 
	D3D_COMMAND_LIST()->IASetIndexBuffer(&index_view);
}

void Model::drawInstanced()
{
	// インデックスを使用しないトライアングルリストで描画.
	D3D_COMMAND_LIST()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 描画.
	D3D_COMMAND_LIST()->DrawInstanced(vertex_buffer_num_, 1, 0, 0);
}
