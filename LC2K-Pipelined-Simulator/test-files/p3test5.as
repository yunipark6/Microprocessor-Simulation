        lw 0 1 one
        lw 0 2 two
        add 1 2 3 // WB stage (check MEMWB reg)
        add 3 3 4 // MEM stage (check EXEMEM reg)
        add 1 3 2 // EXE stage <---
        add 3 4 5
        halt
one .fill 11
two .fill 22