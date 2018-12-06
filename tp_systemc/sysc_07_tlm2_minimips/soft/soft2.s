.section data
.org 0x80

.word 0x1
.word 0x2

.section text
.org 0x0

.set noreorder
.set noat
debut:
addi $1, $0, 0x80
lw $2, 0($1)
lw $3, 4($1)
beq $5, $1, debut
beq $0, $0, debut
break
.set at
.set reorder
