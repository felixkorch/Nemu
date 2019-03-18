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
		NESKeyMapper* keyMapper = nullptr;
		NESJoystickMapper* joystickMapper = nullptr;
	public:

		void AddKeyboardConfig(const NESKeyMapper& mapper)
		{
			keyMapper = new NESKeyMapper(mapper);
		}

		void AddJoystickConfig(const NESJoystickMapper& mapper)
		{
			joystickMapper = new NESJoystickMapper(mapper);
		}

		bool Get(NESButton btn)
		{
			bool val = false;
			if (keyMapper) val = keyMapper->Get(btn);
			if (joystickMapper && !val) val = joystickMapper->Get(btn);
			return val;
		}

		~NESInput()
		{
			if (keyMapper) delete keyMapper;
			if (joystickMapper) delete joystickMapper;
		}
	};
}