; quick smoke test of every instruction/pseudo
CLR R1
LI R2, 23
ADDI R3, R2, -5
ADD R4, R2, R3
SUB R5, R4, R1
AND R6, R2, R3
XOR R7, R2, R3
OR  R8, R2, R3
SLL R9, R2, R1
INC R9
DEC R9
MOV R10, R9
NEG R11, R10
SW R0, R2
LW R12, R0
JMP 0
NOP
HALT