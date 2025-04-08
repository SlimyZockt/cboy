#include "SDL3/SDL_log.h"
#include "memory.h"
#include <assert.h>
#include <cJSON.h>
#include <emulation.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool is_condition_met(const enum OperandType condition, struct CPU *cpu) {
  switch (condition) {
  case C:
    return ((cpu->registers.AF.F << 4) & 1) == 1;
  case NC:
    return ((cpu->registers.AF.F << 4) & 1) == 0;
  case Z:
    return ((cpu->registers.AF.F << 7) & 1) == 1;
  case NZ:
    return ((cpu->registers.AF.F << 7) & 1) == 0;
  default:
    assert(true);
    return true;
  }
}

bool is_condition(const enum OperandType operand) {
  switch (operand) {
  case C:
  case NC:
  case Z:
  case NZ:
    return true;
  default:
    return false;
  }

  return false;
}

void jump(const enum OperandType condition, struct CPU *cpu,
          const uint16_t n16) {
  if (condition == NONE || is_condition_met(condition, cpu)) {
    cpu->registers.PC = n16;
    return;
  }
}

void jump_relative(const enum OperandType condition, struct CPU *cpu,
                   const void *n16) {
  if (condition == NONE || is_condition_met(condition, cpu)) {
    cpu->registers.PC += *(int8_t *)n16;
    return;
  }
}

void push(struct CPU *cpu, uint16_t *val) {
  cpu->registers.SP += 0x02;
  cpu->bus.memory[cpu->registers.SP] = *val;
}

void rst(struct CPU *cpu, uint8_t id) {
  push(cpu, &cpu->registers.PC);
  SDL_Log("%d", id);
}

uint8_t *get_reg8(struct Registers *registers, enum OperandType reg_type) {
  switch (reg_type) {
  case R_A:
    return &registers->AF.A;
  case R_B:
    return &registers->BC.B;
  case R_C:
    return &registers->BC.C;
  case R_D:
    return &registers->DE.D;
  case R_E:
    return &registers->DE.E;
  case R_H:
    return &registers->HL.H;
  case R_L:
    return &registers->HL.L;
  default:
    return NULL;
  }
}

uint16_t *get_reg16(struct Registers *registers, enum OperandType reg_type) {
  switch (reg_type) {
  case R_BC:
    return &registers->BC.full;
  case R_DE:
    return &registers->DE.full;
  case R_HL:
    return &registers->HL.full;
  default:
    return NULL;
  }

  return NULL;
}

void ld(struct Instruction *instruction, struct CPU *cpu) {
  assert(instruction->operand_count == 2);
  struct Operand operand1 = instruction->operands[0];
  struct Operand operand2 = instruction->operands[1];

  if (operand1.increment != NULL) {
    if (operand1.type == R_HL && operand2.type == R_A) {
      cpu->registers.AF.A = get_mem(cpu->registers.HL.full, &cpu->bus);
      cpu->registers.HL.full += *instruction->operands->increment;
      return;
    }
    assert(operand1.type == R_HL && operand2.type == R_A);
    set_mem(cpu->registers.HL.full, cpu->registers.AF.A, &cpu->bus);
    cpu->registers.HL.full += *instruction->operands->increment;
    return;
  }

  // r8, r8'
  if (operand1.type == r8 && operand2.type == r8) {
    *get_reg8(&cpu->registers, operand1.type) =
        *get_reg8(&cpu->registers, operand2.type);
  }

  // r8, n8
  if (operand1.type == r8 && operand2.type == n8) {
    *get_reg8(&cpu->registers, operand1.type) =
        get_mem((cpu->registers.PC + *operand2.bytes), &cpu->bus);
    return;
  }

  // r16, n16
  if (operand1.type == r16 && operand2.type == n16) {
    *get_reg16(&cpu->registers, operand1.type) =
        get_mem_16(cpu->registers.PC + *operand2.bytes, &cpu->bus);
    return;
  }

  // [HL], r8
  if (operand1.type == R_HL && operand2.type == r8) {
    set_mem(cpu->registers.HL.full, *get_reg8(&cpu->registers, operand2.type),
            &cpu->bus);
    return;
  }

  // [HL], n8
  if (operand1.type == R_HL && operand2.type == n8) {
    set_mem(cpu->registers.HL.full,
            get_mem(cpu->registers.PC + *operand2.bytes, &cpu->bus), &cpu->bus);
    return;
  }

  // r8, [HL]
  if (operand1.type == r8 && operand2.type == R_HL) {
    *get_reg8(&cpu->registers, operand1.type) =
        get_mem(cpu->registers.HL.full, &cpu->bus);

    return;
  }

  // r8, [HL]
  if (operand1.type == r16 && operand2.type == R_A) {
    set_mem(cpu->registers.BC.full, cpu->registers.AF.A, &cpu->bus);
    return;
  }

  // [n16], A
  if (operand1.type == n16 && operand2.type == R_A) {
    set_mem(get_mem_16(cpu->registers.PC + 1, &cpu->bus), cpu->registers.AF.A,
            &cpu->bus);
    return;
  }

  // A, [n16]
  if (operand1.type == R_A && operand2.type == n16) {
    assert(operand1.bytes != NULL);
    cpu->registers.AF.A = get_mem(
        get_mem_16(cpu->registers.PC + *operand2.bytes, &cpu->bus), &cpu->bus);
  }

  // A, [n16]
  assert(operand1.type == R_A && operand2.type == r16);
  cpu->registers.AF.A =
      get_mem(*get_reg16(&cpu->registers, operand2.type), &cpu->bus);
}

