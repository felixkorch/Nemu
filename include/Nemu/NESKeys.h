#pragma once
#include <unordered_map>

enum class NESKey {
	A, B, Left, Right, Up, Down, Start, Select
};

class NESKeyMapper {
private:
	std::unordered_map<NESKey, int> keys;
public:
	static void RunMapper()
	{

	}

	void MapKey(NESKey from, int to)
	{
		keys[from] = to;
	}

	int GetKey(NESKey nesKey)
	{
		return keys[nesKey];
	}
};