
; crt0 code for standard Mailstation apps running from 0x4000

	.module crt0
	.globl	_main

	.area	_HEADER (ABS)

	.org	0x4000

	jp init

	; Mailstation app header
	.dw	(_ms_app_icon)	; Start of the icon header
	.dw	(_ms_app_name)	; Start of the App name header
	.dw	#0x0000		; Start of data header with unknown function
	.db	#0x00		; Byte of unknown meaning
	.dw	#0x0000		; X position adjustment of Channel/icon1
	.dw	#0x0000		; Y position adjustment of Channel/icon1

init:
	ld	sp, #0x0000

	call	gsinit
	call	_main
	jp	_exit


_exit::
	jp	0x0000

gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:
	ret


code_start:

	; Ordering of segments for the linker.
	.area   _CODE

	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED

	.area   _HEAP

