        lw      0 1 one  // reg1 = 1
        beq     0 1 end  // if 0 == 1
        lw      0 2 neg2 // reg2 = -2
        lw      2 2 15   // reg2 = mem[-2 + _]
        lw      0 7 sev
        add     2 7 1
        nor     1 1 1
        lw      0 4 elev
        nor     4 1 5
end     halt
one     .fill   1
neg2    .fill   -2
sev     .fill   7
elev    .fill   11