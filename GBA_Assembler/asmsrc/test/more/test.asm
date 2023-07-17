//This is a comment
~join "test/test2.asm"
ENTRY:
ADC R3         R1
	   BIC R0, R0
	   TST R1, R1
	   LSL R4, R5
	   ASR R4, R4, #27
	   MOVA R6, &label
	   MOV Ra, R6
label:							MUL R1, R1
MOVA R5, &Generic
	   B ENTRY

Bytes:          {   FF ,  FF    ,  00,  99		}

//This is an another comment