void ldh(struct Instruction *instruction, struct CPU *cpu) {
  assert(instruction->operand_count == 2);
  struct Operand operand1 = instruction->operands[0];
  struct Operand operand2 = instruction->operands[1];

  if (operand1.type == n16 && operand2.type == R_A) {
    uint16_t target = get_mem_16(
        0xFF00 + cpu->bus.memory[cpu->registers.PC + *operand1.bytes],
        &cpu->bus);

    return;
  }
  if (operand1.type == R_C && operand2.type == R_A) {
    uint16_t target =
        get_mem_16(0xFF00 + cpu->bus.memory[cpu->registers.BC.C], &cpu->bus);

    // load(&target, &cpu->registers.AF.A);
    return;
  }
  if (operand1.type == R_C && operand2.type == n16) {
    uint16_t val = get_mem_16(
        0xFF00 + cpu->bus.memory[cpu->registers.PC + *operand1.bytes],
        &cpu->bus);

    // load(&cpu->registers.A, &val);
    return;
  }
  if (operand1.type == R_A && operand2.type == R_C) {
    uint16_t val =
        get_mem_16(0xFF00 + cpu->bus.memory[cpu->registers.BC.C], &cpu->bus);
    // load(&cpu->registers.A, &val);
    return;
  }
}

void jp(struct Instruction *instruction, struct CPU *cpu) {
  struct Operand operand1 = instruction->operands[0];

  if (instruction->operand_count == 1) {
    switch (operand1.type) {
    case R_HL:
      return jump(NONE, cpu, cpu->bus.memory[cpu->registers.PC]);
    case n16:
      return jump(NONE, cpu, cpu->bus.memory[cpu->registers.PC]);
    default:
      return;
    }
    return;
  }

  assert(instruction->operand_count == 2);
  jump(operand1.type, cpu, cpu->bus.memory[cpu->registers.PC]);
  return;
}

void jr(struct Instruction *instruction, struct CPU *cpu) {
  struct Operand operand1 = instruction->operands[0];

  if (instruction->operand_count == 1) {
    return jump_relative(NONE, cpu, &cpu->bus.memory[cpu->registers.PC]);
  }

  assert(instruction->operand_count == 2);
  jump_relative(operand1.type, cpu, &cpu->bus.memory[cpu->registers.PC]);
  return;
}

void call(struct Instruction *instruction, struct CPU *cpu) {
  struct Operand operand1 = instruction->operands[0];

  if (is_condition(operand1.type) && !is_condition_met(operand1.type, cpu)) {
    return;
  }

  uint16_t n16 = get_mem_16(cpu->registers.PC + 1, &cpu->bus);
  push_sp_16(&cpu->registers.SP, n16, &cpu->bus);
  cpu->registers.PC = n16;

  cpu->bus.memory[cpu->registers.SP] = 3;
}

void adc(struct Instruction *instruction, struct CPU *cpu) {
  struct Operand operand1 = instruction->operands[0];
}
