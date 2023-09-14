.ifndef VIDEO_INC
VIDEO_INC=1

	jmp	VIDEO_INC_END
offset		= $400
scrwidth 	= offset+0
scrheight	= offset+1
txtcolor	= offset+2
hilightcol	= offset+3
headercol	= offset+4
hotkeycol	= offset+5
twopanels	= offset+6
panel2x		= offset+7
curhotkeymenu	= offset+8
maxhotkeymenu	= offset+9

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

HOR_LINL = $67
HOR_LINR = $65

helpstr:
	.byte	"help "
editstr:
	.byte	"edit "
copystr:
	.byte	"copy "
renmovstr:
	.byte	"renmov "
mkdirstr:
	.byte	"mkdir "
deletestr:
	.byte	"delete "
pulldnstr:
	.byte	"pulldn "
quitstr:
	.byte	"quit "
morestr:
	.byte	"more "
morestr_end:

HELPSTRLEN = editstr-helpstr
EDITSTRLEN = copystr-editstr
COPYSTRLEN = renmovstr-copystr
RENMOVESTRLEN = mkdirstr-renmovstr
MKDIRSTRLEN = deletestr-mkdirstr
DELETESTRLEN = pulldnstr-deletestr
PULLDNSTRLEN = quitstr-pulldnstr
QUITSTRLEN = morestr-quitstr
MORESTRLEN = morestr_end-morestr

;******************************************************************************
; Draw the box around the first panel
;******************************************************************************
drawboxs:
	lda	#2
	VTUI_SET_STRIDE
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
drawbox2:
	lda	twopanels
	bne	:+
	rts
:	ldy	#0
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
; global variables scrwidht & scrheight
;******************************************************************************
getscreenmode:
	stz	twopanels
	sec
	jsr	Screen_mode
	stx	scrwidth
	sty	scrheight
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
	jsr	printhk1
	jsr	printhk2
	jmp	@end
@ismenu1_20_22:			; Are we showing menu line 1?
	cmp	#1
	bne	@ismenu2_20_22
	; Menu line 1
	jsr	printhk3
	jsr	printhk4
	ldx	txtcolor
	lda	#' '
	VTUI_PLOT_CHAR
	jmp	@end
@ismenu2_20_22:			; Are we showing menu line 2?
	cmp	#2
	bne	@ismenu3_20_22
	; Menu line 2
	jsr	printhk5
	jsr	printhk6
	jmp	@end
@ismenu3_20_22:			; Then we must be showing menu line 3!
	; Menu line 2
	jsr	printhk7
	jsr	printhk8
	jmp	@end
@is32:
	cmp	#2		; Value of 2 = mode width 32 columns
	bne	@is40
	; width = 32
	lda	curhotkeymenu	; This tells the menu line to show
	bne	@ismenu1_32
	; Menu line 0
	jsr	printhk1
	jsr	normspace
	jsr	printhk2
	jsr	normspace
	jsr	printhk3
	jsr	normspace
	jsr	normspace
	bra	@end
@ismenu1_32:			; Are we showing menu line 1?
	cmp	#1
	bne	@ismenu2_32
	; Menu line 1
	jsr	printhk4
	jsr	normspace
	jsr	printhk5
	jsr	normspace
	jsr	printhk6
	jsr	normspace
	bra	@end
@ismenu2_32:			; Then we must be showing menu line 2!
	; menu line 2
	jsr	printhk7
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