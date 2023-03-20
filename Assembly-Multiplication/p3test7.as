        lw 0 3 one
        lw 0 1 three // no stalls
        lw 0 2 neg1
loop    beq 0 1 end
        add 1 2 1       // reg1 -= 1
        add 0 2 2       // reg2 = reg2
        nor 1 1 4       // reg1 nor reg2
        beq 0 0 loop
end     halt
three   .fill 3
neg1    .fill -1
one     .fill 1