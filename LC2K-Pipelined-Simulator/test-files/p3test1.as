    lw 0 7 sev
    lw 0 1 one
    lw 0 2 two
    lw 0 3 thre
    sw 7 7 0    // mem[7] = 7
    sw 1 7 2    // mem[3] = 7
    sw 2 7 -1   // mem[1] = 7
    sw 5 7 10   // mem[10] = 7
    halt
one .fill 1
two .fill 2
thre .fill 3
sev .fill 7