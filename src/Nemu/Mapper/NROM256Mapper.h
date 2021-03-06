// -----------------------------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -------------------------------------------------------------------------------------------------

#include <vector>
#include <cstddef>

namespace nemu::mapper {

/// Provides mapping for the NROM-256 cartridge layout.
///
/// Mapping for PRG:
///     Slot 0:
///         range: (0x8000, 0xFFFF)
///         size: 0x4000 (16kB)
///
/// Mapping for CHR:
///     Slot 0:
///         range: (0x0000, 0x1FFF)
///         size: 0x2000 (8kB)
///
class NROM256Mapper {
    std::vector<unsigned> prgROM, prgRAM, chrRAM;

public:

    std::shared_ptr<PPU<PPUMapper<NROM256Mapper>>> ppu;
    std::shared_ptr<CPU<CPUMapper<NROM256Mapper>>> cpu;

    NROM256Mapper(std::vector<unsigned>&& prg, std::vector<unsigned>&& chr)
        : prgROM(std::move(prg))
        , prgRAM(0x2000)
        , chrRAM(std::move(chr))
    {
		if (chrRAM.size() == 0)
			chrRAM = std::vector<unsigned>(0x2000);
    }

    void Update()
    {

    }

	void SetPRGROM(std::vector<unsigned>&& newData)
	{
		prgROM = std::move(newData);
	}

	void SetCHRROM(std::vector<unsigned>&& newData)
	{
		chrRAM = std::move(newData);
	}

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address < 0x6000)
            return 0;
        if (address <= 0x7FFF)
            return prgRAM[address & 0x2000];
        return prgROM[address % 0x8000];
    }

    void WritePRG(std::size_t address, std::uint8_t value)
    {
        if (address >= 0x6000 && address <= 0x7FFF)
            prgRAM[address % 0x2000] = value;
    }

    std::uint8_t ReadCHR(std::size_t address)
    {
        return chrRAM[address];
    }

    void WriteCHR(std::size_t address, std::uint8_t value)
    {
        chrRAM[address] = value;
    }

    void OnScanline() {}
};

} // namespace mapper