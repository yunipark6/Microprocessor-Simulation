        nor 0 0 0
        lw  5 1 5       // load reg1
        lw  0 2 hi
        lw  0 2 65535
        lw  0 3 5
        beq 0 3 -32768
start   beq 0 3 end
        nor 0 1 4
        nor 6 6 7
        nor 6 2 2
        add 3 -1 3      // decrement reg3
tmp     add 0 -2 7
        beq 0 0 start
end     halt
hi      .fill 1000