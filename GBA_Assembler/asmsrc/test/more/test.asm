//This is a comment
~join "test/test2.asm"
ENTRY:
ADC R3         R1
	   BIC R0, R0
	   TST R1, R1
label:							MUL R1, R1
	   B ENTRY

Bytes:          {   FF ,  FF    ,  00,  99		}

//This is an another comment