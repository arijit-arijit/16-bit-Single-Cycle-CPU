# 16-bit Single-Cycle CPU — Instruction Set Architecture

## Overview

- **Word size:** 16 bits
- **Registers:** 16 general-purpose registers, R0–R15 (16-bit each)
- **Memory model:** Harvard — separate 64K x16 instruction ROM and 64K x16 data RAM
- **Execution model:** single-cycle (one instruction fully completes per clock tick)
- **ALU:** 8 operations (ADD, SUB, AND, XOR, OR, SLL, SRL, SRA), selected by a 3-bit ALUOp
- **Flags:** N (negative), C (carry), V (overflow) — exposed from the ALU; not currently
  used for conditional control flow (see Limitations)

## Instruction format

Every instruction is a single 16-bit word, split into four 4-bit fields:

```
15        12 11         8 7          4 3          0
+-----------+------------+------------+------------+
|  opcode   |   field1   |   field2   |   field3   |
+-----------+------------+------------+------------+
```

The meaning of field1/field2/field3 depends on the instruction type below.

## Native instructions (13)

| Opcode | Mnemonic | Format              | Encoding (f1\|f2\|f3)      | Operation                          |
|--------|----------|----------------------|----------------------------|-------------------------------------|
| 0000   | ADD      | `ADD rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 + rs2                      |
| 0001   | SUB      | `SUB rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 − rs2                      |
| 0010   | AND      | `AND rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 & rs2                      |
| 0011   | XOR      | `XOR rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 ^ rs2                      |
| 0100   | OR       | `OR rd, rs1, rs2`    | rd \| rs1 \| rs2            | rd = rs1 \| rs2                     |
| 0101   | SLL      | `SLL rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 << rs2                     |
| 0110   | SRL      | `SRL rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 >> rs2 (logical)           |
| 0111   | SRA      | `SRA rd, rs1, rs2`   | rd \| rs1 \| rs2            | rd = rs1 >> rs2 (arithmetic)        |
| 1000   | ADDI     | `ADDI rd, rs1, imm`  | rd \| rs1 \| imm4 (signed)  | rd = rs1 + sign_extend(imm4)        |
| 1001   | LW       | `LW rd, rs1`         | rd \| rs1 \| 0000           | rd = RAM[rs1]                       |
| 1010   | SW       | `SW rs1, rs2`        | 0000 \| rs1 \| rs2          | RAM[rs1] = rs2                      |
| 1011   | BR       | `BR offset`          | 0000 \| 0000 \| offset4     | PC = PC + sign_extend(offset4)      |
| 1100   | JMP      | `JMP target`         | target[11:8]\|[7:4]\|[3:0]  | PC = zero_extend(target12)          |

Opcodes `1101`–`1111` are reserved (decode to all control signals = 0, i.e. an
implicit no-op).

**Immediate ranges:**
- `ADDI` / `BR` immediates are 4-bit signed two's complement: **−8 to +7**
- `JMP` targets are 12-bit unsigned: **0 to 4095**

## Pseudo-instructions (8)

These don't have their own opcode — the assembler expands them into one or more
native instructions.

| Pseudo              | Expands to                                         | Notes |
|----------------------|-----------------------------------------------------|-------|
| `NOP`                | `0xF000` (reserved opcode, all control signals 0)   | 1 word |
| `MOV rd, rs`         | `ADDI rd, rs, 0`                                     | 1 word |
| `CLR rd`             | `SUB rd, rd, rd`                                     | 1 word |
| `INC rd`             | `ADDI rd, rd, 1`                                     | 1 word |
| `DEC rd`             | `ADDI rd, rd, -1`                                    | 1 word |
| `NEG rd, rs`         | `CLR rd` + `SUB rd, rd, rs`                           | 2 words |
| `LI rd, imm16`       | `CLR rd` + repeated `ADDI rd, rd, ±7` chunks          | variable |
| `HALT`               | `BR -1` (infinite self-branch)                        | 1 word |

`LI` (load immediate) handles any 16-bit signed value by chunking it into
steps of at most ±7 (the largest value a single `ADDI` can add), since there's
no dedicated "load a full 16-bit constant" instruction in the native set.

## Known hardware limitation

`Branch` is currently **unconditional** in this CPU revision — it is asserted
directly from the opcode decode and is not gated by the ALU's zero/equality
result. This means `BR` always branches; there is no conditional branch yet.
Programs in this repo are written branch-free (fully unrolled) to stay
entirely within what the hardware actually does. A flag-gated conditional
branch is a natural next addition.

## Register convention

There is no hardware-enforced zero register. By software convention, avoid
relying on any register holding 0 unless you've just cleared it yourself
(e.g. with `CLR`).
