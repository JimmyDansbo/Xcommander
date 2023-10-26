.ifndef FILE_INC
FILE_INC=1
	jmp	FILE_INC_END
foffset		= video_variables_end

file_variables_end = foffset+0

listdir:
	.byte	"$=l"
LISTDIRLEN	= *-listdir
curdir:
	.byte	"$=c"
CURDIRLEN	= *-curdir
listpart:
	.byte	"$=p"
LISTPARTLEN	= *-listpart


FILE_INC_END:
.endif
