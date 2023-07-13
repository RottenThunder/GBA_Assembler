//This is a comment
~join "test/test2.asm"
ENTRY:
ADC R3         R1
	   BIC R0, R0
	   TST R1, R1
	   MOV R6, &label
label:							MUL R1, R1
MOV R5, &Generic
	   B ENTRY

Bytes:          {   FF ,  FF    ,  00,  99		}

//This is an another comment