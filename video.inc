.ifndef VIDEO_INC
VIDEO_INC=1

	jmp	VIDEO_INC_END
voffset		= $400
scrwidth 	= voffset+0
scrheight	= voffset+1
txtcolor	= voffset+2
hilightcol	= voffset+3
headercol	= voffset+4
hotkeycol	= voffset+5
twopanels	= voffset+6
panel2x		= voffset+7
activex		= voffset+8
activey		= voffset+9
cursor1y	= voffset+10
cursor2y	= voffset+11
cursorwidth	= voffset+12
curhotkeymenu	= voffset+13
maxhotkeymenu	= voffset+14
scrollval	= voffset+15

video_variables_end = voffset+16

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
BACKSLASH= $4D ; PET=$6D

SCR20_SCROL_VAL	= $A0
SCR22_SCROL_VAL = $B0
SCR32_SCROL_VAL = $00

SCR20_CURSOR_WIDTH	= 18
SCR22_CURSOR_WIDTH	= 20
SCR32_CURSOR_WIDTH	= 30
SCR40_CURSOR_WIDTH	= 18
SCR64_CURSOR_WIDTH	= 30
SCR80_CURSOR_WIDTH	= 38

HOR_LINL = $67
HOR_LINR = $65

helpstr:
	.byte	"help "
HELPSTRLEN = *-helpstr
editstr:
	.byte	"edit "
EDITSTRLEN = *-editstr
copystr:
	.byte	"copy "
COPYSTRLEN = *-copystr
renmovstr:
	.byte	"renmov "
RENMOVESTRLEN = *-renmovstr
mkdirstr:
	.byte	"mkdir "
MKDIRSTRLEN = *-mkdirstr
deletestr:
	.byte	"delete "
DELETESTRLEN = *-deletestr
pulldnstr:
	.byte	"pulldn "
PULLDNSTRLEN = *-pulldnstr
quitstr:
	.byte	"quit "
QUITSTRLEN = *-quitstr
morestr:
	.byte	"more "
MORESTRLEN = *-morestr

;******************************************************************************
; Increment a 16 bit (word) value
;******************************************************************************
; INPUT:	var = address of LSB of word value
;******************************************************************************
.macro WINC var
	inc	var
	bne	:+
	inc	var+1
:
.endmacro

;******************************************************************************
; Update the cursor
; Set the color attribute for all characters in the line of the cursor
;******************************************************************************
; INPUTS:	.X = Cursor color attribute if set to txtcolor, the cursor is
;		not visible
; USES:		.A & .Y
;******************************************************************************
updatecursor:
	lda	VERA_ADDR_H		; Save current VERA control byte
	pha
	lda	#$21			; Ensure increment by 2
	sta	VERA_ADDR_H
	ldy	activey			; Point to activex,activey
	lda	activex
	VTUI_GOTOXY
	inc	VERA_ADDR_L		; Move to colorcode instead of character
	ldy	cursorwidth
:	stx	VERA_DATA0
	dey
	bne	:-
	pla				; Restore VERA control byte
	sta	VERA_ADDR_H
	rts

;******************************************************************************
; Switch between panel1 and panel2 if they are not shown on a single screen
; This only happens if X resolution is below 40 columns
;******************************************************************************
switchpanel:
	ldx	txtcolor
	jsr	updatecursor

	lda	activex			; If activex is less than 2 then the 
	cmp	#2			; currently active panel is 1
	jsr	swapvars

@scroll:ldx	hilightcol
	jsr	updatecursor
	; If twopanels is non-zero, the two panels is shown on a single screen
	; and no scrolling is necessary
	lda	twopanels
	beq	:+
	rts
	; If L1 HSCROLL value is 0, we are currently showing panel 1 and need
	; to switch to showing panel 2
:	lda	VERA_L1_HSCROLL_H
	ora	VERA_L1_HSCROLL_L
	beq	@show_panel_2
	; Show Panel 1
	lda	VERA_L1_HSCROLL_H	; Only decrement L1_HSCROLL_H if it is
	beq	@countdown		; larger than 0
	dec	VERA_L1_HSCROLL_H
@countdown:
	jsr	irqwait			; Wait for VERA IRQ hit
	; Scroll left by $10 pixels
	lda	VERA_L1_HSCROLL_L
	sec
	sbc	#$10
	sta	VERA_L1_HSCROLL_L
	bne	@countdown		; Loop while we have not reached 0
	rts
