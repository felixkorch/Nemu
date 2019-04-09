// ---------------------------------------------------------------------* C++ *-
// ROMLayout.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/System.h"
#include <cstdint>
#include <vector>

namespace nemu {

/// Decoding of a standard ROM-file.
class ROMLayout {
    using Data = std::vector<std::uint8_t>;

    const Data data;

  public:
    using const_iterator = Data::const_iterator;

    /// Initialize by reading the file at path.
    ROMLayout(const std::string& path)
        : data(ReadFile<Data>(path))
    {}

    unsigned MapperCode() const
    { return (data[7] & 0xF0) | (data[6] >> 4); }

    ppu::MirroringMode MirroringMode() const
    { return data[6] & 1 ? ppu::MirroringMode::Vertical : ppu::MirroringMode::Horizontal; }

    /// Begin iterator of the PRGROM.
    ///
    /// The PRGROM begins after the header at byte 16.
    const_iterator BeginPRGROM() const
    { return std::next(data.cbegin(), 16); }

    /// End iterator of the PRGROM.
    ///
    /// The PRGROM consists of blocks of 16kB each. The amount of blocks needed is located in the
    // header at byte 4.
    const_iterator EndPRGROM() const
    { return std::next(BeginPRGROM(), data[4] * 0x4000); }

    /// Begin iterator of the PRGRAM.
    ///
    /// TODO: Implement
    const_iterator BeginPRGRAM() const
    { return data.cend(); }

    /// End iterator of the PRGRAM.
    ///
    /// TODO: Implement
    const_iterator EndPRGRAM() const
    { return data.cend(); }

    /// Begin iterator of the CHRROM.
    ///
    /// The CHRROM begins where the PRGROM ends.
    const_iterator BeginCHRROM() const
    { return EndPRGROM(); }

    /// End iterator of the CHRROM.
    ///
    /// The CHROM consists of blocks of 8kB each. The amount of blocks needed is located in the
    // header at byte 5.
    const_iterator EndCHRROM() const
    { return std::next(BeginCHRROM(), data[5] * 0x2000); }
};

} // nemu