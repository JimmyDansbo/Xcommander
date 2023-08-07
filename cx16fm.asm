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
	GETSCREENMODE
	jsr	clrscr

	GOTOXY #0, #0
	lda	scrwidth	; Load screen width and half it
	lsr
	sta	r1l
	lda	scrheight
	sta	r2l
	jsr	combinecolors
	tax
	lda	#2
	jsr	vtui_border

	lda	scrwidth
	lsr
	ldy	#0
	GOTOXY
	jsr	combinecolors
	tax
	lda	#2
	jsr	vtui_border

	jsr	CHRIN
	jsr	CHRIN
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
	lda	#BLUE
	sta	bgcolor
	lda	#WHITE
	sta	fgcolor
	lda	#CYAN
	sta	hilightbg
	lda	#BLACK
	sta	hilightfg
	rts
