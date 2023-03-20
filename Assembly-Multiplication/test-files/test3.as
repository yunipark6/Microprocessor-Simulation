        noop 2 0 2
        lw 1 3 512
loop    beq 1 5 end
        jalr 3 6 3 (Think about how that 3 is treated)
        beq 0 0 loop
end     halt yet another comment
        add 4 7 4