.segment "STARTUP"
.segment "INIT"
.segment "ONCE"
.segment "CODE"

.include "cx16.inc"
.include "vera0.9.inc"
.include "vtuilib-ca65.inc"
.include "video.inc"
.include "file.inc"

.macro WINC var
	inc	var
	bne	*+4
	inc	var+1
.endmacro

;******************************************************************************
main:
	jsr	initvars
	jsr	getscreenmode
	jsr	clrscr

	jsr	drawboxs

	; print menu
:	stz	curhotkeymenu
:	jsr	updatehotkeys
	jsr	waitforkey
	inc	curhotkeymenu
	lda	maxhotkeymenu
	cmp	curhotkeymenu
	bcs	:-
	bra	:--
	rts

waitforkey:
	jsr	GETIN
	cmp	#0
	beq	waitforkey

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
