.ifndef VIDEO_INC
VIDEO_INC=1

scrwidth: .byte 0
scrheight:.byte 0
bgcolor:  .byte PET_BLUE
fgcolor:  .byte PET_WHITE
hilightbg:.byte PET_CYAN
hilightfg:.byte PET_BLACK

HOR_LINE = $C0
VER_LINE = $DD
BOT_LEFT = $AD
TOP_RIGT = $AE
BOT_RIGT = $BD
TOP_LEFT = $B0
TOP_TEE  = $B1
BOT_TEE  = $B2
LEFT_TEE = $B3
RIGT_TEE = $AB

;******************************************************************************
; Use PLOT to move cursor to specific coordinate on screen
;******************************************************************************
; INPUT:	xc = X coordinate (0-79)
;		yc = Y coordinate (0-59)
;******************************************************************************
.macro GOTOXY xc, yc
	.ifnblank yc
		ldx	yc
	.endif
	.ifnblank xc
		ldy	xc
	.endif
	clc
	jsr	PLOT
.endmacro

;******************************************************************************
; Use Kernal to clear the screen
;******************************************************************************
clrscr:
	lda	bgcolor
	jsr	CHROUT
	lda	#PET_SWAP_FGBG
	jsr	CHROUT
	lda	fgcolor
	jsr	CHROUT
	lda	#147
	jmp	CHROUT

;******************************************************************************
; Get the current screen mode and write the width and height to the
; global variables scrwidht & scrheight
;******************************************************************************
.macro GETSCREENMODE
	sec
	jsr	Screen_mode
	stx	scrwidth
	sty	scrheight
.endmacro

;******************************************************************************
; Print a horizontal line
;******************************************************************************
; INPUT:	.A = character to use for printing the line
;		.X = length of the line
;******************************************************************************
hline:
	jsr	CHROUT
	dex
	bne	hline
	rts

;******************************************************************************
; Print a vertical line
;******************************************************************************
; INPUT:	.A = character to use for printing the line
;		.Y = height of the line
;******************************************************************************
vline:
	jsr	CHROUT
	dey
	beq	:+		; If .Y is 0, we are done
	pha
	lda	#PET_CSR_LEFT
	jsr	CHROUT
	lda	#PET_CSR_DOWN
	jsr	CHROUT
	pla
	bra	vline
:	rts

;******************************************************************************
; Print a box
;******************************************************************************
; INPUT:	.X = Width
;		.Y = Height
;******************************************************************************
box:
@xc = TMP0
@yc = TMP1
@width = TMP2
@height = TMP3

	dex			; Subtract 2 from width to make room for corners
	dex
	stx	@width		; Save width and height
	dey			; Subtract 2 from height to make room for corners
	dey
	sty	@height
	sec			; Get current cursor coordinates
	jsr	PLOT
	sty	@xc		; Save current coordinates
	stx	@yc

	lda	#TOP_LEFT	
	jsr	CHROUT
	ldx	@width		; Get width minus corners into .X
	lda	#HOR_LINE
	jsr	hline
	lda	#TOP_RIGT
	jsr	CHROUT

	lda	#PET_CSR_LEFT	; Move cursor 1 column back and 1 line down
	jsr	CHROUT
	lda	#PET_CSR_DOWN
	jsr	CHROUT

	ldy	@height		; Get height minus corners into .Y
	lda	#VER_LINE
	jsr	vline

	; Move back to top left corner to draw the other half of the box
	ldy	@xc		; Get original coordinates 
	ldx	@yc
	inx			; Go down to next line
	GOTOXY
	ldy	@height		; Get height minus corners into .Y
	lda	#VER_LINE
	jsr	vline

	lda	#PET_CSR_LEFT	; Move cursor 1 column back and 1 line down
	jsr	CHROUT
	lda	#PET_CSR_DOWN
	jsr	CHROUT

	lda	#BOT_LEFT
	jsr	CHROUT

	ldx	@width
	lda	#HOR_LINE
	jsr	hline

	; Trick to write to the bottom right corner of the screen without scrolling
	lda	#PET_CSR_LEFT	; Move cursor left
	jsr	CHROUT
	lda	#BOT_RIGT	; Write the char that is going to bottom right
	jsr	CHROUT
	lda	#PET_CSR_LEFT	; Move cursor left again
	jsr	CHROUT
	lda	#PET_INSERT	; Print INSERT which moves chars right
	jsr	CHROUT
	lda	#HOR_LINE	; Reprint the horizontal line that was overwritten
	jmp	CHROUT
	

.endif