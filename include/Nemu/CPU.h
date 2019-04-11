// -----------------------------------------------------------------------------------------* C++ *-
// CPU.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/StatusRegister.h"
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace nemu {

template <class Mapper>
class CPU {
    bool nmi, irq;

    int remainingCycles;

    constexpr static unsigned Flag_C      = (1 << 0);
    constexpr static unsigned Flag_Z      = (1 << 1);
    constexpr static unsigned Flag_I      = (1 << 2);
    constexpr static unsigned Flag_D      = (1 << 3); // Disabled on the NES (decimal mode).
    constexpr static unsigned Flag_B      = (1 << 4); // Bits 4 and 5 are used to indicate whether a
    constexpr static unsigned Flag_Unused = (1 << 5); // Software or hardware interrupt occured
    constexpr static unsigned Flag_V      = (1 << 6);
    constexpr static unsigned Flag_N      = (1 << 7);

    constexpr static unsigned ResetVector = 0xFFFC;
    constexpr static unsigned NMIVector   = 0xFFFA;
    constexpr static unsigned IRQVector   = 0xFFFE;

   public:
    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<Mapper> mapper;

    std::uint8_t   regX;
    std::uint8_t   regY;
    std::uint8_t   regA;
    std::uint8_t   regSP;
    std::uint16_t  regPC;
    StatusRegister regStatus;

    CPU()
        : regX(0)
        , regY(0)
        , regA(0)
        , regSP(0xFF)
        , regPC(0)
        , regStatus()
        , nmi(false)
        , irq(false)
        , remainingCycles(0)
    {}

    std::function<void()> Tick;

    void Reset()
    {
        regSP -= 3;
        Tick(); Tick(); Tick(); Tick();
        remainingCycles = 0;
        nmi = irq = false;
        regStatus.data = 0x04;
        regSP = 0xFF;
        regX  = regY = regA = regPC = 0;
        regPC = Read16(ResetVector);
    }

    void SetNMI()
    {
        nmi = true;
    }

    void SetIRQ()
    {
        irq = true;
    }

    void RunFrame()
    {
        remainingCycles += 29781;

        while (remainingCycles > 0) {
            if (nmi)
                InvokeNMI();
            else if (irq && !regStatus.I)
                InvokeIRQ();
            Execute();
        }
    }

    void StepInstruction() 
    {
        Execute();
    }

    void DecrementCycles()
    {
        remainingCycles--;
    }

    int GetCyclesElapsed()
    {
        return 29781 - remainingCycles;
    }

    /// Write 256 successive bytes to the OAMDMA register.
    void DmaOam(unsigned bank)
    {
        for (int i = 0; i < 256; i++)
            WriteMemory(0x2014, ReadMemory(bank * 0x100 + i));
    }

  private:

      enum class AddressMode {
          Immediate, Absolute,
          AbsoluteX, AbsoluteY,
          _AbsoluteX, _IndirectY,
          Indirect, Relative,
          Zeropage, ZeropageX,
          ZeropageY, IndirectX,
          IndirectY, Implied
      };

      static constexpr unsigned InstructionSizeImmediate = 2;
      static constexpr unsigned InstructionSizeAbsolute  = 3;
      static constexpr unsigned InstructionSizeAbsoluteX = 3;
      static constexpr unsigned InstructionSizeAbsoluteY = 3;
      static constexpr unsigned InstructionSizeIndirect  = 3;
      static constexpr unsigned InstructionSizeRelative  = 2;
      static constexpr unsigned InstructionSizeZeropage  = 2;
      static constexpr unsigned InstructionSizeZeropageX = 2;
      static constexpr unsigned InstructionSizeZeropageY = 2;
      static constexpr unsigned InstructionSizeIndirectX = 2;
      static constexpr unsigned InstructionSizeIndirectY = 2;
      static constexpr unsigned InstructionSizeImplied   = 1;

      unsigned InstructionSize(AddressMode mode)
      {
          switch (mode) {
          case AddressMode::Immediate:  return InstructionSizeImmediate;
          case AddressMode::Absolute:   return InstructionSizeAbsolute;
          case AddressMode::AbsoluteX:  return InstructionSizeAbsoluteX;
          case AddressMode::_AbsoluteX: return InstructionSizeAbsoluteX;
          case AddressMode::AbsoluteY:  return InstructionSizeAbsoluteY;
          case AddressMode::Indirect:   return InstructionSizeIndirect;
          case AddressMode::Relative:   return InstructionSizeRelative;
          case AddressMode::Zeropage:   return InstructionSizeZeropage;
          case AddressMode::ZeropageX:  return InstructionSizeZeropageX;
          case AddressMode::ZeropageY:  return InstructionSizeZeropageY;
          case AddressMode::IndirectX:  return InstructionSizeIndirectX;
          case AddressMode::IndirectY:  return InstructionSizeIndirectY;
          case AddressMode::_IndirectY: return InstructionSizeIndirectY;
          case AddressMode::Implied:    return InstructionSizeImplied;
          default:                      return InstructionSizeImmediate;
          }
      }

      void StackPush(std::uint8_t value)
      {
          Tick();
          mapper->Write(0x0100 | regSP, value);
          regSP--;
      }

      std::uint8_t StackPop()
      {
          Tick();
          regSP++;
          auto temp = mapper->Read(0x0100 | regSP);
          return temp;
      }

      bool HasCrossedPage(std::uint16_t a, std::uint8_t b)
      {
          return ((a + b) & 0xFF00) != ((a & 0xFF00));
      }

      std::uint16_t GetAddressImmediate()
      {
          return regPC + 1;
      }
      std::uint16_t GetAddressAbsolute()
      {
          std::uint16_t address = Read16(regPC + 1);
          return address;
      }
      std::uint16_t _GetAddressAbsoluteX()
      {
          Tick();
          std::uint16_t address = GetAddressAbsolute() + regX;
          return address;
      }

      std::uint16_t GetAddressAbsoluteX()
      {
          std::uint16_t address = GetAddressAbsolute();
          if (HasCrossedPage(address, regX)) Tick();
          return address + regX;
      }

      std::uint16_t GetAddressAbsoluteY()
      {
          std::uint16_t address = GetAddressAbsolute();
          if (HasCrossedPage(address, regY)) Tick();
          return address + regY;
      }
      std::uint16_t GetAddressRelative()
      {
          return regPC + 1;
      }
      std::uint16_t GetAddressIndirectX()
      {
          std::uint16_t lowByte = GetAddressZeropageX();
          return Read16At(lowByte, (lowByte + 1) % 0x100);
      }
      std::uint16_t GetAddressIndirectY()
      {
          std::uint16_t lowByte = GetAddressZeropage();
          std::uint16_t address = Read16At(lowByte, (lowByte + 1) % 0x100) + regY;
          if (HasCrossedPage(address - regY, regY)) Tick();
          return address;
      }
      std::uint16_t _GetAddressIndirectY()
      {
          std::uint16_t lowByte = GetAddressZeropage();
          return Read16At(lowByte, (lowByte + 1) % 0x100) + regY;
      }
      std::uint16_t GetAddressZeropage()
      {
          std::uint8_t address = ReadMemory(regPC + 1);
          return address;
      }

      std::uint16_t GetAddressZeropageX()
      {
          Tick();
          std::uint8_t address = ReadMemory(regPC + 1) + regX;
          return address;
      }
      std::uint16_t GetAddressZeropageY()
      {
          Tick();
          std::uint8_t address = ReadMemory(regPC + 1) + regY;
          return address;
      }

      std::uint16_t GetAddress(AddressMode mode)
      {
          switch (mode) {
          case AddressMode::Immediate:  return GetAddressImmediate();
          case AddressMode::Absolute:   return GetAddressAbsolute();
          case AddressMode::AbsoluteX:  return GetAddressAbsoluteX();
          case AddressMode::_AbsoluteX: return GetAddressAbsoluteX();
          case AddressMode::AbsoluteY:  return GetAddressAbsoluteY();
          case AddressMode::Relative:   return GetAddressRelative();
          case AddressMode::Zeropage:   return GetAddressZeropage();
          case AddressMode::ZeropageX:  return GetAddressZeropageX();
          case AddressMode::ZeropageY:  return GetAddressZeropageY();
          case AddressMode::IndirectX:  return GetAddressIndirectX();
          case AddressMode::IndirectY:  return GetAddressIndirectY();
          case AddressMode::_IndirectY: return GetAddressIndirectY();
          default:                      return GetAddressImmediate();
          }
      }

    void WriteMemory(std::size_t index, unsigned value) 
    {
        Tick();
        mapper->Write(index, value);
    }

    std::uint8_t ReadMemory(std::size_t index) 
    {
        Tick();
        return mapper->Read(index);
    }

    std::uint16_t Read16At(std::size_t low, std::size_t high)
    {
        return ReadMemory(low) | (ReadMemory(high) << 8);
    }

    std::uint16_t Read16(std::size_t index)
    {
        return Read16At(index, index + 1);
    }

    void InvokeNMI()
    {
        Tick(); Tick();
        StackPush(regPC >> 8);
        StackPush(regPC);
        StackPush(regStatus & ~Flag_B);
        regStatus.I = 1;
        regPC = Read16(NMIVector);
        nmi = false;
    }

    void InvokeIRQ()
    {
        Tick(); Tick();
        StackPush(regPC >> 8);
        StackPush(regPC);
        StackPush(regStatus & ~Flag_B);
        regStatus.I = 1;
        regPC = Read16(IRQVector);
        irq = false;
    }

    /// Fetch / Decode / Execute
    void Execute()
    {
        switch (ReadMemory(regPC)) {
        case 0x00: OpBRK();                             break;
        case 0xA0: OpLD(AddressMode::Immediate, regY);  break;
        case 0xA4: OpLD(AddressMode::Zeropage, regY);   break;
        case 0xB4: OpLD(AddressMode::ZeropageX, regY);  break;
        case 0xAC: OpLD(AddressMode::Absolute, regY);   break;
        case 0xBC: OpLD(AddressMode::AbsoluteX, regY);  break;
        case 0xA2: OpLD(AddressMode::Immediate, regX);  break;
        case 0xA6: OpLD(AddressMode::Zeropage, regX);   break;
        case 0xB6: OpLD(AddressMode::ZeropageY, regX);  break;
        case 0xAE: OpLD(AddressMode::Absolute, regX);   break;
        case 0xBE: OpLD(AddressMode::AbsoluteY, regX);  break;
        case 0xEA: OpNOP();                             break;
        case 0x18: OpCLR(Flag_C);                       break;
        case 0xD8: OpCLR(Flag_D);                       break;
        case 0x58: OpCLR(Flag_I);                       break;
        case 0xB8: OpCLR(Flag_V);                       break;
        case 0x38: OpSET(Flag_C);                       break;
        case 0xF8: OpSET(Flag_D);                       break;
        case 0x78: OpSET(Flag_I);                       break;
        case 0xA9: OpLD(AddressMode::Immediate, regA);  break;
        case 0xA5: OpLD(AddressMode::Zeropage, regA);   break;
        case 0xB5: OpLD(AddressMode::ZeropageX, regA);  break;
        case 0xAD: OpLD(AddressMode::Absolute, regA);   break;
        case 0xBD: OpLD(AddressMode::AbsoluteX, regA);  break;
        case 0xB9: OpLD(AddressMode::AbsoluteY, regA);  break;
        case 0xA1: OpLD(AddressMode::IndirectX, regA);  break;
        case 0xB1: OpLD(AddressMode::IndirectY, regA);  break;
        case 0x69: OpADC(AddressMode::Immediate);       break;
        case 0x65: OpADC(AddressMode::Zeropage);        break;
        case 0x75: OpADC(AddressMode::ZeropageX);       break;
        case 0x6D: OpADC(AddressMode::Absolute);        break;
        case 0x7D: OpADC(AddressMode::AbsoluteX);       break;
        case 0x79: OpADC(AddressMode::AbsoluteY);       break;
        case 0x61: OpADC(AddressMode::IndirectX);       break;
        case 0x71: OpADC(AddressMode::IndirectY);       break;
        case 0xE9: OpSBC(AddressMode::Immediate);       break;
        case 0xE5: OpSBC(AddressMode::Zeropage);        break;
        case 0xF5: OpSBC(AddressMode::ZeropageX);       break;
        case 0xED: OpSBC(AddressMode::Absolute);        break;
        case 0xFD: OpSBC(AddressMode::AbsoluteX);       break;
        case 0xF9: OpSBC(AddressMode::AbsoluteY);       break;
        case 0xE1: OpSBC(AddressMode::IndirectX);       break;
        case 0xF1: OpSBC(AddressMode::IndirectY);       break;
        case 0x85: OpST(AddressMode::Zeropage, regA);   break;
        case 0x95: OpST(AddressMode::ZeropageX, regA);  break;
        case 0x8D: OpST(AddressMode::Absolute, regA);   break;
        case 0x9D: OpSTAbsoluteX();                     break;
        case 0x99: OpSTAbsoluteY();                     break;
        case 0x81: OpST(AddressMode::IndirectX, regA);  break;
        case 0x91: OpSTIndirectY();                     break;
        case 0x86: OpST(AddressMode::Zeropage, regX);   break;
        case 0x96: OpST(AddressMode::ZeropageY, regX);  break;
        case 0x8E: OpST(AddressMode::Absolute, regX);   break;
        case 0x84: OpST(AddressMode::Zeropage, regY);   break;
        case 0x94: OpST(AddressMode::ZeropageX, regY);  break;
        case 0x8C: OpST(AddressMode::Absolute, regY);   break;
        case 0x4C: OpJMPAbsolute();                     break;
        case 0x6C: OpJMPIndirect();                     break;
        case 0x20: OpJSR();                             break;
        case 0x48: OpPHA();                             break;
        case 0x08: OpPHP();                             break;
        case 0x68: OpPLA();                             break;
        case 0x28: OpPLP();                             break;
        case 0x40: OpRTI();                             break;
        case 0x60: OpRTS();                             break;
        case 0x10: OpBRA(!regStatus.N);                 break;
        case 0xF0: OpBRA(regStatus.Z);                  break;
        case 0x90: OpBRA(!regStatus.C);                 break;
        case 0xB0: OpBRA(regStatus.C);                  break;
        case 0x30: OpBRA(regStatus.N);                  break;
        case 0xD0: OpBRA(!regStatus.Z);                 break;
        case 0x50: OpBRA(!regStatus.V);                 break;
        case 0x70: OpBRA(regStatus.V);                  break;
        case 0x24: OpBIT(AddressMode::Zeropage);        break;
        case 0x2C: OpBIT(AddressMode::Absolute);        break;
        case 0x88: OpDEY();                             break;
        case 0xCA: OpDEX();                             break;
        case 0xC6: OpDEC(AddressMode::Zeropage);        break;
        case 0xD6: OpDEC(AddressMode::ZeropageX);       break;
        case 0xCE: OpDEC(AddressMode::Absolute);        break;
        case 0xDE: OpDEC(AddressMode::_AbsoluteX);      break;
        case 0x8A: OpTXA();                             break;
        case 0xAA: OpTAX();                             break;
        case 0xA8: OpTAY();                             break;
        case 0xBA: OpTSX();                             break;
        case 0x9A: OpTXS();                             break;
        case 0x98: OpTYA();                             break;
        case 0x0A: OpASLImplied();                      break;
        case 0x06: OpASL(AddressMode::Zeropage);        break;
        case 0x16: OpASL(AddressMode::ZeropageX);       break;
        case 0x0E: OpASL(AddressMode::Absolute);        break;
        case 0x1E: OpASL(AddressMode::_AbsoluteX);      break;
        case 0x4A: OpLSRImplied();                      break;
        case 0x46: OpLSR(AddressMode::Zeropage);        break;
        case 0x56: OpLSR(AddressMode::ZeropageX);       break;
        case 0x4E: OpLSR(AddressMode::Absolute);        break;
        case 0x5E: OpLSR(AddressMode::_AbsoluteX);      break;
        case 0x2A: OpROLImplied();                      break;
        case 0x26: OpROL(AddressMode::Zeropage);        break;
        case 0x36: OpROL(AddressMode::ZeropageX);       break;
        case 0x2E: OpROL(AddressMode::Absolute);        break;
        case 0x3E: OpROL(AddressMode::_AbsoluteX);      break;
        case 0x6A: OpRORImplied();                      break;
        case 0x66: OpROR(AddressMode::Zeropage);        break;
        case 0x76: OpROR(AddressMode::ZeropageX);       break;
        case 0x6E: OpROR(AddressMode::Absolute);        break;
        case 0x7E: OpROR(AddressMode::_AbsoluteX);       break;
        case 0x29: OpAND(AddressMode::Immediate);       break;
        case 0x25: OpAND(AddressMode::Zeropage);        break;
        case 0x35: OpAND(AddressMode::ZeropageX);       break;
        case 0x2D: OpAND(AddressMode::Absolute);        break;
        case 0x3D: OpAND(AddressMode::AbsoluteX);       break;
        case 0x39: OpAND(AddressMode::AbsoluteY);       break;
        case 0x21: OpAND(AddressMode::IndirectX);       break;
        case 0x31: OpAND(AddressMode::IndirectY);       break;
        case 0xC9: OpCMP(AddressMode::Immediate, regA); break;
        case 0xC5: OpCMP(AddressMode::Zeropage, regA);  break;
        case 0xD5: OpCMP(AddressMode::ZeropageX, regA); break;
        case 0xCD: OpCMP(AddressMode::Absolute, regA);  break;
        case 0xDD: OpCMP(AddressMode::AbsoluteX, regA); break;
        case 0xD9: OpCMP(AddressMode::AbsoluteY, regA); break;
        case 0xC1: OpCMP(AddressMode::IndirectX, regA); break;
        case 0xD1: OpCMP(AddressMode::IndirectY, regA); break;
        case 0xE0: OpCMP(AddressMode::Immediate, regX); break;
        case 0xE4: OpCMP(AddressMode::Zeropage, regX);  break;
        case 0xEC: OpCMP(AddressMode::Absolute, regX);  break;
        case 0xC0: OpCMP(AddressMode::Immediate, regY); break;
        case 0xC4: OpCMP(AddressMode::Zeropage, regY);  break;
        case 0xCC: OpCMP(AddressMode::Absolute, regY);  break;
        case 0x09: OpORA(AddressMode::Immediate);       break;
        case 0x05: OpORA(AddressMode::Zeropage);        break;
        case 0x15: OpORA(AddressMode::ZeropageX);       break;
        case 0x0D: OpORA(AddressMode::Absolute);        break;
        case 0x1D: OpORA(AddressMode::AbsoluteX);       break;
        case 0x19: OpORA(AddressMode::AbsoluteY);       break;
        case 0x01: OpORA(AddressMode::IndirectX);       break;
        case 0x11: OpORA(AddressMode::IndirectY);       break;
        case 0x49: OpEOR(AddressMode::Immediate);       break;
        case 0x45: OpEOR(AddressMode::Zeropage);        break;
        case 0x55: OpEOR(AddressMode::ZeropageX);       break;
        case 0x4D: OpEOR(AddressMode::Absolute);        break;
        case 0x5D: OpEOR(AddressMode::AbsoluteX);       break;
        case 0x59: OpEOR(AddressMode::AbsoluteY);       break;
        case 0x41: OpEOR(AddressMode::IndirectX);       break;
        case 0x51: OpEOR(AddressMode::IndirectY);       break;
        case 0xE6: OpINC(AddressMode::Zeropage);        break;
        case 0xF6: OpINC(AddressMode::ZeropageX);       break;
        case 0xEE: OpINC(AddressMode::Absolute);        break;
        case 0xFE: OpINC(AddressMode::_AbsoluteX);      break;
        case 0xE8: OpINX();                             break;
        case 0xC8: OpINY();                             break;
        default: // Illegal opcode
            std::cout << "Error: Illegal op-code" << std::endl;
            std::cin.get();
            break;
        };
    }

    /// Read-Modify-Write Instructions
    // TODO: Dummy writes
    void OpROL(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        unsigned oper = ReadMemory(address);
        Tick();
        oper <<= 1;
        oper = regStatus.C ? oper | 1 : oper & ~1;
        regStatus.C = oper & 0x0100;
        WriteMemory(address, oper & 0xFF);
        UpdateFlagN(oper);
        UpdateFlagZ(oper);
        regPC += InstructionSize(mode);
    }

    void OpROLImplied()
    {
        unsigned temp = regA;
        temp <<= 1;
        temp = regStatus.C ? temp | 1 : temp & ~1;
        regStatus.C = temp & 0x0100;
        regA = temp & 0xFF;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(AddressMode::Implied);
        Tick();
    }

    void OpROR(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        unsigned oper = ReadMemory(address);
        Tick();
        oper = regStatus.C ? oper | 0x0100 : oper & ~0x0100;
        regStatus.C = oper & 1;
        oper >>= 1;
        WriteMemory(address, oper & 0xFF);
        UpdateFlagN(oper);
        UpdateFlagZ(oper);
        regPC += InstructionSize(mode);
    }

    void OpRORImplied()
    {
        unsigned temp = regA;
        temp = regStatus.C ? temp | 0x0100 : temp & ~0x0100;
        regStatus.C = temp & 1;
        temp >>= 1;
        regA = temp & 0xFF;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(AddressMode::Implied);
        Tick();
    }

    void OpASL(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        std::uint8_t oper = ReadMemory(address);
        Tick();
        regStatus.C = oper & 0x80;
        oper <<= 1;
        WriteMemory(address, oper);
        UpdateFlagN(oper);
        UpdateFlagZ(oper);
        regPC += InstructionSize(mode);
    }

    void OpASLImplied()
    {
        regStatus.C = regA & 0x80;
        regA <<= 1;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(AddressMode::Implied);
        Tick();
    }

    void OpLSR(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        std::uint8_t oper = ReadMemory(address);
        Tick();
        regStatus.C = oper & 1;
        oper >>= 1;
        WriteMemory(address, oper);
        regStatus.Z = oper == 0;
        regStatus.N = 0;
        regPC += InstructionSize(mode);
    }

    void OpLSRImplied()
    {
        regStatus.C = regA & 1;
        regA >>= 1;
        regStatus.Z = regA == 0;
        regStatus.N = 0;
        regPC += InstructionSize(AddressMode::Implied);
        Tick();
    }

    void OpDEC(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        std::uint8_t oper = ReadMemory(address);
        Tick();
        oper--;
        WriteMemory(address, oper);
        regStatus.Z = oper == 0;
        regStatus.N = oper & 0x80;
        regPC += InstructionSize(mode);
    }

    void OpINC(AddressMode mode)
    {
        std::uint16_t address = GetAddress(mode);
        std::uint8_t oper = ReadMemory(address);
        Tick();
        oper++;
        WriteMemory(address, oper);
        UpdateFlagN(oper);
        UpdateFlagZ(oper);
        regPC += InstructionSize(mode);
    }

    /// Write Instructions

    void OpST(AddressMode mode, std::uint8_t& reg)
    {
        WriteMemory(GetAddress(mode), reg);
        regPC += InstructionSize(mode);
    }

    // Special Cases
    void OpSTIndirectY()
    {
        Tick();
        WriteMemory(GetAddress(AddressMode::_IndirectY), regA);
        regPC += InstructionSize(AddressMode::_IndirectY);
    }

    void OpSTAbsoluteX()
    {
        Tick();
        WriteMemory(GetAddress(AddressMode::Absolute) + regX, regA);
        regPC += InstructionSize(AddressMode::AbsoluteX);
    }
    void OpSTAbsoluteY()
    {
        Tick();
        WriteMemory(GetAddress(AddressMode::Absolute) + regY, regA);
        regPC += InstructionSize(AddressMode::AbsoluteY);
    }

    /// Read Instructions

    void OpLD(AddressMode mode, std::uint8_t& reg)
    {
        reg = ReadMemory(GetAddress(mode));
        UpdateFlagN(reg);
        UpdateFlagZ(reg);
        regPC += InstructionSize(mode);
    }

    void OpORA(AddressMode mode)
    {
        regA |= ReadMemory(GetAddress(mode));
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(mode);
    }

    void OpBIT(AddressMode mode)
    {
        auto oper = ReadMemory(GetAddress(mode));
        regStatus.N = oper & 0x80;
        regStatus.V = oper & 0x40;
        regStatus.Z = (oper & regA) == 0;
        regPC += InstructionSize(mode);
    }

    void OpADC(AddressMode mode)
    {
        auto oper = ReadMemory(GetAddress(mode));
        unsigned result = regA + oper + regStatus.C;
        bool overflow = !((regA ^ oper) & 0x80) && ((regA ^ result) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result > 0xFF;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
        regPC += InstructionSize(mode);
    }


    void OpSBC(AddressMode mode)
    {
        auto oper = ReadMemory(GetAddress(mode));
        unsigned result = regA - oper - !regStatus.C;
        bool overflow = ((regA ^ result) & 0x80) && ((regA ^ oper) & 0x80);
        regStatus.Z = (result & 0xFF) == 0;
        regStatus.C = result < 0x0100;
        regStatus.V = overflow;
        regStatus.N = result & 0x80;
        regA = result & 0xFF;
        regPC += InstructionSize(mode);
    }

    void OpCMP(AddressMode mode, std::uint8_t& reg)
    {
        auto oper = ReadMemory(GetAddress(mode));
        unsigned result = reg - oper;
        regStatus.Z = reg == oper;
        regStatus.N = result & 0x80;
        regStatus.C = result < 0x0100;
        regPC += InstructionSize(mode);
    }

    void OpAND(AddressMode mode)
    {
        auto oper = ReadMemory(GetAddress(mode));
        regA &= oper;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(mode);
    }

    void OpEOR(AddressMode mode)
    {
        auto oper = ReadMemory(GetAddress(mode));
        regA ^= oper;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC += InstructionSize(mode);
    }

    /// Flow control
    void OpJMPAbsolute()
    {
        regPC = Read16(regPC + 1);
    }

    void OpJMPIndirect()
    {
        std::uint16_t offset = Read16(regPC + 1);
        regPC = Read16At(offset, (offset & 0xFF00) | ((offset + 1) % 0x100));
    }

    void OpBRA(bool condition)
    {
        std::int8_t oper = ReadMemory(GetAddress(AddressMode::Immediate));
        if (condition) Tick();
        regPC += condition ? oper + InstructionSize(AddressMode::Immediate) : InstructionSize(AddressMode::Immediate);
    }

    /// Instructions accessing the stack

    void OpBRK()
    {
        Tick();
        regPC += 2;
        StackPush(regPC >> 8);
        StackPush(regPC);
        StackPush(regStatus | Flag_B);
        regStatus.I = 1;
        regPC = Read16(IRQVector);
    }

    void OpPHA()
    {
        Tick();
        StackPush(regA);
        regPC++;
    }

    void OpPHP()
    {
        Tick();
        StackPush(regStatus | Flag_B | Flag_Unused);
        regPC++;
    }
    void OpPLA()
    {
        Tick(); Tick();
        regA = StackPop();
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC++;
    }
    void OpPLP()
    {
        Tick(); Tick();
        regStatus = StackPop();
        regPC++;
    }
    void OpRTI()
    {
        regStatus = StackPop();
        unsigned PC_low = StackPop();
        unsigned PC_high = StackPop();
        regPC = (PC_high << 8) | PC_low;
    }

    void OpRTS()
    {
        Tick(); Tick();
        unsigned PC_low = StackPop();
        unsigned PC_high = StackPop();
        regPC = ((PC_high << 8) | PC_low) + 1;
        Tick();
    }

    void OpJSR()
    {
        Tick();
        regPC += 2;
        StackPush(regPC >> 8);
        StackPush(regPC);
        regPC = Read16(regPC - 1);
    }

    /// Immediate instructions (No memory access)

    void OpTXA()
    {
        regA = regX;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC++;
        Tick();
    }
    void OpTAX()
    {
        regX = regA;
        UpdateFlagN(regX);
        UpdateFlagZ(regX);
        regPC++;
        Tick();
    }
    void OpTAY()
    {
        regY = regA;
        UpdateFlagN(regY);
        UpdateFlagZ(regY);
        regPC++;
        Tick();
    }
    void OpTSX()
    {
        regX = regSP;
        UpdateFlagN(regX);
        UpdateFlagZ(regX);
        regPC++;
        Tick();
    }
    void OpTXS()
    {
        regSP = regX;
        regPC++;
        Tick();
    }
    void OpTYA()
    {
        regA = regY;
        UpdateFlagN(regA);
        UpdateFlagZ(regA);
        regPC++;
        Tick();
    }

    void OpCLR(unsigned flag)
    {
        regStatus = (unsigned)regStatus & ~flag;
        regPC++;
        Tick();
    }
    void OpSET(unsigned flag)
    {
        regStatus = (unsigned)regStatus | flag;
        regPC++;
        Tick();
    }

    void OpDEX()
    {
        regX--;
        UpdateFlagN(regX);
        UpdateFlagZ(regX);
        regPC++;
        Tick();
    }
    void OpDEY()
    {
        regY--;
        regStatus.Z = regY == 0;
        regStatus.N = regY & 0x80;
        regPC++;
        Tick();
    }

    void OpINX()
    {
        regX++;
        UpdateFlagN(regX);
        UpdateFlagZ(regX);
        regPC++;
        Tick();
    }
    void OpINY()
    {
        regY++;
        UpdateFlagN(regY);
        UpdateFlagZ(regY);
        regPC++;
        Tick();
    }

    void OpNOP()
    {
        regPC++;
        Tick();
    }

    void UpdateFlagN(std::uint8_t oper)
    {
        regStatus.N = oper & 0x80; // If 7th bit is 1, set negative
    }

    void UpdateFlagZ(std::uint8_t oper)
    {
        regStatus.Z = oper == 0;   // If register is 0, set zero
    }

};

} // namespace nemu