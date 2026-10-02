; Fibonacci up to F(23) = 28657 (largest Fib number that fits in
; signed 16-bit two's complement, max +32767). Fully unrolled --
; no branches, since this CPU revision only has unconditional BR.

CLR R1        ; R1 = F(0) = 0
LI R2, 1      ; R2 = F(1) = 1

ADD R3, R1, R2  ; R3 = F(2)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(3)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(4)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(5)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(6)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(7)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(8)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(9)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(10)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(11)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(12)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(13)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(14)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(15)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(16)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(17)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(18)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(19)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(20)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(21)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(22)
MOV R1, R2
MOV R2, R3
ADD R3, R1, R2  ; R3 = F(23)
MOV R1, R2
MOV R2, R3

SW R0, R2     ; store F(23) to RAM address held in R0 (=0)
HALT