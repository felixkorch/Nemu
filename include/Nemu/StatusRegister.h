namespace nemu
{
    class StatusRegister {
	private:

        template <int N>
        class Bit {
            int &data;

        public:
            constexpr Bit(int &data)
                    : data(data)
            {}

            int &operator=(bool value)
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

	private:
		int data;
    public:
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

        constexpr operator int() const
        {
            return data;
        }

		int &operator=(int newValue)
        {
            return data = newValue;
        }
    };

} // namespace nemu