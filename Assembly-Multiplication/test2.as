        lw 0 1 count
        lw 0 2 i
        lw 0 3 neg1
start   beq 0 1 end
        add 1 3 1
        add 2 2 2
        beq 0 1 1
        beq 0 0 start
end     halt
        count .fill 10
        i .fill 1
        neg1 .fill -1