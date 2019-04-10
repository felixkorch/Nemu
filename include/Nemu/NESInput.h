// -----------------------------------------------------------------------------------------* C++ *-
// NESInput.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <Sgl.h>
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

using namespace sgl;


class NESKeyMapper {
    UnorderedMap<NESButton, int> map;
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

    int axis;
    Value val;
    float threshhold;

    AxisConfig() :
        axis(),
        val(),
        threshhold()
    {}

    AxisConfig(int axis, Value val, float threshhold = 0.1) :
        axis(axis),
        val(val),
        threshhold(threshhold)
    {}

};

class NESJoystickMapper {

    UnorderedMap<NESButton, int> keyMap;
    UnorderedMap<NESButton, AxisConfig> axisMap;
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

} // namespace nemu