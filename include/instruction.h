#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <emulation.h>

void ld(struct Instruction *instruction, struct CPU *cpu);
void ldh(struct Instruction *instruction, struct CPU *cpu);
void jp(struct Instruction *instruction, struct CPU *cpu);

#endif
