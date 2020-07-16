.section .text.start

.type _start, %function
.func _start
.global _start
_start:
	li $sp, __stack_top
	li $gp, 0

	li $t0, __bss_start
	li $t1, __bss_end
	li $t2, 0
	beq $t0, $t1, 2f

1:
	sw $t2, 0($t0)
	addiu $t0, 4
	bne $t0, $t1, 1b

2:
	jal entry
	j 2b

.endfunc
