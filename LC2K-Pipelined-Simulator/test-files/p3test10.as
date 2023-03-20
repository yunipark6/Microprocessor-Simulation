        lw      0 1 one  // lw followed by non-dependent lw
        lw      0 1 two
        lw      0 2 two  // lw followed by dependent lw
        lw      2 3 one  // reg[3] = 7
        halt
one     .fill 1
two     .fill 2
sev     .fill 7