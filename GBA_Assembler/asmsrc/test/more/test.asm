//This is a comment
//~join "test/test2.asm"
ENTRY:		MOVA R1, &MEM_IO
			MOVA R0, &Bytes
			LDRH R2, R0, #2
			STRH R2, R1, #0
			MOVA R1, &MEM_VRAM
			LDRH R2, R0, #0
			STRH R2, R1, #0
			LDRH R2, R0, #1
			STRH R2, R1, #1

Bytes:          {   0A ,  0A    ,  0F,  0F, 03, 04		}

//This is an another comment