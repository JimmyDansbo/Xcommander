.ifndef FILE_INC
FILE_INC=1
	jmp	FILE_INC_END
foffset		= video_variables_end
devid1		= foffset+0
devid2		= foffset+1
part1		= foffset+2
part2		= foffset+3

file_variables_end = foffset+4

listdir:
	.byte	"$=l"
LISTDIRLEN	= *-listdir
curdir:
	.byte	"$=c"
CURDIRLEN	= *-curdir
listpart:
	.byte	"$=p"
LISTPARTLEN	= *-listpart

;******************************************************************************
; Function that checks if a drive is present and returns the FS type if a
; a drive is present
;******************************************************************************
; INPUT:	.X = Drive/device ID (8-15)
; OUTPUT:	.C = 0 if drive present
;		.A: 0=Host FS, 1=FAT32, 2=Other
;******************************************************************************
drivepresent:
	lda	#15		; File ID
				; X is already set to drive ID
	ldy	#0		; Secondary command ID
	jsr	SETLFS
	lda	#LISTDIRLEN
	ldx	#<listdir
	ldy	#>listdir
	jsr	SETNAM
	jsr	OPEN		; OPEN sets .C on error
	bcc	@continue
	rts			; Exit with .C=1 (drive does not exist)
@continue:
	; In emulator, OPEN does not set .C for devices that are not present
	; instead it outputs "garbage" when reading a non-existing device
	; Garbage is usually a string of spaces
	ldx	#15		; Set file id as input
	jsr	CHKIN
	jsr	CHRIN		; If first byte is $01, it is a valid filesystem
	cmp	#01		; If the byte is $01, drive is present
	beq	@valid
	; Drive not present
	jsr	@end
	sec
	rts
@valid:
	; Read until first " character
:	jsr	CHRIN
	cmp	#$22
	bne	:-
	; Read the title of the filesystem and discard it
:	jsr	CHRIN
	cmp	#$22
	bne	:-
	jsr	CHRIN		; Discard the space before information
	jsr	CHRIN		; Read 1st char of filesystem type
	; switch (1st char of fs type)
@caseH:	cmp	#'h'
	bne	@caseF
	jsr	CHRIN
	; switch (2nd char of fs type)
@caseO: cmp	#'o'
	bne	@default
	jsr	CHRIN
	; switch (3rd char of fs type)
@caseS: cmp	#'s'
	bne	@default
	jsr	CHRIN
	; Switch (4th char of fs type)
@caseOT:cmp	#'t'
	bne	@default
	jsr	CHRIN
	; switch (5th char of fs type)
@caseSP:cmp	#' '
	bne	@default
	lda	#0
	bra	@switchend

@caseF:	cmp	#'f'
	bne	@default
	jsr	CHRIN
	; switch (2nd char of fs type)
@caseA:	cmp	#'a'
	bne	@default
	jsr	CHRIN
	; switch (3rd char of fs type)
@caseT:	cmp	#'t'
	bne	@default
	jsr	CHRIN
	; switch (4th char of fs type)
@case3:	cmp	#'3'
	bne	@default
	jsr	CHRIN
	; switch (5th char of fs type)
@case2:	cmp	#'2'
	bne	@default
	lda	#1
	bra	@switchend
@default:
	lda	#2
@switchend:
	pha
	jsr	@end
	pla
	clc
	rts
@end:	lda	#15
	jsr	CLOSE
	jsr	CLRCHN
	rts

FILE_INC_END:
.endif
