.ifndef FILE_INC
FILE_INC=1
	jmp	FILE_INC_END
foffset		= video_variables_end
devid1		= foffset+0
devid2		= foffset+1
part1		= foffset+2
part2		= foffset+3
activedev	= foffset+4
activepart	= foffset+5
activefileid	= foffset+6
activepathaddr	= foffset+7 ; 2 bytes - VERA BANK will always be 1
file1offset	= foffset+9
file2offset	= foffset+10

file_variables_end = foffset+11

listdir:
	.byte	"$=l"
LISTDIRLEN	= *-listdir
curdir:
	.byte	"$=c"
CURDIRLEN	= *-curdir
changedir:
	.byte	"cd:"
CHANGEDIRLEN	= *-changedir
listpart:
	.byte	"$=p"
LISTPARTLEN	= *-listpart
changepart:
	.byte	"cp "
CHANGEPARTLEN	= *-changepart

FILEID1		= $10		; File ID for panel1 dir listing
FILEID2		= $20		; File ID for panel2 dir listing

PATH1ADDR	= $1EC00
PATH2ADDR	= $1EE00
PATHTMP		= $1F9BF

discardtoquote:
	jsr	CHRIN		; Read character and save it in .Y
	tax			; while reading error status
	jsr	READST		; If no error, we can continue
	cmp	#0
	beq	:+
	sec			; Otherwise, set .C and exit
	rts
:	cpx	#'"'		; Have we found a quote?
	bne	discardtoquote	; If not, keep reading
	clc
	rts	

;******************************************************************************
; Skip the header of directory-/partition-/currentdir- listing by reading from
; an already opened input stream until all bytes of the header is read.
; This is done by discarding all bytes until the first quote and then
; continuing until a $00 is read.
;******************************************************************************
; OUTPUT:	.C set on error
;		.A READST-error code if error otherwise 0
;******************************************************************************
skipheader:
	jsr	discardtoquote
	bcc	@zero
	rts
@zero:
	jsr	CHRIN		; Read character and save it in .Y
	tax			; while reading error status
	jsr	READST		; If no error, we can continue
	cmp	#0
	beq	:+
	sec			; Otherwise, set .C and exit
	rts
:	cpx	#0		; Have we found a $00 ?
	bne	@zero		; If not, keep reading
	clc			
	rts

;******************************************************************************
; Get current directory and write it to the path addr of the currently
; selected panel. 
;******************************************************************************
; USES:		All registers
;		TMP_PTR0
;		global buffer
;******************************************************************************
getcurdir:
	ldx	#<PATHTMP		; Point VERA_DATA1 to PATHTMP address
	ldy	#>PATHTMP
	lda	#$19			; Decrement by 1 on VERA_DATA1 access
	jsr	set_vera1_addr
	stz	VERA_DATA1		; 0-termination of reversely written string

	lda	#<buffer		; Store buffer address in TMP_PTR0
	sta	TMP_PTR0
	lda	#>buffer
	sta	TMP_PTR0+1

	lda	activefileid		; Open device/drive for reading
	ldx	activedev
	ldy	#0
	jsr	SETLFS
	lda	#CURDIRLEN		; $=L
	ldx	#<curdir
	ldy	#>curdir
	jsr	SETNAM
	jsr	OPEN
	bcc	:+	
	rts				; Exit on error
:	; Now file is open and we need to remember to close it
	; before exiting this function
	ldx	activefileid		; Ensure channel is input
	jsr	CHKIN

	jsr	skipheader
	bcs	@end

@dirloop:
	ldy	#0			; Reset buffer counter
	jsr	discardtoquote		; Find next quote
	bcs	@end
@nextchar:
	jsr	CHRIN			; Read a char of dir-name and store in .X
	tax
	jsr	READST			; Read error code and exit if error
	cmp	#0
	beq	:+
	sec
	bra	@end
