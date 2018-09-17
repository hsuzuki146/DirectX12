#pragma once

#include "../../common/common.h"
#include "gltf_type.h"

namespace glTF
{
	struct Buffer
	{
		UInt64		filesize_;	// �t�@�C���T�C�Y.
		std::string type_;		// �^�C�v.
		std::string filename_;	// �t�@�C����.
	};

	struct BufferView
	{
		std::string buffer_;		// �e.
		Int32		byte_offset_;	// �I�t�Z�b�g.
		Int32		byte_length_;	// �T�C�Y.
		glTF::Type	target_;		// �o�b�t�@�^�C�v.
	};
	struct Accessor
	{
		Int32		buffer_view_;
		Int32		byte_offset_;
		glTF::Type	component_type_;
		Int32		count_;
		std::string type_;	
		std::vector<Float64>	min_;
		std::vector<Float64>	max_;
	};
	struct Mesh
	{
		struct Primitive
		{
			std::map<const std::string, Int32>	attributes_;	// �f�[�^�����̕���.
			Int32							indices_;		// ���_�C���f�b�N�X�̎Q��.
			glTF::Type						mode_;			// �v���~�e�B�u�̕`��^�C�v.
			Int32							material_;		// �}�e���A���ւ̎Q��.
		};
		std::vector<Primitive>	primitives_;
		std::string				name_;
	};
	
#define VERTEX_BUFFER_SIZE( type, count ) (sizeof(type) * count)
#define INDEX_BUFFER_SIZE( count ) (sizeof(UInt16) * count)
#define CONSTANT_BUFFER_SIZE( type ) (sizeof(type))

}