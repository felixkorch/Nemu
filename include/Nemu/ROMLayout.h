// ---------------------------------------------------------------------* C++ *-
// ROMLayout.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/System.h"
#include "Nemu/PPU.h"
#include <cstdint>
#include <vector>
#include <iostream>

namespace nemu {

/// Decoding of a standard ROM-file.
class ROMLayout {
    std::vector<std::uint8_t> data;

   public:
    using const_iterator = decltype(data)::const_iterator;
    using iterator = decltype(data)::iterator;

    /// Initialize by reading the file at path.
    ROMLayout(const std::string& path)
        : data(ReadFile<decltype(data)>(path))
    {}

    unsigned MapperCode() const
    { return (data[7] & 0xF0) | (data[6] >> 4); }

    void SetMapperCode(unsigned code)
    {
        data[6] &= 0x0F;
        data[6] |= (code << 4);
        data[7] &= 0x0F;
        data[7] |= (code & 0xF0);
    }    

    ppu::MirroringMode MirroringMode() const
    { return data[6] & 1 ? ppu::MirroringMode::Vertical : ppu::MirroringMode::Horizontal; }

    void SetMirroringMode(ppu::MirroringMode mode)
    {
        data[6] &= ~1;
        if (mode == ppu::MirroringMode::Vertical)
            data[6] |= 1;
    }

    /// Begin iterator of the PRGROM.
    ///
    /// The PRGROM begins after the header at byte 16.
    const_iterator BeginPRGROM() const
    { return std::next(data.cbegin(), 16); }

    /// Mutable begin iterator of the PRGROM.
    iterator MutableBeginPRGROM()
    { return std::next(data.begin(), 16); }

    /// End iterator of the PRGROM.
    ///
    /// The PRGROM consists of blocks of 16kB each. The amount of blocks needed is located in the
    // header at byte 4.
    const_iterator EndPRGROM() const
    { return std::next(BeginPRGROM(), data[4] * 0x4000); }

    /// Mutable end iterator of the PRGROM.
    iterator MutableEndPRGROM()
    { return std::next(MutableBeginPRGROM(), data[4] * 0x4000); }

    /// Begin iterator of the CHRROM.
    ///
    /// The CHRROM begins where the PRGROM ends.
    const_iterator BeginCHRROM() const
    { return EndPRGROM(); }

    /// Mutable begin iterator of the CHRROM.
    iterator MutableBeginCHRROM()
    { return MutableEndPRGROM(); }

    /// End iterator of the CHRROM.
    ///
    /// The CHROM consists of blocks of 8kB each. The amount of blocks needed is located in the
    // header at byte 5.
    const_iterator EndCHRROM() const
    { return std::next(BeginCHRROM(), data[5] * 0x2000); }

    /// Mutable end iterator of the CHRROM
    iterator MutableEndCHRROM()
    { return std::next(MutableBeginCHRROM(), data[5] * 0x2000); }
};

template <class CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& stream, const ROMLayout& rom) 
{
    return stream << "ROMLayout: {\n"
                  << "    MapperCode: " << rom.MapperCode() << ",\n"
                  << "    MirroringMode: " << (rom.MirroringMode() == ppu::MirroringMode::Horizontal
                                                  ? "horizontal,\n"
                                                  : "vertical,\n")
                  << "    PRGROMSize: " << (rom.EndPRGROM() - rom.BeginPRGROM()) << ",\n"
                  << "    CHRROMSize: " << (rom.EndCHRROM() - rom.BeginCHRROM()) << '\n'
                  << "}";
}

} // nemu