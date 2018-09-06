#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../lib/json11.hpp"

bool Model::LoadFromFile(const std::string& filename, const std::string& rootPath)
{
	std::ifstream fin( (rootPath + filename).c_str());
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
		bin_buffer_.filesize_ = item["byteLength"].int_value();
		bin_buffer_.filename_ = item["uri"].string_value();
	}
	
	file_buffer_ = NEW_ARRAY(UInt8, bin_buffer_.filesize_);

	return true;
}