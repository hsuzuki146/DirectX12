#pragma once

#include "../../common/common.h"
#include "def_gltf.h"

class Model
{
public:
	Model() {}
	~Model() { }
	bool LoadFromFile( const std::string& gltf_name, const std::string& bin_name, const std::string& root_path );

private:
	bool loadBinary(const std::string& bin_name, const std::string& root_path);
	bool createBuffer(const UInt8* buffer_top, Int32 accessor_no);
	bool createIndexBuffer( const UInt8* buffer, const glTF::Accessor& accessor);
	bool createVertexBuffer(const UInt8* buffer, const glTF::Accessor& accessor);

private:
	std::vector<glTF::Buffer>		buffers_;
	std::vector<glTF::BufferView>	buffer_views_;
	std::vector<glTF::Accessor>		accessors_;
	std::vector<glTF::Mesh>			meshs_;

};