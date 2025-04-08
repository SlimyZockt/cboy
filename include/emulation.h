#ifndef EMULATION_H
#define EMULATION_H
#include <cJSON.h>
#include <stdbool.h>
#include <stdint.h>

union Register {
  uint16_t full;
  struct {
  };
  uint8_t half[2];
};

struct Registers {
  union {
    uint16_t full;
    struct {
      uint8_t A;
      uint8_t F;
  };
  } AF;
  union {
    uint16_t full;
    struct {
      uint8_t C;
      uint8_t B;
  };
  } BC;
  union {
    uint16_t full;
    struct {
      uint8_t E;
      uint8_t D;
  };
  } DE;
  union {
    uint16_t full;
    struct {
      uint8_t L;
      uint8_t H;
  };
  } HL;
  uint16_t SP;
  uint16_t PC;
};

struct MemoryBus {
  uint8_t *memory;
};

struct CPU {
  struct Registers registers;
  struct MemoryBus bus;
};

struct RAM {
  uint16_t size;
  void *buffer;
};

struct File {
  void *data;
  uint32_t size;
};

enum OperandType {
  NONE = 0,
  r8 = 1 << 0,
  r16 = 1 << 1,
  n8 = 1 << 2,
  n16 = 1 << 3,
  e8 = 1 << 4,
  u3 = 1 << 5,
  cc = 1 << 6,
  vec = 1 << 7,
  R_A = 1 << 8,
  R_B = 1 << 9,
  R_C = 1 << 10,
  R_D = 1 << 11,
  R_E = 1 << 12,
  R_H = 1 << 13,
  R_L = 1 << 14,
  R_BC = 1 << 15,
  R_DE = 1 << 16,
  R_HL = 1 << 17,
  Z = 1 << 18,
  NZ = 1 << 19,
  C = 1 << 20,
  NC = 1 << 21,
  a8 = 1 << 22,
  a16 = 1 << 23,
  R_SP = 1 << 24,
  R_AF = 1 << 25,
};


struct Instruction {
  char *mnemonic;
  bool *immediate;
  uint8_t *cycles;
  uint8_t bytes;
  uint8_t cycle_count;
  uint8_t operand_count;
  struct Operand {
    enum OperandType type;
    bool *immediate;
    uint8_t *bytes;
    int8_t *increment;
    void *address;
  } *operands;
  struct Flags {
    char *Z;
    char *N;
    char *H;
    char *C;
  } *flags;
  struct Opcode {
    bool prefixed;
    uint8_t val;
    char *id;
  } *opcode;
};
struct OperandTable {
  char *id;
  enum OperandType operand;
};

/*void log_opcode(const struct Opcode *opcode);*/
/*void log_instruction(const struct Instruction *instruction);*/
/*uint8_t get_opcode(struct File *rom, struct Opcode *opcode, uint32_t
 * *read_index);*/
/*int serialize_instruction(struct Instruction *instruction, cJSON *json);*/
/*void jump(struct CPU *cpu, const enum Conditions *condition, const uint16_t
 * n16);*/
int cpu_step(struct CPU *cpu, cJSON *json);

#endif
