//This is a comment
~join "test/test2.asm"
~align "8"
ENTRY:			MOVA R1, &MEM_IO
				MOVA R0, &Bytes
				LDRH R2, R0, #2
				STRH R2, R1, #0
				MOVA R1, &MEM_VRAM
				LDRH R2, R0, #0
				STRH R2, R1, #0
				LDRH R2, R0, #1
				STRH R2, R1, #1
FunctionCall:   CALL Function
				ADD R0, R1, #7
				ADDHI R0, R0
				ADDHI R0, RC
				SUB R0, #230
				B ENTRY
Function:		MOV R5, #255
				RETURN

MoreBytes: {BB, BB, BB, BB, BB, BB, BB, BB, BB}
Bytes:          {   0A ,  0A    ,  0F,  0F, 03, 04		}

//This is an another comment