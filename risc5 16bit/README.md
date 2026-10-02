# 16-bit Single-Cycle CPU — Assembler & Demo

A custom C assembler for a hand-built 16-bit single-cycle CPU designed from
scratch in Logisim-Evolution (Harvard architecture, 16 registers, 8-operation
ALU, 64K instruction ROM + 64K data RAM).

See `ISA.md` for the full instruction set reference.

## Build

```
gcc -std=c99 -Wall -Wextra -o assembler assembler.c
```

## Assemble a program

```
./assembler fib.asm fib.hex
```

This produces a Logisim-Evolution "v2.0 raw" hex image.

## Load it into the CPU

1. Open the circuit in Logisim-Evolution.
2. Right-click the ROM component -> **Load Image...**
3. Select `fib.hex`.
4. Reset the PC, then tick the clock to step through the program.

## Demo program: `fib.asm`

Computes the Fibonacci sequence up through **F(23) = 28657** — the largest
Fibonacci number that fits in a signed 16-bit two's complement register
(F(24) = 46368 would overflow). The result ends up in register R2 and is also
stored to RAM address 0.

The program is fully unrolled (no branches) because this CPU revision's
`Branch` control signal is currently unconditional — see the "Known hardware
limitation" note in `ISA.md`.

## Files

| File           | Purpose                                      |
|----------------|-----------------------------------------------|
| `ISA.md`       | Full instruction set specification            |
| `assembler.c`  | Two-pass assembler, source -> machine code    |
| `fib.asm`      | Fibonacci demo program                        |
| `fib.hex`      | Assembled Logisim-loadable ROM image          |
| `test.asm`     | Smoke test exercising every instruction       |
