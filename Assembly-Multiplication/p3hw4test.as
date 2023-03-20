        lw  0   0   zero
        lw  0   1   one
        lw  0   2   two
        lw  0   3   three
        lw  0   4   four
        lw  0   5   five
        lw  0   6   six
        lw  0   7   seven
start   sw  0   1   num
        lw  0   2   numtwo
        add 3   4   5
        nor 0   1   7
        lw  1   2   done
done    halt
num     .fill 370
numtwo  .fill 595
zero    .fill 0
one     .fill 2
two     .fill -9
three   .fill -5
four    .fill 6
five    .fill 5
six     .fill -2
seven   .fill -4