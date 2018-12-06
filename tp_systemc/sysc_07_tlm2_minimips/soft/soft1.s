.section data
.org 0x80

.word 0x1
.word 0x2

.section text
.org 0x0

.set noreorder
.set noat
addi $1, $0, 0x80
lw $2, 0($1)
lw $3, 4($1)
add $4, $2, $3
sw $4, 8($1)
break
.set at
.set reorder
