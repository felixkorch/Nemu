#include <cstddef>
#include <iostream>

namespace nemu {
namespace ppu {
enum MirroringMode { Vertical, Horizontal };

class PPUInterface {
   public:
    virtual std::uint8_t ReadRegister(std::size_t index) = 0;
    virtual void WriteRegister(std::size_t index, std::uint8_t val) = 0;
    virtual void Step() = 0;
    virtual void Reset() = 0;
    virtual void SetMirroring(MirroringMode mode) = 0;
};

}  // namespace ppu
}  // namespace nemu