#include <SDL3/SDL_log.h>
#include <assert.h>
#include <cJSON.h>
#include <emulation.h>
#include <instruction.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint32_t operand_table_size = 35;

const struct OperandTable operand_table[] = {
    {"$00", vec}, {"$08", vec}, {"$10", vec}, {"$18", vec}, {"$20", vec},
    {"$28", vec}, {"$30", vec}, {"$38", vec}, {"0", u3},    {"1", u3},
    {"2", u3},    {"3", u3},    {"4", u3},    {"5", u3},    {"6", u3},
    {"7", u3},    {"A", R_A},   {"AF", R_AF}, {"B", R_B},   {"BC", R_BC},
    {"C", R_C},   {"D", R_D},   {"DE", R_DE}, {"E", R_E},   {"H", R_H},
    {"HL", R_HL}, {"L", R_L},   {"NC", NC},   {"NZ", NZ},   {"SP", R_SP},
    {"Z", Z},     {"a16", a16}, {"a8", a8},   {"e8", e8},   {"n16", n16},
    {"n8", n8}};

const char *mnemonics[] = {
    "ADC",        "ADD",        "AND",        "BIT",        "CALL",
    "CCF",        "CP",         "CPL",        "DAA",        "DEC",
    "DI",         "EI",         "HALT",       "ILLEGAL_D3", "ILLEGAL_DB",
    "ILLEGAL_DD", "ILLEGAL_E3", "ILLEGAL_E4", "ILLEGAL_EB", "ILLEGAL_EC",
    "ILLEGAL_ED", "ILLEGAL_F4", "ILLEGAL_FC", "ILLEGAL_FD", "INC",
    "JP",         "JR",         "LD",         "LDH",        "NOP",
    "OR",         "POP",        "PREFIX",     "PUSH",       "RES",
    "RET",        "RETI",       "RL",         "RLA",        "RLC",
    "RLCA",       "RR",         "RRA",        "RRC",        "RRCA",
    "RST",        "SBC",        "SCF",        "SET",        "SLA",
    "SRA",        "SRL",        "STOP",       "SUB",        "SWAP",
    "XOR"};

void log_opcode(const struct Opcode *opcode) {
  SDL_Log("Opcode: \n");
  SDL_Log("  hex: %X \n", opcode->val);
  SDL_Log("  id: %s \n", opcode->id);
  SDL_Log("  prefixed: %b \n", opcode->prefixed);
}

void log_instruction(const struct Instruction *instruction) {

  SDL_Log("\n Instruction: \n");

  SDL_Log("  Opcode: \n");
  SDL_Log("    Hex: %X \n", instruction->opcode->val);
  SDL_Log("    Id: %s \n", instruction->opcode->id);
  SDL_Log("    Prefixed: %b \n", instruction->opcode->prefixed);
  if (instruction->immediate != NULL) {
    SDL_Log("  Immediate: %b", *instruction->immediate);
  }
  SDL_Log("  Mnemoni: %s", instruction->mnemonic);
  SDL_Log("  Bytes: %u", instruction->bytes);
  SDL_Log("  Cycle count: %u", instruction->cycle_count);
  for (uint8_t i = 0; i < instruction->cycle_count; i++) {
    SDL_Log("  Cycle %u: %u", i + 1, instruction->cycles[i]);
  }

  SDL_Log("  Operand count: %u", instruction->operand_count);
  for (uint8_t i = 0; i < instruction->operand_count; i++) {
    struct Operand operand = instruction->operands[i];
    if (operand.bytes != NULL) {
      SDL_Log("    Operand bytes: %u", *operand.bytes);
    }
    if (operand.immediate != NULL) {
      SDL_Log("    Operand immediate: %b", *operand.immediate);
    }
    if (operand.increment != NULL) {
      SDL_Log("    Operand increment: %u", *operand.increment);
    }
  }

  SDL_Log("  Flag:");
  if (instruction->flags != NULL) {
    SDL_Log("    H: %x", instruction->flags->H[0]);
    SDL_Log("    C: %x", instruction->flags->C[0]);
    SDL_Log("    N: %x", instruction->flags->N[0]);
    SDL_Log("    Z: %x", instruction->flags->Z[0]);
  }
}

char *get_operand_id(enum OperandType operand) {
  for (uint32_t i = 0; i < operand_table_size; i++) {
    if (operands_table[i].operand == operand) {
      return operands_table[i].id;
    }
  }

  return NULL;
}

enum OperandType get_operand_val(char *operand_name) {
  for (uint32_t i = 0; i < operand_table_size; i++) {
    if (strncmp(operands_table[i].id, operand_name, strlen(operand_name)) ==
        0) {
      return operands_table[i].operand;
    }
  }

  return NONE;
}

uint8_t get_opcode(uint8_t *rom, struct Opcode *opcode, uint16_t *pc) {
  char *opcode_str = calloc(1, 4);
  if (opcode_str == NULL) {
    SDL_Log("malloc error");
    return -2;
  }

  const uint8_t byte = (rom + *pc)[0];

  uint16_t count = sprintf(opcode_str, "0x%02X", (uint8_t)byte);
  if (count != 4) {
    printf("prefix opcode error");
    return -4;
  }
  opcode->val = byte;
  opcode->id = opcode_str;

  if (opcode->val == 0xCB) {
    free(opcode_str);
    opcode->id = NULL;
    opcode->prefixed = true;
    *pc += 1;
    const uint8_t err = get_opcode(rom, opcode, pc);

    if (err > 0) {
      printf("prefix opcode error");
      return -4;
    }
  }

  return 0;
}

