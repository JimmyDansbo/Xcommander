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
buffer		= moffset+3 ; 41 bytes - 40 bytes buffer + 0 termination
main_vars_end	= moffset+44	; temporary variable that can just bre replaced
				; if more variables is needed. It is only to show
				; where the next variable should start.
				; It also shows how much memory is allocated to
				; variables

;******************************************************************************
main:
	jsr	initvars
	jsr	getcurdir
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
	ldx	hilightcol
	jsr	updatecursor

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

	; Clear screen
	lda	#PET_CLEAR
	jsr	CHROUT

	rts

;******************************************************************************
; Switch the active variable set between the two panels
;******************************************************************************
; INPUT:	.C - 1 = switch to panel1, 0 = switch to panel2
;******************************************************************************
swapvars:
	bcs	@activate_panel1
@activate_panel2:
	lda	panel2x
	inc
	sta	activex
	lda	activey			; Save panel1 cursor Y position
	sta	cursor1y
	lda	cursor2y		; Retrieve panel2 cursor Y position
	sta	activey
	lda	activedev		; Save panel1 device id
	sta	devid1
	lda	devid2			; Retrieve panel2 device id
	sta	activedev
	lda	activepart		; Save panel1 partition id
	sta	part1
	lda	part2			; Retrieve panel2 partition id
	sta	activepart
	lda	#FILEID2		; Set panel2 file id
	sta	activefileid
	lda	activefileoffset	; Save panel1 file offset
	sta	file1offset
	lda	file2offset		; Retrieve panel2 file offset
	sta	activefileoffset
	rts
@activate_panel1:
	lda	#1
	sta	activex
	lda	activey			; Save panel2 cursor Y position
	sta	cursor2y
	lda	cursor1y		; Retrieve panel1 cursor Y position
	sta	activey
	lda	activedev		; Save panel2 device id
	sta	devid2
	lda	devid1			; Retrieve panel1 device id
	sta	activedev
	lda	activepart		; Save panel2 partition id
	sta	part2
	lda	part1			; Retrieve panel1 partition id
	sta	activepart
	lda	#FILEID1		; Set panel1 file id
	sta	activefileid
	lda	activefileoffset	; Save panel2 file offset
	sta	file2offset
	lda	file1offset		; Retrieve panel1 file offset
	sta	activefileoffset
	rts

;******************************************************************************
; SET VERA DATA1 address while preserving VERA_CTRL
;******************************************************************************
; INPUTS:	.A = ADDR_H
;		.X = ADDR_L
;		.Y = ADDR_M
; USES:		.X & .A
;******************************************************************************
set_vera1_addr:
	pha				; Save ADDR_H
	lda	VERA_CTRL		; Save CTRL register so it can be
	pha				; Restored again
	ora	#$01			; Set ADDRSEL to 1
	sta	VERA_CTRL
	
	stx	VERA_ADDR_L
	sty	VERA_ADDR_M
	plx				; Pull CTRL register
	pla				; Pull ADDR_H
	sta	VERA_ADDR_H

	stx	VERA_CTRL		; Restore CTRL register
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
; USES:		.A & r0
;******************************************************************************
strlen:
	clc
	stx	r0l		; Store string address in r0
	sty	r0h
	ldy	#0
:	lda	(r0),y	; Read bytes from string until 0-char
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
; USES:		.A & r0
;******************************************************************************
strrev:
	phx
	phy
	jsr	strlen		; Get length of string
	bcs	@end		; If longer than 256 bytes, it's an error
	cpy	#2		; No reversal if length<2
	bcc	@end
	dey
@loop:	lda	(r0),y		; Read from end of string
	pha			
	lda	(r0)		; Read from beginning of string
	sta	(r0),y		; Write to end of string
	pla
	sta	(r0)		; Write to beginning of string
	WINC	r0		; Inc r0 to go to next char
	dey			; Dec .Y to go from right to left
	beq	@end		; If not 0, do it twice as r0 has just
	dey			; been moved 1 char forward
	bne	@loop
@end:	pla
	sta	r0h
	pla
	sta	r0l
	rts

;******************************************************************************
; Initialize global variables to sane default values
;******************************************************************************
initvars:
	stz	buffer+40
	; Colors
	lda	#(BLUE<<4)|WHITE
	sta	txtcolor
	lda	#(YELLOW<<4)|BLUE
	sta	hilightcol
	lda	#(BLUE<<4)|YELLOW
	sta	headercol
	lda	#(CYAN<<4)|BLACK
	sta	hotkeycol
	; Panels, menus and coordinates
	lda	#40
	sta	panel2x
	stz	curhotkeymenu
	lda	#3			; Number of menus for 20 or 22 width
	sta	maxhotkeymenu
	lda	#1
	sta	twopanels
	sta	activex
	sta	activey
	sta	cursor1y
	sta	cursor2y
	;	#0
	stz	file1offset
	stz	file2offset
	stz	activefileoffset

	; File-/device-id's, partitions and paths
	sta	part1			; Partition# active on panel 1
	sta	part2			; Partition# actove on panel 2
	sta	activepart
	lda	#8
	sta	devid1			; DeviceID active on panel 1
	sta	devid2			; DeviceID active on panel 2
	sta	activedev
	lda	#FILEID1
	sta	activefileid
	lda	#^PATH1ADDR		; Store VERA Bank ID in .A
	ora	#$10			; and increment by 1
	ldx	#<PATH1ADDR
	stx	activepathaddr
	ldy	#>PATH1ADDR
	sty	activepathaddr+1
	jsr	set_vera1_addr
	lda	#'/'			; Set root path for panel 1
	sta	VERA_DATA1
	stz	VERA_DATA1		; Path string is 0-terminated
	lda	#^PATH2ADDR		; Store VERA Bank ID in .A
	ora	#$10			; and increment by 1
	ldx	#<PATH2ADDR
	ldy	#>PATH2ADDR
	jsr	set_vera1_addr
	lda	#'/'			; Set root path for panel 2
	sta	VERA_DATA1
	stz	VERA_DATA1		; Path string is 0-terminated
	rts
