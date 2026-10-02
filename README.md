# 16-bit Single-Cycle RISC CPU

A custom 16-bit single-cycle RISC CPU designed and implemented from scratch
using Logisim-Evolution.

The project includes the CPU datapath, control unit, register file, ALU,
instruction/data memory, a custom instruction set, and a C-based assembler
for converting assembly programs into Logisim-compatible machine code.

---

## Overview

### CPU Specifications

| Feature | Specification |
|---|---|
| Architecture | 16-bit RISC |
| Datapath | 16-bit |
| Execution | Single-cycle |
| Registers | 16 × 16-bit |
| Register names | R0–R15 |
| Instruction memory | 64K × 16-bit |
| Data memory | 64K × 16-bit |
| Memory architecture | Harvard |
| ALU operations | 8 |
| HDL / Simulator | Logisim-Evolution |

The CPU uses separate instruction ROM and data RAM and completes one
instruction in a single clock cycle.

---

## Architecture

The CPU consists of the following major components:

- Program Counter (PC)
- Instruction ROM
- Instruction Decoder / Control Unit
- Register File
- 16-bit ALU
- Data RAM
- Multiplexers
- Sign Extension Logic
- Control and datapath logic

The complete CPU circuit is implemented in Logisim-Evolution.

---

## Instruction Set

The processor uses a custom 16-bit instruction set with 13 native
instructions.

### ALU Instructions

- `ADD`
- `SUB`
- `AND`
- `XOR`
- `OR`
- `SLL`
- `SRL`
- `SRA`

### Immediate / Memory / Control Instructions

- `ADDI`
- `LW`
- `SW`
- `BR`
- `JMP`

The assembler also supports pseudo-instructions such as:

- `NOP`
- `MOV`
- `CLR`
- `INC`
- `DEC`
- `NEG`
- `LI`
- `HALT`

See [`isa/ISA.md`](isa/ISA.md) for the complete instruction encoding,
operations, immediate ranges, and pseudo-instruction expansion.

---

## Assembler

A custom two-pass assembler is included with the project.

It converts assembly source code:

```text
program.asm
