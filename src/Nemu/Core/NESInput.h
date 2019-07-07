// -----------------------------------------------------------------------------------------* C++ *-
// NESInput.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/Graphics/Input.h"
#include <unordered_map>

namespace nemu {

	struct EnumClassHash
	{
		template <typename T>
		int operator()(T t) const
		{
			return static_cast<int>(t);
		}
	};

	template <typename Key>
	using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

	template <typename Key, typename T>
	using UnorderedMap = std::unordered_map<Key, T, HashType<Key>>;

	enum class NESButton {
		A, B, Left, Right, Up, Down, Start, Select
	};

	class NESKeyMapper {
		UnorderedMap<NESButton, int> map;
	public:

		static NESKeyMapper DefaultMap()
		{
			NESKeyMapper map;
			map.Map(NESButton::A, GLFW_KEY_X);
			map.Map(NESButton::B, GLFW_KEY_Z);
			map.Map(NESButton::Start, GLFW_KEY_ENTER);
			map.Map(NESButton::Select, GLFW_KEY_BACKSPACE);
			map.Map(NESButton::Up, GLFW_KEY_UP);
			map.Map(NESButton::Down, GLFW_KEY_DOWN);
			map.Map(NESButton::Left, GLFW_KEY_LEFT);
			map.Map(NESButton::Right, GLFW_KEY_RIGHT);
			return map;
		}

		void Reset()
		{
			map.clear();
		}

		void Map(NESButton from, int to)
		{
			map[from] = to;
		}

		bool Get(NESButton btn)
		{
			return graphics::Input::IsKeyPressed(map[btn]);
		}
	};


	class NESJoystickMapper {
		UnorderedMap<NESButton, int> keyMap;
	public:

		void Reset()
		{
			keyMap.clear();
		}

		void MapKey(NESButton from, int to)
		{
			keyMap[from] = to;
		}

		bool Get(NESButton btn)
		{
			if (!graphics::Input::IsJoystickPresent(0))
				return false;
			return graphics::Input::IsJoystickButtonPressed(keyMap[btn], 0);
		}
	};

	class NESInput {
		NESKeyMapper keyMapper;
		NESJoystickMapper joystickMapper;

	public:

		void SetKeyboardConfig(const NESKeyMapper& mapper)
		{
			keyMapper = NESKeyMapper(mapper);
		}

		void SetJoystickConfig(const NESJoystickMapper& mapper)
		{
			joystickMapper = NESJoystickMapper(mapper);
		}

		bool Get(NESButton btn)
		{
			return keyMapper.Get(btn) | joystickMapper.Get(btn);
		}

		std::uint8_t GetState()
		{
			return
				(Get(NESButton::A) << 0) |
				(Get(NESButton::B) << 1) |
				(Get(NESButton::Select) << 2) |
				(Get(NESButton::Start) << 3) |
				(Get(NESButton::Up) << 4) |
				(Get(NESButton::Down) << 5) |
				(Get(NESButton::Left) << 6) |
				(Get(NESButton::Right) << 7);
		}

	};

} // namespace nemu