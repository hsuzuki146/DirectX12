#pragma once

#include "../../common/common.h"
#include "def_gltf.h"

class Model
{
public:
	Model() {}
	~Model() { }
	bool LoadFromFile( const std::string& filename, const std::string& rootPath );
private:

	glTF::BinBuffer bin_buffer_;
	UInt8* file_buffer_ = { nullptr };		// バッファ.
};