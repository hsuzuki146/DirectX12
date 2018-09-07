#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../lib/json11.hpp"

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

	// バイナリファイル読み込み.
	loadBinary( bin_name, root_path );

	return true;
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
			const UInt16* index_buffer = reinterpret_cast<const UInt16*>(buffer);
			for (Int32 i = 0; i < accessor.count_; ++i)
			{
				//std::cout << index_buffer[i] << std::endl;
			}
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
				const DirectX::XMFLOAT3* vertex_buffer = reinterpret_cast<const DirectX::XMFLOAT3*>(buffer);
				for (Int32 i = 0; i < accessor.count_; ++i)
				{
					//std::cout << "[" << vertex_buffer[i].x << "," << vertex_buffer[i].y  << "," << vertex_buffer[i].z << "]" << std::endl;
				}
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