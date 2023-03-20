    lw      0   1   hun // r1 = 100  
    jalr    0   0       // r0 = 0
    sw      0   1   0   // address 100: 100
    sw      1   1   1   // address 201: 100
    add     1   1   1   // r1 = 200
    lw      0   2   two // r2 = 100
    add     2   2   3   // r3 = 200
    beq     3   1   2   // skip next two lines
    halt
back add     2   1   2   // r2 = 300
    noop    hellooo 100
    nor     0   3   4   // r4 = store inverted 200
    add     3   3   3   // r3 = 400
    lw      3   5   neg // r5 = 100
    add     0   0   0
    beq     5   2   back
    noop
    lw      0   7   two1
    jalr    7   0       // jump to address 21
    halt
hun .fill   100
two .fill   201
neg .fill   -199
two1    .fill   21