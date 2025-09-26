
.globl _ms_port_shadow
.globl _ms_irq_handlers

IRQ_HANDLER_PAGE = 0xD6

_ms_install_irq_handler::

    ; Create a jump trampoline for IRQ 2 mode.
    ; This is needed because in this mode, the CPU expects the I/O device to provide
    ;  the low byte of the address table on the data bus.
    ; Of course the custom SoC is not going to do that, so when the interrupt occurs,
    ;  a random byte will be on the bus so the CPU will read a random location
    ;  within the specified interrupt vector page.
    ; To counter this, we fill the entire page with a 0xD6 byte. This way, the CPU
    ;  will always go to 0xD6D6 upon interrupt.
    ; Then we place a jump instruction at that location to our actual IRQ handler
    ;  which we place at 0xD600.
    ; As long as the IRQ handler does not exceed 192 bytes so that it doesn't overwrite
    ;  the jump instruction at 0xD6D6, this elaborate scheme will work.
    ; This also ensures optimal memory usage - the RAM at 0xD500 oto 0xD6D8 is used
    ;  by the interrupt code, and the rest is free for other uses.

    ; First, fill the entire page at 0xD500 with 0xD6.

    ld b, #0
    ld hl, #((IRQ_HANDLER_PAGE - 1) * 0x100)
.loop:
    ld (hl), #IRQ_HANDLER_PAGE
    inc hl
    djnz .loop

    ; Fill the last byte (if the random byte on the data bus happens to be 0xFF,
    ;  the CPU will NOT wrap around, contrary to some misleading documentation).

    ld (hl), #IRQ_HANDLER_PAGE

    ; Then place a jump instruction at 0xD6D6 to our IRQ handler.

    ld hl, #(IRQ_HANDLER_PAGE * 0x100 + IRQ_HANDLER_PAGE)
    ld (hl), #0xC3      ; JP nn opcode
    inc hl
    ld (hl), #0x01      ; Low byte of routine address
    inc hl
    ld (hl), #IRQ_HANDLER_PAGE      ; High byte of routine address

    ; Now copy the actual interrupt handler routine to the location at 0xD601.

    ld hl, #_ms_irq_handler
    ld de, #(IRQ_HANDLER_PAGE * 0x100 + 1)
    ld bc, #_ms_irq_handler_end - _ms_irq_handler
    ldir

    ; Patch the absolute call instruction in the IRQ handler

    ld bc, #_irq_do_call - _ms_irq_handler + IRQ_HANDLER_PAGE*0x100 + 1
    ld hl, #_irq_patch_instruction - _ms_irq_handler + IRQ_HANDLER_PAGE*0x100 + 2
    ld (hl), c
    inc hl
    ld (hl), d

    ; Finally, set the appropriate IRQ mode and vector.

    ld a, #(IRQ_HANDLER_PAGE - 1)
    ld i, a
    im 2

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; The actual IRQ handler routine.
; It gets copied to RAM at 0xD601 so that it runs correctly regardless of what memory
;  is mapped at 0x4000 and 0x8000.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_ms_irq_handler:

    ; Push all registers onto the stack.

    push af
    push bc
    push de
    push hl
    push ix
    push iy

    ; Load current IRQ status.

    in a, (0x03)

    ; Check which IRQ is active, by testing bits until we find one that is set.
    ; Register B contains the interrupt number, and register C contains the bit mask
    ;  to disable the interrupt.

    ld b, #8
    ld c, #0x7F
.irq_loop:
    rla
    jr c, .irq_found
    rrc c
    djnz .irq_loop
    jr .irq_done        ; No IRQ active, should not happen but let's be safe.

    ; Now we need to acknowledge the interrupt by disabling and re-enabling it.
.irq_found:
    ld a, (_ms_port_shadow)
    and c
    out (0x03), a
    ld a, (_ms_port_shadow)
    out (0x03), a

    ; Now the IRQ number is in register B, incremented by 1.
    ; Go through the list of handlers and call the appropriate ones.

    ld hl, #_ms_irq_handlers-1
.irq_table_loop:
    inc hl
    ld a, (hl)
    cp #0
    jr z, .irq_done     ; End of table, no handler found.
    cp b
    jr z, .irq_call     ; Found a matching handler, call it.
    inc hl
    inc hl
    inc hl
    inc hl
    inc hl
    jr .irq_table_loop
.irq_call:
    inc hl
    ld c, (hl)
    inc hl
    ld a, (hl)
    in d, (c)
    out (c), a
    inc hl
    ld a, (hl)
    dec c
    in e, (c)
    out (c), a
    push de
    inc hl
    ld e, (hl)
    inc hl
    ld d, (hl)
    push hl
    push bc
    ex de, hl
_irq_patch_instruction:
    call _irq_do_call
    pop bc
    pop hl
    pop de
    out (c), e
    inc c
    out (c), d
    jr .irq_table_loop

_irq_do_call:
    jp (hl)

    ; Pop registers and re-enable interrupts.

.irq_done:
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
    ei
    ret

_ms_irq_handler_end:
