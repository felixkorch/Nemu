namespace nemu
{
	namespace cpu
	{
		class CPUInterface {
		public:
			virtual void SetNMI() = 0;
			virtual void SetIRQ() = 0;
			virtual void Execute() = 0;
			virtual void Reset() = 0;
		};

	}
}