#pragma once

#include "../../common/common.h"
#include "gltf_type.h"

namespace glTF
{
	struct Buffer
	{
		UInt64		filesize_;	// ファイルサイズ.
		std::string type_;		// タイプ.
		std::string filename_;	// ファイル名.
	};

	struct BufferView
	{
		std::string buffer_;		// 親.
		Int32		byte_offset_;	// オフセット.
		Int32		byte_length_;	// サイズ.
		glTF::Type	target_;		// バッファタイプ.
	};
	struct Accessor
	{
		Int32		buffer_view_;
		Int32		byte_offset_;
		glTF::Type	component_type_;
		Int32		count_;
		std::string type_;			
	};
	struct Mesh
	{
		struct Primitive
		{
			std::map<const std::string, Int32>	attributes_;	// データ属性の並び.
			Int32							indices_;		// 頂点インデックスの参照.
			glTF::Type						mode_;			// プリミティブの描画タイプ.
			Int32							material_;		// マテリアルへの参照.
		};
		std::vector<Primitive>	primitives_;
		std::string				name_;
	};
		
}