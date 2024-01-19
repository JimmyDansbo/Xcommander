.ifndef VTUILIB_INC
VTUILIB_INC=1

; ******************************* Functions ***********************************
VTUI_LIB=VTUI+2

vtui_initialize	= VTUI_LIB+0
vtui_screen_set	= VTUI_LIB+2	; .A = screenmode
vtui_set_bank	= VTUI_LIB+5	; .C = bank number
vtui_set_stride	= VTUI_LIB+8	; .A = stride value
vtui_set_decr	= VTUI_LIB+11	; .C = (1 = decrement, 0 = increment)
vtui_clr_scr	= VTUI_LIB+14	; .A = char, .X = color
vtui_gotoxy	= VTUI_LIB+17	; .A = X coor, .Y = Y coord
vtui_plot_char	= VTUI_LIB+20	; .A = char, .X = color
vtui_scan_char	= VTUI_LIB+23	; .A = char, .X = color
vtui_hline	= VTUI_LIB+26	; .A = char, .Y = length, .X = color
vtui_vline	= VTUI_LIB+29	; .A = char, .Y = length, .X = color
vtui_print_str	= VTUI_LIB+32	; .A=($80=noconvert),r0=ptr,.Y=length,.X=color
vtui_fill_box	= VTUI_LIB+35	; .A=char,r1l=width,r2l=height,.X=color
vtui_pet2scr	= VTUI_LIB+38	; .A = char
vtui_scr2pet	= VTUI_LIB+41	; .A = char
vtui_border	= VTUI_LIB+44	; .A=bordermode,r1l=width,r2l=height,.X=color
vtui_save_rect	= VTUI_LIB+47	; .A=dest(0sys,$80vram),.C=VBANK,r0=src,r1l=w,r2l=h
vtui_rest_rect	= VTUI_LIB+50	; .A=src(0sys,$80vram),.C=VBANK,r0=src,r1l=w,r2l=h
vtui_input_str	= VTUI_LIB+53	; r0=pointer to buffer, .Y=max length, X=color
vtui_get_bank	= VTUI_LIB+56	; .C = bank number
vtui_get_stride	= VTUI_LIB+59	; .A = stride
vtui_get_decr	= VTUI_LIB+62	; .C = decr value

	jsr	vtui_initialize
	jmp	VTUI_LIB_END

VTUI:	.incbin	"VTUI1.0.BIN"

