#ifndef MEMORY_H
#define MEMORY_H

#include "emulation.h"
#include <stdint.h>
#define WORD_SIZE 8
#define ADREES_BUS_SIZE = 16
#define MEM_SIZE 0xFFFF * WORD_SIZE

typedef uint16_t Address;
typedef uint8_t Word;

void push_sp(Address *sp, const Word val, struct MemoryBus *bus);
Word pop_sp(Address *sp, struct MemoryBus *bus);

void push_sp_16(Address *sp, const uint16_t val, struct MemoryBus *bus);
uint16_t pop_sp_16(Address *sp, struct MemoryBus *bus);

Word get_mem(const Address address, struct MemoryBus *bus);
void set_mem(const Address address, const Word val, struct MemoryBus *bus);

uint16_t get_mem_16(const Address address, struct MemoryBus *bus);
void set_mem_16(const Address address, const uint16_t val,  struct MemoryBus *bus);
#endif