@show_panel_2:
@countup:
	jsr	irqwait			; Wait for VERA IRQ hit
	; Scroll right by $10 pixels
	lda	VERA_L1_HSCROLL_L
	clc
	adc	#$10
	sta	VERA_L1_HSCROLL_L
	cmp	scrollval		; Loop while we have not hit scrollval
	bne	@countup
	lda	scrollval		; If scrollvall is 0, we need to inc
	bne	:+			; L1_HSCROLL_H to get to the correct
	inc	VERA_L1_HSCROLL_H	; postition 
:	rts


;******************************************************************************
; Wait for the VERA IRQ hit
;******************************************************************************
irqwait:
	pha
:	wai
	lda	irqhit
	bne	:-
	inc	irqhit
	pla
	rts

;******************************************************************************
; Draw the box around the first panel
;******************************************************************************
drawboxs:
	VERA_SET_STRIDE 2
	ldy	#0		; Gotoxy 0, 0
	lda	#0
	VTUI_GOTOXY
	lda	scrwidth
	ldx	twopanels	; If 0, then panel width = screenwidth
	beq	:+
	lsr			; If not 0, then panel width = screenwidth/2
:	sta	r1l
	lda	scrheight
	dec
	sta	r2l
	lda	#2
	VTUI_BORDER
	lda	scrheight	; Calculate 4 lines up from bottom
	sec
	sbc	#4
	tay
	lda	#0
	VTUI_GOTOXY
	lda	#RIGT_TEE	; Draw a horizontal line to be used as 
	VTUI_PLOT_CHAR		; full-length display of filename
	lda	#HOR_LINE
	ldy	r1l		; Loading panel width
	dey
	dey
	VTUI_HLINE
	lda	#LEFT_TEE
	VTUI_PLOT_CHAR
;******************************************************************************
; Draw the box around the second panel
;******************************************************************************
drawbox2:
	ldy	#0
	lda	panel2x
	VTUI_GOTOXY
	lda	panel2x
	sta	r1l
	lda	scrheight
	dec
	sta	r2l
	lda	#2
	VTUI_BORDER
	lda	scrheight	; Calculate 4 lines up from bottom
	sec
	sbc	#4
	tay
	lda	panel2x
	VTUI_GOTOXY
	lda	#RIGT_TEE	; Draw a horizontal line to be used as
	VTUI_PLOT_CHAR		; full-length display of filename
	lda	#HOR_LINE
	ldy	r1l		; panel width
	dey
	dey
	VTUI_HLINE
	lda	#LEFT_TEE
	jmp	vtui_plot_char

;******************************************************************************
; Get the current screen mode and write the width and height to the
; global variables scrwidth & scrheight
; Store the cursor width as well
;******************************************************************************
getscreenmode:
	stz	twopanels	; Assume only 1 panel is shown at a time
	stz	scrollval	; SCR32_SCROL_VAL
	sec
	jsr	Screen_mode
	stx	scrwidth
	sty	scrheight

	; switch (scrwidth)
@case20:cpx	#20		; case: 20
	bne	@case22
	lda	#SCR20_SCROL_VAL
	sta	scrollval
	lda	#SCR20_CURSOR_WIDTH
	sta	cursorwidth
	bra	:+		; Do the same as in case 22
@case22:cpx	#22		; case: 22
	bne	@case32
	lda	#SCR22_SCROL_VAL
	sta	scrollval
	lda	#SCR22_CURSOR_WIDTH
	sta	cursorwidth
:	lda	#3		; There are 4 (0-based) lines of menus
	sta	maxhotkeymenu
	bra	@switchend	; break
@case32:cpx	#32		; case: 32
	bne	@case40
	lda	#SCR32_CURSOR_WIDTH
	sta	cursorwidth
	lda	#2		; There are 3 (0-based) lines of menus
	sta	maxhotkeymenu
	bra	@switchend	; break
@case40:cpx	#40		; case: 40
	bne	@case64
	lda	#SCR40_CURSOR_WIDTH
	sta	cursorwidth
	lda	#1		; There are 2 (0-based) lines of menu
	sta	maxhotkeymenu
	sta	twopanels	; Show 2 panels
	bra	@switchend	; break
@case64:cpx	#64		; case: 64
	bne	@default
	lda	#SCR64_CURSOR_WIDTH
	sta	cursorwidth
	bra	@switchend	; break
@default:			; default
	stz	maxhotkeymenu	; There are 1 (0-based) menu line
	inc	twopanels	; Show 2 panels
	lda	#SCR80_CURSOR_WIDTH
	sta	cursorwidth
@switchend:			; break

	stx	panel2x		; Assume panel2 is outside current screen
				; (X resolution < 40) start x position is then
				; the same as the screen width.

	lda	#1		; Assume 40 columns
	sta	maxhotkeymenu
	cpx	#40		; is screen width 40 ?
	bcc	@below
	; 40 columns or more
	beq	:+
	dec	maxhotkeymenu	; More than 40 columns so only 1 menu line
:	inc	twopanels
	txa			; Move screen width to a for calculations
	lsr			; divide by 2
	sta	panel2x		; That is the start x coordinate for panel2
	rts
@below:
	; less than 40 columns
	cpx	#32		; Is the width 32 columns?
	bcc	@width2x	; If not, it is 20 or 22
	lda	#2
	sta	maxhotkeymenu
	rts
@width2x:
	lda	#3
	sta	maxhotkeymenu
	rts

; 20&22 width menu have 9MORE right most
; F1-F8 can be displayed two at a time
;1HELP 2RENMOV
;3EDIT 4MKDIR
;5COPY 6DELETE
;7PULLDN 8QUIT

; 32 width
; 1HELP 2RENMOV 3EDIT  9MORE
; 4MKDIR 5COPY 6DELETE 9MORE
; 7PULLDN 8QUIT        9MORE

; 40 width
; 1HELP 2RENMOV 3EDIT 4MKDIR  9MORE
; 5COPY 6DELETE 7PULLDN 8QUIT 9MORE

; >=64 width
; 1HELP 2RENMOV 3EDIT 4MKDIR 5COPY 6DELETE 7PULLDN 8QUIT

;******************************************************************************
; Update hotkey line at bottom of screen
;******************************************************************************
updatehotkeys:
	lda	#0
	ldy	scrheight
	dey
	VTUI_GOTOXY
	lda	#1		; Set stride to one to ensure bgcolor is written
	VTUI_SET_STRIDE
	lda	maxhotkeymenu	; This tells us how many lines needs to be cycled through
	cmp	#3		; Value of 3 = mode width 20 or 22 columns
	bne	@is32		; Branch to check if mode is 32 column width
	; Width = 20 or 22
	lda	curhotkeymenu	; This tells the menu line to show
	bne	@ismenu1_20_22
	; Menu line 0
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk1
	jsr	printhk2
	jmp	@end
@ismenu1_20_22:			; Are we showing menu line 1?
	cmp	#1
	bne	@ismenu2_20_22
	; Menu line 1
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk3
	jsr	printhk4
	ldx	txtcolor
	lda	#' '
	VTUI_PLOT_CHAR
	jmp	@end
@ismenu2_20_22:			; Are we showing menu line 2?
	cmp	#2
	bne	@ismenu3_20_22
	; Menu line 2
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk5
	jsr	printhk6
	jmp	@end
@ismenu3_20_22:			; Then we must be showing menu line 3!
	; Menu line 2
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk7
	jsr	printhk8
	jmp	@end
@is32:
	cmp	#2		; Value of 2 = mode width 32 columns
	bne	@is40
	; width = 32
	lda	curhotkeymenu	; This tells the menu line to show
	bne	@ismenu1_32
	; Menu line 0
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk1
	jsr	normspace
	jsr	printhk2
	jsr	normspace
	jsr	printhk3
	jsr	normspace
	jsr	normspace
	jmp	@end
@ismenu1_32:			; Are we showing menu line 1?
	cmp	#1
	bne	@ismenu2_32
	; Menu line 1
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk4
	jsr	normspace
	jsr	printhk5
	jsr	normspace
	jsr	printhk6
	jsr	normspace
	bra	@end
@ismenu2_32:			; Then we must be showing menu line 2!
	; menu line 2
	jsr	:+
	lda	panel2x		; Start X for panel 2
	asl
	sta	VERA_ADDR_L
:	jsr	printhk7
	jsr	normspace
	jsr	printhk8
	ldy	#9		; As this menu line is shorter than the others
	ldx	txtcolor	; we print a series of spaces with blue background
