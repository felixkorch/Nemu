// -----------------------------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -------------------------------------------------------------------------------------------------

#include <vector>
#include <cstddef>

namespace nemu {
namespace mapper {

/// Provides mapping for the NROM-256 cartridge layout.
///
/// PRG-ROM:
///     Bank 0:
///         range: (0x8000, 0xFFFF)
///         size: 0x8000 (16kB)
///         mirroring: None
///
/// CHR-ROM:
///     Page 0:
///         range: (0x0000, 0x0FFF)
///         size: 0x1000 (8kB)
///         mirroring: None
///
class NROM256Mapper {
    using Iterator = std::vector<unsigned>::iterator;
    std::vector<unsigned> prgROM;
    std::vector<unsigned> chrROM;

   public:
    NROM256Mapper() 
        : prgROM(0x8000)
        , chrROM(0x1000)
    {}

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address <= 0x7FFF)
            return 0;
        return prgROM[address];
    }

    void WritePRG(std::size_t address, std::uint8_t value) {}

    std::uint8_t ReadCHR(std::size_t address)
    {
        if (address <= 0x0FFF)
            return chrROM[address];
        return 0;
    }

    void WriteCHR(std::size_t address, std::uint8_t value) {}
};

} // namespace mapper
} // namespace nemu
