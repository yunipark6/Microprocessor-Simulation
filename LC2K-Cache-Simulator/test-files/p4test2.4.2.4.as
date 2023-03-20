        lw      0       1       one
        lw      0       6       sixty
        sw      1       6       20
start   beq     0       6       end
        lw      0       2       neg
        add     6       2       6
end     halt
one     .fill   1
sixty   .fill   60
neg     .fill   -5