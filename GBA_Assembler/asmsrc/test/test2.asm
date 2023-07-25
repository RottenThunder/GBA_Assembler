//This is a comment

AnotherLabel:
ADC R3         R1
	   BIC R0, R0
	   TST R1, R1
AndAnotherOne:							MUL R1, R1
MOVA R3, &AnotherLabel
	   CALL ENTRY

//This is an another comment