:	cpx	#'"'			; If we reach quote, current directory
	beq	@dirdone		; name is complete
	txa				; Store char of dir-name in buffer
	sta	(TMP_PTR0),y
	iny
	bra	@nextchar
@dirdone:
	lda	#0			; Terminate the dir-name with 0
	sta	(TMP_PTR0),y
	ldx	#<buffer		; Reverse the dir-name
	ldy	#>buffer
	jsr	strrev

	lda	#'/'			
	sta	VERA_DATA1
	ldy	#0			; Reset buffer counter
@vloop:	lda	(TMP_PTR0),y		; Copy buffer to VRAM
	beq	@dirloop		; If char is 0, this dirname is done
	cmp	#'/'			; Skip dir-name if it is /
	beq	:+
	sta	VERA_DATA1
:	iny
	bra	@vloop

@end:	pha				; Save possible error code
	; Copy temp VRAM buffer to real VRAM path addr
	lda	VERA_CTRL		; Save VERA_CTRL
	pha
	ora	#$01			; ADDRSEL=1
	sta	VERA_CTRL
	lda	#$11			; Increment 1 on VERA_DATA1 access
	sta	VERA_ADDR_H
	pla				; Restore VERA_CTRL
	sta	VERA_CTRL
	lda	VERA_DATA1		; Move to starting byte of path

	lda	#$11			; Increment 1 on VERA_DATA0 access
	sta	VERA_ADDR_H
	lda	activepathaddr		; Set VERA_DATA0 to activepathaddr
	sta	VERA_ADDR_L
	lda	activepathaddr+1
	sta	VERA_ADDR_M

:	lda	VERA_DATA1		; Copy temp VRAM buffer to PATH ADDR
	beq	:+
	sta	VERA_DATA0
	bra	:-

:	stz	VERA_DATA0		; 0-terminate the path string
	lda	activefileid		; Close device
	jsr	CLOSE
	jsr	CLRCHN
	pla				; Restore possible error code and save it
	pha				; again so it can be returned to caller
	sec				; Assume error
	and	#64			; If End Of File
	beq	:+			; Then no error
	clc				; so reset .C
:	pla				; Restore possible error code
	rts

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
	jsr	@doclose
	sec			; Show that drive is not present
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
	; Check if it is HOSTFS
	; switch (1st char of fs type)
	cmp	#'h'			; Case 'H'
	bne	@fat32
	jsr	CHRIN
	; switch (2nd char of fs type)
	cmp	#'o'			; Case 'O'
	bne	@default
	jsr	CHRIN
	; switch (3rd char of fs type)
	cmp	#'s'			; Case 'S'
	bne	@default
	jsr	CHRIN
	; Switch (4th char of fs type)
	cmp	#'t'			; Case 'T'
	bne	@default
	jsr	CHRIN
	; switch (5th char of fs type)
	cmp	#' '			; Case ' '
	bne	@default
	lda	#0			; Show that it is HOSTFS
	bra	@switchend
	; Check if it is FAT32
@fat32:	cmp	#'f'			; Case 'F'
	bne	@default
	jsr	CHRIN
	; switch (2nd char of fs type)
	cmp	#'a'			; Case 'A'
	bne	@default
	jsr	CHRIN
	; switch (3rd char of fs type)
	cmp	#'t'			; Case 'T'
	bne	@default
	jsr	CHRIN
	; switch (4th char of fs type)
	cmp	#'3'			; Case '3'
	bne	@default
	jsr	CHRIN
	; switch (5th char of fs type)
	cmp	#'2'			; Case '2'
	bne	@default
	lda	#1			; Show that it is FAT32
	bra	@switchend
@default:
	lda	#2			; Show that it is OTHER
@switchend:
	pha				; Save FS type while closing input stream
	jsr	@doclose
	pla
	clc				; Ensure drive presence is correct
	rts
@doclose:
	lda	#15
	jsr	CLOSE
	jsr	CLRCHN
	rts

FILE_INC_END:
.endif