:	lda	#' '		; to erase any previous text
	VTUI_PLOT_CHAR
	dey
	bne	:-
	bra	@end
@is40:
	cmp	#1		; Valuie of 1 = mode width 40 columns
	bne	@longmenu
	; width = 40
	lda	curhotkeymenu	; This tells the menu line to show
	bne	@ismenu1_40
	; Menu line 0
	jsr	printhk1
	jsr	normspace
	jsr	printhk2
	jsr	normspace
	jsr	printhk3
	jsr	normspace
	jsr	printhk4
	jsr	normspace
	jsr	normspace
	bra	@end
@ismenu1_40:			; Here we must be showing menu line 1!
	; Menu line 1
	jsr	printhk5
	jsr	normspace
	jsr	printhk6
	jsr	normspace
	jsr	printhk7
	jsr	normspace
	jsr	printhk8
	jsr	normspace

@end:	ldx	txtcolor
	lda	#'9'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<morestr
	sta	r0l
	lda	#>morestr
	sta	r0h
	lda	#0
	ldy	#MORESTRLEN
	jmp	vtui_print_str

@longmenu:			; Value of 0 = mode width = 80 columns
	jsr	printhk1
	jsr	normspace
	jsr	printhk2
	jsr	normspace
	jsr	printhk3
	jsr	normspace
	jsr	printhk4
	jsr	normspace
	jsr	printhk5
	jsr	normspace
	jsr	printhk6
	ldx	txtcolor
	lda	#' '
	VTUI_PLOT_CHAR
	jsr	printhk7
	jsr	normspace
	jsr	printhk8
	rts

;******************************************************************************
; Set sane VERA defaults and clear the screen with txtcolor
;******************************************************************************
clrscr:
	lda	#$11		; Stride=1, Decr=0 & Bank=1
	sta	VERA_ADDR_H
	ldx	txtcolor
	lda	#' '
	jmp	vtui_clr_scr

;******************************************************************************
; Helper functions for updatehotkeys to display the hotkey menu line at the bottom
; of the screen
;******************************************************************************
normspace:
	ldx	txtcolor
	lda	#' '
	jmp	vtui_plot_char


printhk1:
	ldx	txtcolor
	lda	#'1'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<helpstr
	sta	r0l
	lda	#>helpstr
	sta	r0h
	lda	#0
	ldy	#HELPSTRLEN
	jmp	vtui_print_str

printhk2:
	ldx	txtcolor
	lda	#'2'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<renmovstr
	sta	r0l
	lda	#>renmovstr
	sta	r0h
	lda	#0
	ldy	#RENMOVESTRLEN
	jmp	vtui_print_str

printhk3:
	ldx	txtcolor
	lda	#'3'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<editstr
	sta	r0l
	lda	#>editstr
	sta	r0h
	lda	#0
	ldy	#EDITSTRLEN
	jmp	vtui_print_str

printhk4:
	ldx	txtcolor
	lda	#'4'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<mkdirstr
	sta	r0l
	lda	#>mkdirstr
	sta	r0h
	lda	#0
	ldy	#MKDIRSTRLEN
	jmp	vtui_print_str

printhk5:
	ldx	txtcolor
	lda	#'5'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<copystr
	sta	r0l
	lda	#>copystr
	sta	r0h
	lda	#0
	ldy	#COPYSTRLEN
	jmp	vtui_print_str

printhk6:
	ldx	txtcolor
	lda	#'6'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<deletestr
	sta	r0l
	lda	#>deletestr
	sta	r0h
	lda	#0
	ldy	#DELETESTRLEN
	jmp	vtui_print_str

printhk7:
	ldx	txtcolor
	lda	#'7'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<pulldnstr
	sta	r0l
	lda	#>pulldnstr
	sta	r0h
	lda	#0
	ldy	#PULLDNSTRLEN
	jmp	vtui_print_str

printhk8:
	ldx	txtcolor
	lda	#'8'
	VTUI_PLOT_CHAR
	ldx	hotkeycol
	lda	#<quitstr
	sta	r0l
	lda	#>quitstr
	sta	r0h
	lda	#0
	ldy	#QUITSTRLEN
	jmp	vtui_print_str
	
VIDEO_INC_END:
.endif