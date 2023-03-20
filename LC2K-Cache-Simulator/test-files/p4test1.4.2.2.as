        lw          0       1       n           r1 = n
        lw          0       2       r           r2 = r
        lw          0       4       Caddr       load combination function address (r4 = Caddr)
        jalr        4       7                   call function (r7 = 4, branch to start)
        halt
start   lw          0       6       pos1        r6 = 1
        noop    <save values to stack>
        sw          5       7       Stack       save return address to stack
        add         5       6       5           increment r5 stack pointer by 1
        sw          5       1       Stack       save n to stack
        add         5       6       5           increment r5 stack pointer by 1
        sw          5       2       Stack       save r on stack
        add         5       6       5           increment r5 stack pointer by 1
        noop    <base cases>
        beq         0       2       base        base case: if r == 0, return 1
        beq         1       2       base        base case: if n == r, return 1
        noop    <first recursion: n-1>
        lw          0       6       neg1        r6 = -1
        add         1       6       1           n -= 1
        jalr        4       7                   r7 = return address, branch to start
        noop    <second recursion: n-1, r-1>
        sw          5       3       Stack       save running total (stack frame: r7, n, r, r3)
        lw          0       6       pos1        r6 = 1
        add         5       6       5           increment r5 stack pointer by 1
        lw          0       6       neg1        r6 = -1
        add         2       6       2           r -= 1
        jalr        4       7                   r7 = return address, branch to start
        lw          0       6       neg1        r6 = -1
        add         5       6       5           decrement r5 stack pointer by 1
        lw          5       6       Stack       retrieve stack total
        add         3       6       3           add to total
        beq         0       0       skip        skip base case action
base    lw          0       3       pos1        store 1 into r3   
        noop    <restore values from stack>
skip    lw          0       6       neg1        r6 = -1
        add         5       6       5           decrement r5 stack pointer by 1 (point at last value on stack)
        lw          5       2       Stack       restore r2 (r2 = r from prev recursion level)
        add         5       6       5           decrement r5 stack pointer by 1  
        lw          5       1       Stack       restore r1 (r1 = n from prev recursion level)    
        add         5       6       5           decrement r5 stack pointer by 1
        lw          5       7       Stack       restore r7 (r7 = return address from prev recursion level)    
        jalr        7       6                   go to return address (from prev recursion level)
n       .fill       7
r       .fill       3
Caddr   .fill       start
pos1    .fill       1
neg1    .fill       -1
Stack   .fill       0