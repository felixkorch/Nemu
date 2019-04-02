#pragma once
#include <Sgl.h>
#include <unordered_map>

namespace nemu
{
	enum class NESButton {
		A, B, Left, Right, Up, Down, Start, Select
	};

	using namespace sgl;


	class NESKeyMapper {
	private:
		std::unordered_map<NESButton, int> map;
	public:

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
			return Input::IsKeyPressed(map[btn]);
		}
	};



	struct AxisConfig {
		enum class Value {
			Negative, Positive
		};
		static constexpr auto threshhold = 0.1;
		int axis;
		Value val;
	};

	class NESJoystickMapper {
	private:
		std::unordered_map<NESButton, int> keyMap;
		std::unordered_map<NESButton, AxisConfig> axisMap;
	public:

		void Reset()
		{
			keyMap.clear();
			axisMap.clear();
		}

		void MapKey(NESButton from, int to)
		{
			keyMap[from] = to;
			axisMap.erase(from);
		}

		void MapAxis(NESButton from, AxisConfig axis)
		{
			axisMap[from] = axis;
			keyMap.erase(from);
		}

		bool Get(NESButton btn)
		{
			if (keyMap.count(btn) != 0)
				return Input::IsJoystickButtonPressed(keyMap[btn], 0);

			if (axisMap.count(btn) != 0) {
				const AxisConfig conf = axisMap[btn];
				if (!Input::IsJoystickPresent(0)) return false;
				const auto axes = Input::GetJoystickAxis(0);
				if (conf.axis > axes.size() - 1) return false;
				return conf.val == AxisConfig::Value::Negative ? axes[conf.axis] <= -conf.threshhold : axes[conf.axis] >= conf.threshhold;
			}

			return false;
		}
	};

	class NESInput {
		NESKeyMapper keyMapper;
		NESJoystickMapper joystickMapper;
		bool keyMapperExists = false;
		bool joystickMapperExists = false;
	public:

		void AddKeyboardConfig(const NESKeyMapper& mapper)
		{
			keyMapper = NESKeyMapper(mapper);
			keyMapperExists = true;
		}

		void AddJoystickConfig(const NESJoystickMapper& mapper)
		{
			joystickMapper = NESJoystickMapper(mapper);
			joystickMapperExists = true;
		}

		bool Get(NESButton btn)
		{
			bool val = false;
			if (keyMapperExists)
				val = keyMapper.Get(btn);

			if (joystickMapperExists && !val)
				val = joystickMapper.Get(btn);
			return val;
		}

		std::uint8_t GetState()
		{
			return
				(Get(NESButton::A)      << 0) |
				(Get(NESButton::B)      << 1) |
				(Get(NESButton::Select) << 2) |
				(Get(NESButton::Start)  << 3) |
				(Get(NESButton::Up)     << 4) |
				(Get(NESButton::Down)   << 5) |
				(Get(NESButton::Left)   << 6) |
				(Get(NESButton::Right)  << 7);
		}

	};
}