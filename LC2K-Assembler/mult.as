        lw      0       2       mcand   // reg2 = mcand
        lw      0       3       mplier  // reg3 = mplier
        lw      0       4       i       // reg4 = 16
        lw      0       5       one     // reg5 = 1
loop    beq     0       4       end     // loop: 16 iterations
        nor     3       3       6       // nor the mplier (inverted)
        nor     5       5       7       // nor the mask (mostly ones)
        nor     7       6       6       // reg6 = isolated bit
        add     5       5       5       // left shift mask by 1
        beq     0       6       skip    // if bit is 0, don't add
        add     1       2       1       // add mcand to reg1
skip    add     2       2       2       // left shift mcand by 1
        lw      0       7       neg1    // r7  = -1
        add     4       7       4       // decrement r4
        beq     0       0       loop    // loop
end     halt                            // end
mcand   .fill   1103 // r2
mplier  .fill   7043 // r3
i       .fill   16   // r4
neg1    .fill   -1   // 
one     .fill   1  
