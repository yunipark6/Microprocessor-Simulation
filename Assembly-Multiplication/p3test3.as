        lw 0 1 three
        lw 0 2 neg1
        noop
        noop
loop    beq 0 1 end
        add 1 2 1
        noop
        beq 0 0 loop
end     halt
three   .fill 3
neg1    .fill -1