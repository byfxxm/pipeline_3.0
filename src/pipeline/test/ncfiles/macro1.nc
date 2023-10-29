#1 = 1
#2 = 20
IF NOT [#1 GE #2] THEN
	#3 = 3
	
	WHILE [#1 LT #2] DO
		#1 = #1 +1
		#3 = #3 + 1
		IF #1 LT #3 THEN
			#10 = 234.5
			#20 = MAX[#MIN[1,2,3], 3.5, 10]
			WHILE [#20 LT 25] DO
				#20 = 1 + #20
			END
			#100 = 100
			IF #20 EQ 25 THEN
				#[#20 ]= #100
			ENDIF
		ENDIF

		G1 X#3
	END
	#5=5
ELSE
	#4 = 4
ENDIF