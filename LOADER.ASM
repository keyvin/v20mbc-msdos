bits 16
ORG 0
MOV AX,1000

MOV AL,087H  ;MOUNT SD VOLUME
OUT 01H,AL
IN  AL,00H

MOV AL,09H ;LOAD DISK 0 
OUT 01H,AL
MOV AL,00H
OUT 00H,AL

MOV AX,00C0H
MOV DS,AX;
MOV DI,0
MOV CX,60   ;sectors to copy (probably way too many)
MOV DX,61   ;starting logical sector of io.sys 
MOV AL,01H  ;print ! to the console       
OUT 01h,al
mov al,33
out 00h,al

POSITION_DISK:                     
	MOV     SI,DX
	MOV 	AX,dx   ;uses default 
	MOV     Bx,32   ;32 sectors per track
	xor     dx,dx
	DIV 	BX
	MOV     BX,AX   ;Remainder is sector
	MOV     AL,0Bh   ;Sector
	OUT     01H,AL
	MOV     AL,DL   ;remainder in DL
	OUT	00H,AL
	MOV     AL, 0AH  ;track IS TWO BYTES, lsb FIRST
	OUT     01H, Al
	MOV     AL,BL  
	OUT 	00H,AL  
	mov     AL,BH	
	OUT     00H, AL
	MOV     DX,SI	
	MOV 	AL,086H      ;READ SECTOR
	OUT 	01H,AL
	MOV 	BX,512
DSK_SR:	
   	in  al,00h
	MOV [DI],AL
	INC DI
	DEC BX
	JNZ DSK_SR	
	DEC CX
	JZ E_DRW         ;DONE
	INC DX
	JMP POSITION_DISK
E_DRW:
	MOV AL,01H
	OUT 01h,al
	mov al,34          ;print " to signify load complete
	out 00h,al
	JMP  00c0h:0       ;jump to bios segment