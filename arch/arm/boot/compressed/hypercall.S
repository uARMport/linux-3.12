.globl memcpy_hpc

memcpy_hpc:
    MOV R12, #6
	.word 0xF7BBBBBB;
	BX LR