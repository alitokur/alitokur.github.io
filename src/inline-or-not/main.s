	.text
	.intel_syntax noprefix
	.file	"main.cpp"
	.globl	_Z3sumii                        # -- Begin function _Z3sumii
	.p2align	4, 0x90
	.type	_Z3sumii,@function
_Z3sumii:                               # @_Z3sumii
	.cfi_startproc
# %bb.0:
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + rsi]
	ret
.Lfunc_end0:
	.size	_Z3sumii, .Lfunc_end0-_Z3sumii
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	mov	dword ptr [rsp - 4], 0
	xor	eax, eax
	add	dword ptr [rsp - 4], 3
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc
                                        # -- End function
	.ident	"Ubuntu clang version 18.1.3 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
