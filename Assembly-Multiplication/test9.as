    lw  0   1   5       // TEST CASE FOR -32768 beq
    beq 0   1   -5      // 0000000 100 000 001 111111111111011
    beq 0   1   -32768  // 0000000 100 000 001 1000000000000000
    beq 0   1   -32769  // exit(1)