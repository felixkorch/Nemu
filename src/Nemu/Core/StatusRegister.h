// -----------------------------------------------------------------------------------------* C++ *-
// StatusRegister.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

namespace nemu {

class StatusRegister {
    
    template <unsigned N>
    class Bit {
        unsigned &data;

       public:
        constexpr Bit(unsigned &data)
                : data(data)
        {}

        unsigned &operator=(bool value)
        {
            if (value)
                return data |= (1 << N);
            return data &= ~(1 << N);
        }

        constexpr operator bool() const
        {
            return data & (1 << N);
        }
    };

   public:
    unsigned data;
    Bit<0> C;
    Bit<1> Z;
    Bit<2> I;
    Bit<3> D;
    Bit<4> B;
    Bit<5> Unused;
    Bit<6> V;
    Bit<7> N;

    constexpr StatusRegister()
            : data(0)
            , C(data)
            , Z(data)
            , I(data)
            , D(data)
            , B(data)
            , Unused(data)
            , V(data)
            , N(data)
    {}

    constexpr StatusRegister(const StatusRegister& other)
        : data(other.data)
        , C(data)
        , Z(data)
        , I(data)
        , D(data)
        , B(data)
        , Unused(data)
        , V(data)
        , N(data)
    {}

    constexpr StatusRegister(unsigned value)
        : data(value)
        , C(data)
        , Z(data)
        , I(data)
        , D(data)
        , B(data)
        , Unused(data)
        , V(data)
        , N(data)
    {}

    constexpr operator unsigned() const
    {
        return data;
    }

    unsigned &operator=(unsigned newValue)
    {
        return data = newValue;
    }
};

} // namespace nemu