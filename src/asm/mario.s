    .text
    .file "mario.s"
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2                               # -- Begin function main
.LCPI0_0:
	.long	0x3B000015                      # float 0.00195313
	.p2align	2                               # -- Begin function main
.LCPI0_1:
	.long	0xC1800000                      # float -16.0
    .text
    .globl scaleTreeSlideSpeed__Fff        # -- Begin function scaleTreeSlideSpeed__Fff
    .p2align	2
	.type	scaleTreeSlideSpeed__Fff,@function
scaleTreeSlideSpeed__Fff:                   # @scaleTreeSlideSpeed__Fff
.Lfunc_begin0:
    .cfi_startproc
# %bb.0:                                # %entry
    stwu 1, -32(1)
	.cfi_def_cfa_offset 32
	.cfi_offset f1, -12
	.cfi_offset f2, -8
    stfs 1, 20(1)                           # 4-byte Folded Spill
    stfs 2, 24(1)                           # 4-byte Folded Spill
    lfs 1, 2840(31)
    lis 3, .LCPI0_0@ha
	lfs 0, .LCPI0_0@l(3)
    fmuls 1, 0, 1
    lis 3, .LCPI0_1@ha
	lfs 0, .LCPI0_1@l(3)
    fcmpo cr0, 2, 0
    li 3, 1
    blt cr0, .Ltmp0
    li 3, 0
    stw 3, 0xA8 (31)
    .Ltmp0:
    lfs 2, 24(1)                            # 4-byte Folded Reload
    lfs 1, 20(1)                            # 4-byte Folded Reload
    addi 1, 1, 32
    blr
.Lfunc_end0:
	.size	scaleTreeSlideSpeed__Fff, .Lfunc_end0-.Lfunc_begin0
	.cfi_endproc
                                        # -- End function
    .ident	"clang version 13.0.0 (https://github.com/DotKuribo/llvm-project.git)"