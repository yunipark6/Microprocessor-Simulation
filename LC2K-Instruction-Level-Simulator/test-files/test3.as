        lw      0       1       six             r1 contains 3
        lw      0       2       stop            r2 contains 6
        lw      1       3       loo             r3 contains *address 6
        lw      0       4       neg2            r4 contains -2
loop    add     1       3       1
Six2    add     2       4       2
        beq     2       0       skip
        beq     0       0       loop
        noop
skip    halt
six     .fill   Six2
loo     .fill   loop
neg2    .fill   -2