; *****************************************************************************
; Use KERNAL API to set screen to 80x60 or 40x30 or swap between them.
; *****************************************************************************
; INPUT:		.A = Screenmode ($00, $02 or $FF)
; USES:			.A, .X & ,Y
; RETURNS:		.C = 1 in case of error.
; *****************************************************************************
.macro VTUI_SCREEN_SET scrmode
.ifnblank scrmode
	.if (.match (.left (1, {scrmode}), #))
		lda	#scrmode
	.else
		lda	scrmode
	.endif	
.endif
	jsr	vtui_screen_set
.endmacro

; *****************************************************************************
; Set VERA bank (High memory) without touching anything else
; *****************************************************************************
; INPUTS:	.C = Bank number, 0 or 1
; USES:		.A
; *****************************************************************************
.macro VTUI_SET_BANK bank
.ifnblank bank
	.if (.match (.left (1, {bank}), #))
		lda	#bank
	.else
		lda	bank
	.endif
.endif	
	jsr	vtui_set_bank
.endmacro

; *****************************************************************************
; Get current VERA bank (high memory bit)
; *****************************************************************************
; USES:		.A
; RETURNS:	.C = Bank number, 0 or 1
; *****************************************************************************
.macro VTUI_GET_BANK
	jsr	vtui_get_bank
.endmacro

; *****************************************************************************
; Set the stride without changing other values in VERA_ADDR_H
; *****************************************************************************
; INPUT:		.A = Stride value
; USES:			r0l
; *****************************************************************************
.macro VTUI_SET_STRIDE stride
.ifnblank stride
	.if (.match (.left (1, {stride}), #))
		lda	#stride
	.else
		lda	stride
	.endif
.endif
	jsr	vtui_set_stride
.endmacro

; *****************************************************************************
; Get current VERA stride value
; *****************************************************************************
; RETURNS:		.A = stride value
; *****************************************************************************
.macro VTUI_GET_STRIDE
	jsr	vtui_get_stride
.endmacro

; *****************************************************************************
; Set the decrement value without changing other values in VERA_ADDR_H
; *****************************************************************************
; INPUT:		.C (1 = decrement, 0 = increment)
; USES:			.A
; *****************************************************************************
.macro VTUI_SET_DECR decr
.ifnblank decr
	.if (.match (.left (1, {decr}), #))
		.if decr < 1
			clc
		.else
			sec
		.endif
	.else
		lda	decr
		lsr
	.endif
..endif
	jsr	vtui_set_decr
.endmacro

; *****************************************************************************
; Get the current VERA decrement value
; *****************************************************************************
; USES:			.A
; RETURNS:		.C (1 = decrement, 0 = increment)
; *****************************************************************************
.macro VTUI_GET_DECR
	jsr	vtui_get_decr
.endmacro

; *****************************************************************************
; Write character and possibly color to current VERA address
; If VERA stride = 1 and decrement = 0, colorcode in X will be written as well.
; *****************************************************************************
; INPUTS:	.A = character
;		.X = bg-/fg-color
; USES:		.A
; *****************************************************************************
.macro VTUI_PLOT_CHAR ch, col
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		lda	#col
	.else
		lda	col
	.endif
.endif
	jsr	vtui_plot_char
.endmacro

; *****************************************************************************
; Read character and possibly color from current VERA address
; If VERA stride = 1 and decrement = 0, colorcode will be returned in X
; *****************************************************************************
; OUTPUS:	.A = character
;		.X = bg-/fg-color
; USES		.X
; *****************************************************************************
.macro VTUI_SCAN_CHAR
	jsr	vtui_scan_char
.endmacro

; *****************************************************************************
; Create a horizontal line going from left to right.
; *****************************************************************************
; INPUTS:	.A	= Character to use for drawing the line
;		.Y	= Length of the line
;		.X	= bg- & fg-color
; *****************************************************************************
.macro VTUI_HLINE ch, len, col
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
.ifnblank len
	.if (.match (.left (1, {len}), #))
		ldy	#len
	.else
		ldy	len
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_hline
.endmacro

; *****************************************************************************
; Create a vertical line going from top to bottom.
; *****************************************************************************
; INPUTS:	.A	= Character to use for drawing the line
;		.Y	= Height of the line
;		.X	= bg- & fg-color
; USES:		r1h & r2h
; *****************************************************************************
.macro VTUI_VLINE ch, hght, col
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
.ifnblank len
	.if (.match (.left (1, {hght}), #))
		ldy	#hght
	.else
		ldy	hght
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_vline
.endmacro

; *****************************************************************************
; Set VERA address to point to specific point on screen
; *****************************************************************************
; INPUTS:	.A = x coordinate
;		.Y = y coordinate
; *****************************************************************************
.macro VTUI_GOTOXY xcord, ycord
.ifnblank xcord
	.if (.match (.left (1, {xcord}), #))
		lda	#xcord
	.else
		lda	xcord
	.endif
.endif
.ifnblank ycord
	.if (.match (.left (1, {ycord}), #))
		ldy	#ycord
	.else
		ldy	ycord
	.endif
.endif
	jsr	vtui_gotoxy
.endmacro

; *****************************************************************************
; Convert PETSCII codes between $20 and $5F to screencodes.
; *****************************************************************************
; INPUTS:	.A = character to convert
; OUTPUS:	.A = converted character or $56 if invalid input
; *****************************************************************************
.macro VTUI_PET2SCR ch
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
	jsr	vtui_pet2scr
.endmacro

; *****************************************************************************
; Convert screencodes between $00 and $3F to PETSCII.
; *****************************************************************************
; INPUTS:	.A = character to convert
; OUTPUS:	.A = converted character or $76 if invalid input
; *****************************************************************************
.macro VTUI_SCR2PET ch
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
	jsr	vtui_scr2pet
.endmacro

; *****************************************************************************
; Print PETSCII/Screencode string.
; *****************************************************************************
; INPUTS	.A = Convert string (0 = Convert from PETSCII, $80 = no conversion)
;		r0 = pointer to string
;		.Y = length of string
;		.X  = bg-/fg color (only used if stride=0,decr=0&bank=0)
; USES:		.A, .Y & r1
; *****************************************************************************
.macro VTUI_PRINT_STR str, conv, len, col
.ifnblank str
	lda	#<str
	sta	r0
	lda	#>str
	sta	r0+1
	lda	conv
.else
	.ifnblank conv
		.if (.match (.left (1, {conv}), #))
			lda	#conv
		.else
			lda	conv
		.endif
	.endif
.endif
.ifnblank len
	.if (.match (.left (1, {len}), #))
		ldy	#len
	.else
		ldy	len
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_print_str
.endmacro

; *****************************************************************************
; Create a filled box drawn from top left to bottom right
; *****************************************************************************
; INPUTS:	.A	= Character to use for drawing the line
;		r1l	= Width of box
;		r2l	= Height of box
;		.X	= bg- & fg-color
; USES:		.Y if width and height provided as parameters
; *****************************************************************************
.macro VTUI_FILL_BOX ch, width, height, col
.ifnblank width
	.if (.match (.left (1, {width}), #))
		ldy	#width
	.else
		ldy	width
	.endif
		sty	r1l
.endif
.ifnblank height
	.if (.match (.left (1, {height}), #))
		ldy	#height
	.else
		ldy	height
	.endif
		sty	r2l
.endif
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_fill_box
.endmacro

; *****************************************************************************
; Clear the entire screen with specific character and color
; *****************************************************************************
; INPUTS:	.A	= Character to use for filling
;		.X	= bg- & fg-color
; USES:		.Y, r1l & r2l
; *****************************************************************************
.macro VTUI_CLR_SCR ch, col
.ifnblank ch
	.if (.match (.left (1, {ch}), #))
		lda	#ch
	.else
		lda	ch
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_clr_scr
.endmacro

; *****************************************************************************
; Create a box with a specific border
; *****************************************************************************
; INPUTS:	.A	= Border mode (0-6) any other will default to mode 0
;		r1l	= width
;		r2l	= height
;		.X	= bg-/fg-color
; USES		.Y, r0, r1h & r2h
; *****************************************************************************
.macro VTUI_BORDER mode, width, height, col
.ifnblank width
	.if (.match (.left (1, {width}), #))
		ldy	#width
	.else
		ldy	width
	.endif
		sty	r1l
.endif
.ifnblank height
	.if (.match (.left (1, {height}), #))
		ldy	#height
	.else
		ldy	height
	.endif
		sty	r2l
.endif
.ifnblank mode
	.if (.match (.left (1, {mode}), #))
		lda	#mode
	.else
		lda	mode
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_border
.endmacro

; *****************************************************************************
; Copy contents of screen from current position to other memory area in
; either system RAM or VRAM
; *****************************************************************************
; INPUTS:	.C	= VRAM Bank (0 or 1) if .A=$80
;		.A	= Destination RAM (0=system RAM, $80=VRAM)
;		r0 	= Destination address
;		r1l	= width
;		r2l	= height
; USES:		r1h
; *****************************************************************************
.macro VTUI_SAVE_RECT addr, width, height, bank
.ifnblank addr
	lda	#<addr
	sta	r0
	lda	#>addr
	sta	r0+1
.endif
.ifnblank bank
	.if (.match (.left (1, {bank}), #))
		lda	#bank
	.else
		lda	bank
	.endif
	lsr
	lda	#$80
.else
	lda	#$00
.endif
.ifnblank width
	.if (.match (.left (1, {width}), #))
		ldy	#width
	.else
		ldy	width
	.endif
		sty	r1l
.endif
.ifnblank height
	.if (.match (.left (1, {height}), #))
		ldy	#height
	.else
		ldy	height
	.endif
		sty	r2l
.endif
	jsr	vtui_save_rect
.endmacro

; *****************************************************************************
; Restore contents of screen from other memory area in either system RAM
; or VRAM starting at current position
; *****************************************************************************
; INPUTS:	.C	= VRAM Bank (0 or 1) if .A=$80
;		.A	= Source RAM (0=system RAM, $80=VRAM)
;		r0 	= Source address
;		r1l	= width
;		r2l	= height
; *****************************************************************************
.macro VTUI_REST_RECT addr, width, height, bank
.ifnblank addr
	lda	#<addr
	sta	r0
	lda	#>addr
	sta	r0+1
.endif
.ifnblank bank
	.if (.match (.left (1, {bank}), #))
		lda	#bank
	.else
		lda	bank
	.endif
	lsr
	lda	#$80
.else
	lda	#$00
.endif
.ifnblank width
	.if (.match (.left (1, {width}), #))
		ldy	#width
	.else
		ldy	width
	.endif
		sty	r1l
.endif
.ifnblank height
	.if (.match (.left (1, {height}), #))
		ldy	#height
	.else
		ldy	height
	.endif
		sty	r2l
.endif
	jsr	vtui_rest_rect
.endmacro

; *****************************************************************************
; Show a cursor and get a string input from keyboard.
; *****************************************************************************
; INPUTS:	r0 = pointer to buffer to hold string (must be pre-allocated)
;		.Y = maximum length of string
;		.X = color information for input characters
; OUPUTS:	.Y = actual length of input
; USES:		.A & r1
; *****************************************************************************
.macro VTUI_INPUT_STR buf, len, col
.ifnblank buf
	lda	#<buf
	sta	r0
	lda	#>buf
	sta	r0+1
.endif
.ifnblank len
	.if (.match (.left (1, {len}), #))
		ldy	#len
	.else
		ldy	len
	.endif
.endif
.ifnblank col
	.if (.match (.left (1, {col}), #))
		ldx	#col
	.else
		ldx	col
	.endif
.endif
	jsr	vtui_input_str
.endmacro

VTUI_LIB_END:
.endif