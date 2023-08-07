.ifndef VIDEO_INC
VIDEO_INC=1

	jmp	VIDEO_INC_END

scrwidth: .byte 0
scrheight:.byte 0
bgcolor:  .byte BLUE
fgcolor:  .byte WHITE
hilightbg:.byte CYAN
hilightfg:.byte BLACK

HOR_LINE = $40 ; PET=$C0
VER_LINE = $5D ; PET=$DD
BOT_LEFT = $6D ; PET=$AD
TOP_RIGT = $6E ; PET=$AE
BOT_RIGT = $7D ; PET=$BD
TOP_LEFT = $70 ; PET=$B0
TOP_TEE  = $71 ; PET=$B1
BOT_TEE  = $72 ; PET=$B2
LEFT_TEE = $73 ; PET=$B3
RIGT_TEE = $6B ; PET=$AB

;******************************************************************************
; Use PLOT to move cursor to specific coordinate on screen
;******************************************************************************
; INPUT:	xc = X coordinate (0-79)
;		yc = Y coordinate (0-59)
;******************************************************************************
.macro GOTOXY xc, yc
	.ifnblank yc
		ldy	yc
	.endif
	.ifnblank xc
		lda	xc
	.endif
	jsr	vtui_gotoxy
.endmacro

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
; Combine the foreground and background colors in two bytes to a single byte
; where background color is in high nibble and foreground color is in low.
;******************************************************************************
combinecolors:
	lda	bgcolor
	asl
	asl
	asl
	asl
	ora	fgcolor
	rts

;******************************************************************************
; Function that is meant to replace the kernal CHROUT function
;******************************************************************************
plotchar:
	phx
	pha			; Save character to write temporarily
	lda	#1		; Set VERA stride to 1 as we want to print
	jsr	vtui_set_stride	; both character and color information
	jsr	combinecolors
	tax			; Move it to X register for VTUI function
	pla			; Restore char to print
	pha
	jsr	vtui_plot_char
	pla
	plx
	rts

;******************************************************************************
; Use Kernal to clear the screen
;******************************************************************************
clrscr:	
	jsr	combinecolors
	tax
	lda	#' '
	jmp	vtui_clr_scr

	
VIDEO_INC_END:
.endif