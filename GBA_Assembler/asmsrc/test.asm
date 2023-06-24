//This is a comment

ENTRY:
ADC R3         R1
	   AND R0, R0
	   AND R1, R1
label:							ADC R1, R1
	   AND R2, R2

Bytes:          {   FF ,  FF    ,  00,  99		}

//This is an another comment