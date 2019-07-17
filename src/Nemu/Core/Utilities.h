// -----------------------------------------------------------------------------------------* C++ *-
// System.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <fstream>
#include <iostream>

namespace nemu::util {

	bool StringEndsWith(const std::string& str, const std::string& suffix)
	{
		return str.size() >= suffix.size() &&
			str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	template <class T>
	inline T ReadFile(const std::string& path)
	{
		std::ifstream in{ path, std::ios::binary };
		if (in.fail()) {
			std::cout << "There was a problem reading the requested file." << std::endl;
			return T{};
		}
		return T{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	}

	void WriteFile(const std::string& path, const char* buff, std::streamsize length)
	{
		std::ofstream output{ path, std::ios::binary };
		output.write(buff, length);
	}

}