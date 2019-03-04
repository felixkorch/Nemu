namespace nemu
{
    class FlagRegister {
        template <int N>
        class Bit {
            int &data;

        public:
            Bit(int &data)
                    : data(data)
            {}

            int &operator=(bool value)
            {
                if (value)
                    return data |= (1 << N);
                return data &= ~(1 << N);
            }

            operator bool() const
            {
                return data & (1 << N);
            }
        };

        int data;

    public:

        Bit<0> c;
        Bit<1> z;
        Bit<2> i;
        Bit<3> d;
        Bit<4> b;
        Bit<5> unused;
        Bit<6> v;
        Bit<7> n;

        FlagRegister()
                : data(0)
                , c(data)
                , z(data)
                , i(data)
                , d(data)
                , b(data)
                , unused(data)
                , v(data)
                , n(data)
        {}

        operator int() const
        {
            return data;
        }

        int &operator=(int newValue)
        {
            return data = newValue;
        }
    };

} // namespace nemu