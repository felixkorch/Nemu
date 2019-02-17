#include "pch.h"
#include "Core/CPU.h"
//#include "SFML/Window.hpp"

int main()
{
	std::uint16_t A = 20;
	std::uint16_t B = 30;
	std::uint8_t A_LOW = A & 0xff;
	std::uint8_t A_HIGH = A >> 8;
	std::uint8_t B_LOW = B & 0xff;
	std::uint8_t B_HIGH = B >> 8;

	std::vector<std::uint8_t> program = {
		0xA9, A_LOW,  // lda TAL1_LO      ; L�gg l�gsta byten av f�rsta talet i ackumulatorregistret
		0x18,         // clc              ; Rensa carry-flaggan innan addition
		0x69, B_LOW,  // adc TAL2_LO      ; Addera ackumulatorv�rdet med l�gsta byten i andra talet
		0x8D, 0, 0,   // sta RESULTAT_LO  ; Lagra nya ackumulatorv�rdet i l�gsta byten av resultatet
		0xA9, A_HIGH, // lda TAL1_HI      ; L�gg h�gsta byten av f�rsta talet i ackumulatorn
		0x69, B_HIGH, // adc TAL2_HI      ; Addera, men rensa inte carry-flaggan denna g�ng
		0x8D, 1, 0    // sta RESULTAT_HI  ; Lagra ackumulatorn i h�gsta byten av resultatet
	};

	CPU cpu;
	cpu.LoadProgram(program);

	auto ram = cpu.GetRAM();
	std::cout << A << " + " << B << " = " << *(std::uint16_t*)ram.data() << std::endl;

	std::cin.get();
}