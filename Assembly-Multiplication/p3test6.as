        lw 0 1 one // LW FOLLOWED BY DEPENDENT LW
        lw 1 2 one // reg[2] = 12
        sw 0 2 5   // data[5] = 12
        lw 0 3 5   // reg[3] = 12
        add 3 2 3  // reg[3] = 24
        halt
one     .fill 1
        .fill 12
five    .fill 5