int serialize_instruction(struct Instruction *instruction, cJSON *json) {
  cJSON *bytes = cJSON_GetObjectItem(json, "bytes");
  instruction->bytes = (uint8_t)cJSON_GetNumberValue(bytes);

  cJSON *cycles = cJSON_GetObjectItem(json, "cycles");
  uint8_t cycle_count = cJSON_GetArraySize(cycles);

  instruction->cycles = malloc(sizeof(uint8_t) * cycle_count);
  instruction->cycle_count = cycle_count;

  for (int i = 0; i < cycle_count; i++) {
    uint8_t cycle = (uint8_t)cJSON_GetNumberValue(cycles->child);
    instruction->cycles[i] = cycle;
    cycles = cycles->next;
  }

  cJSON *mnemonic = cJSON_GetObjectItem(json, "mnemonic");
  instruction->mnemonic = cJSON_GetStringValue(mnemonic);

  cJSON *immediate = cJSON_GetObjectItem(json, "immediate");
  if (immediate != NULL) {
    bool imme = (bool)cJSON_GetNumberValue(immediate);
    instruction->immediate = &imme;
  }

  cJSON *operands = cJSON_GetObjectItem(json, "operand");
  if (operands != NULL) {
    uint8_t operand_count = cJSON_GetArraySize(operands);
    instruction->operands = malloc(sizeof(struct Operand) * operand_count);
    instruction->operand_count = operand_count;

    for (int i = 0; i < operand_count; i++) {
      cJSON *j_name = cJSON_GetObjectItem(operands, "name");
      char *name = "";
      if (j_name != NULL) {
        name = cJSON_GetStringValue(j_name);
      }

      cJSON *j_immediate = cJSON_GetObjectItem(operands, "immediate");
      assert(j_immediate != NULL);
      bool immediate = (bool)cJSON_GetNumberValue(j_immediate);
      cJSON *j_bytes = cJSON_GetObjectItem(operands, "bytes");
      uint8_t *bytes = NULL;
      if (j_bytes != NULL) {
        uint8_t b = (uint8_t)cJSON_GetNumberValue(j_immediate);
        bytes = &b;
      }

      cJSON *j_increment = cJSON_GetObjectItem(operands, "increment");
      int8_t *increment = NULL;
      if (j_increment != NULL) {
        int8_t i = (int8_t)cJSON_GetNumberValue(j_immediate);
        increment = &i;
      }

      cJSON *j_decrement = cJSON_GetObjectItem(operands, "decrement");
      if (j_decrement != NULL) {
        int8_t d = (int8_t)-cJSON_GetNumberValue(j_immediate);
        increment = &d;
      }

      enum OperandType type = get_operand_val(name);
      struct Operand operand = {&immediate, bytes, increment, NULL};

      instruction->operands[i] = operand;
      operands = operands->next;
    };
  }

  cJSON *flags = cJSON_GetObjectItem(json, "flags");
  if (flags == NULL) {
    return 0;
  }

  instruction->flags = malloc(sizeof(struct Flags));

  cJSON *Z = cJSON_GetObjectItem(flags, "Z");
  cJSON *N = cJSON_GetObjectItem(flags, "N");
  cJSON *H = cJSON_GetObjectItem(flags, "H");
  cJSON *C = cJSON_GetObjectItem(flags, "C");

  instruction->flags->Z = cJSON_GetStringValue(Z);
  instruction->flags->N = cJSON_GetStringValue(N);
  instruction->flags->H = cJSON_GetStringValue(H);
  instruction->flags->C = cJSON_GetStringValue(C);

  assert(instruction->flags->Z != NULL);
  assert(instruction->flags->N != NULL);
  assert(instruction->flags->H != NULL);
  assert(instruction->flags->Z != NULL);

  return 0;
}

int cpu_step(struct CPU *cpu, cJSON *json) {
  struct Opcode opcode = {false, 0, NULL};

  /*uint8_t *rom = malloc(0x3FFF * 8);*/
  /*get_bytes(cpu->bus.memory, 0x0, 0x3FFF * 8, rom);*/

  uint8_t err = get_opcode(cpu->bus.memory, &opcode, &cpu->registers.PC);
  if (err > 0) {
    SDL_LogError(0, "opcode error");
    return -4;
  }

  if (opcode.prefixed == true) {
    json = cJSON_GetObjectItem(json, "cbprefixed");
  } else {
    json = cJSON_GetObjectItem(json, "unprefixed");
  }

  json = cJSON_GetObjectItem(json, opcode.id);
  struct Instruction instruction = {NULL, NULL, NULL, 0,      0,
                                    0,    0,    NULL, &opcode};

  err = serialize_instruction(&instruction, json);
  if (err > 0) {
    SDL_Log("Serialize error");
  }

  log_instruction(&instruction);
  SDL_Log("###################### \n");

  if (strncmp(instruction.mnemonic, "LD", 2) == 0) {
    ld(&instruction, cpu);
  }

  if (strncmp(instruction.mnemonic, "LDH", 3) == 0) {
    ldh(&instruction, cpu);
  }

  if (strncmp(instruction.mnemonic, "JP", 2) == 0) {
    assert(instruction.operand_count <= 2);
    // ldh(&instruction, cpu);
  }

  switch (instruction.opcode->val) {
  case 0xE9:
    // jump(EMPTY, cpu, cpu->registers.HL.full);
    break;
  default:
    // STOP
    // noop
    break;
  }
  cpu->registers.PC += instruction.bytes;

  return 0;
}
