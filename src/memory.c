#include "memory.h"
#include <stdint.h>

void push_sp(Address *sp, Word val, struct MemoryBus *bus) {
  bus->memory[*sp] = val;
  *sp -= 0x1;
};

Word pop_sp(Address *sp, struct MemoryBus *bus) {
  Word val = bus->memory[*sp];
  *sp += 0x1;
  return val;
};

void push_sp_16(Address *sp, uint16_t val, struct MemoryBus *bus) {
  uint8_t high = (val >> 8) & 0xFF;
  uint8_t low = val & 0xFF;

  bus->memory[*sp] = high;
  bus->memory[*sp - 1] = low;
  *sp -= 0x2;
};

uint16_t pop_sp_16(Address *sp, struct MemoryBus *bus) {
  uint8_t low = bus->memory[(*sp - 1) * WORD_SIZE];
  uint8_t high = bus->memory[*sp * WORD_SIZE];

  *sp += 0x2;
  return (low) + (high << 8);
};

Word get_mem(const Address address, struct MemoryBus *bus) {
  return bus->memory[address * WORD_SIZE];
};

void set_mem(Address address, const Word val, struct MemoryBus *bus) {
  bus->memory[address * WORD_SIZE] = val;
};

uint16_t get_mem_16(const Address address, struct MemoryBus *bus) {
  uint8_t low = bus->memory[address * WORD_SIZE];
  uint8_t high = bus->memory[(address + 1) * WORD_SIZE];

  return (low) + (high << 8);
};

void set_mem_16(Address address, const uint16_t val, struct MemoryBus *bus) {
  uint8_t high = (val >> 8) & 0xFF;
  uint8_t low = val & 0xFF;

  bus->memory[address * WORD_SIZE] = low;
  bus->memory[(address + 1) * WORD_SIZE] = high;
}
