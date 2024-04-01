section .data

section .text
bits 64
default rel
global process_array_asm

;rcx *X
;rdx *Y
;r8 *Z
;xmm3 a
;[rbp+32] size 

process_array_asm:
push rsi
push rbp
mov rbp, rsp
add rbp, 16
add rbp, 8 

mov r10, 0
L1:
	cmp r10, [rbp + 32]
	je DONE
	movss xmm4, [rcx + 4 * r10]
	mulss xmm4, xmm3
	addss xmm4, [rdx + 4 * r10]
	movss [r8 + 4 * r10], xmm4
	inc r10
	jmp L1

DONE:
pop rbp
pop rsi
ret