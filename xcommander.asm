.segment "STARTUP"
.segment "INIT"
.segment "ONCE"
.segment "CODE"

.include "cx16.inc"
.include "vera0.9.inc"
.include "vtuilib-ca65.inc"
.include "video.inc"
.include "file.inc"

moffset		= file_variables_end
irqhit		= moffset
old_irq_handler = moffset+1 ; 2 bytes

.macro WINC var
	inc	var
	bne	*+4
	inc	var+1
.endmacro

;******************************************************************************
main:
	jsr	initvars

	; Save address of original IRQ handler
	lda	$314
	sta	old_irq_handler
	lda	$315
	sta	old_irq_handler+1
	; Install custom IRQ handler, called irq_tick
	sei
	lda	#<irq_tick
	sta	$314
	lda	#>irq_tick
	sta	$315
	cli

	jsr	getscreenmode
	jsr	clrscr

	jsr	drawboxs

	; Empty keyboard buffer
:	jsr	GETIN
	cmp	#0
	bne	:-

	; print menu
@startmenu:
	stz	curhotkeymenu
@updatemenu:
	jsr	updatehotkeys
@wait:	jsr	waitforkey

	; switch (keypress)
@case_esc:
	cmp	#$1B			; ESC
	beq	@end
@caes_tab:
	cmp	#$09			; TAB
	bne	@case_f8
	jsr	switchpanel
@case_f8:
	cmp	#$8C			; F8
	beq	@end
@case_f9:
	cmp	#$10			; F9
	bne	@wait
	inc	curhotkeymenu
	lda	maxhotkeymenu
	cmp	curhotkeymenu
	bcs	@updatemenu
	bra	@startmenu
@end:	; Ensure HSCROLL is set to 0
	stz	VERA_L1_HSCROLL_H
	stz	VERA_L1_HSCROLL_L

	; Restore original IRQ handler
	sei
	lda	old_irq_handler
	sta	$314
	lda	old_irq_handler+1
	sta	$315
	cli

	rts


;******************************************************************************
; Update irqhit variable to show that an interrupt tick has occurred
;******************************************************************************
irq_tick:
	stz	irqhit
	jmp	(old_irq_handler)

;******************************************************************************
; Busy loop to wait for keyboard press
;******************************************************************************
waitforkey:
	wai
	jsr	GETIN
	cmp	#0
	beq	waitforkey
	rts


;******************************************************************************
; Return length of string in .Y
; If length exceeds 256, carry flag will be set
;******************************************************************************
; INPUT:	.X = low byte of string start address
;		.Y = high byte of string start address
; OUTPUT:	.Y = Length of string
;		.C = Set if string longer than 256 bytes
; USES:		.A & TMP_PTR0
;******************************************************************************
strlen:
	stx	TMP_PTR0	; Store string address in TMP_PTR0
	sty	TMP_PTR0+1
	ldy	#0
:	lda	(TMP_PTR0),y	; Read bytes from string until 0-char
	beq	@end
	iny
	bne	:-		; If Y rolled over to zero, string is too long
	sec
@end:	rts

;******************************************************************************
; Reverse a string in current buffer
;******************************************************************************
; INPUT:	.X = low byte of string start address
;		.Y = high byte of string start address
; USES:		.A & TMP_PTR0
;******************************************************************************
strrev:
	jsr	strlen		; Get length of string
	bcs	@end		; If longer than 256 bytes, it's an error
	cpy	#2		; No reversal if length<2
	bcc	@end
	dey
@loop:	lda	(TMP_PTR0),y	; Read from end of string
	pha			
	lda	(TMP_PTR0)	; Read from beginning of string
	sta	(TMP_PTR0),y	; Write to end of string
	pla
	sta	(TMP_PTR0)	; Write to beginning of string
	WINC	TMP_PTR0	; Inc TMP_PTR0 to go to next char
	dey			; Dec .Y to go from right to left
	beq	@end		; If not 0, do it twice as TMP_PTR0 has just
	dey			; been moved 1 char forward
	bne	@loop
@end:	rts

;******************************************************************************
; Initialize global variables to sane default values
;******************************************************************************
initvars:
	lda	#(BLUE<<4)|WHITE
	sta	txtcolor
	lda	#(CYAN<<4)|BLUE
	sta	hilightcol
	lda	#(BLUE<<4)|YELLOW
	sta	headercol
	lda	#(CYAN<<4)|BLACK
	sta	hotkeycol
	lda	#1
	sta	twopanels
	lda	#40
	sta	panel2x
	stz	curhotkeymenu
	lda	#3			; Number of menus for 20 or 22 width
	sta	maxhotkeymenu
	rts
