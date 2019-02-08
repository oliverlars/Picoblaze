# A picoblaze virtual machine and compiler


##How to use the compiler and virtual machine

To use the compiler run
`picoblaze <name of source> <name of binary output file>`

This will produce a binary file which will be interpreted by the picoblaze virtual machine.

To run the program in the virtual machine, run the following command

`picoblaze_vm <name of binary output file>`

This will then run the program and once finished display the stack and ram locations

```
start:
	load s0, 0x34
	load s1, 0x12
	load s2, 0x78
	load s3, 0x56
	
	load sF, 0x10
	output s0, (sF)
	add sF, 1
	output s1, (sF)
	add sF, 1
	output s2, (sF)
	add sF, 1
	output s3, (sF)
	add s15, 1
	
	load s4, 0x10
	load s5, 0x10

loop:
	input sF, (s5)
	add s5, 1
	input sE, (s5)
	load sD, sE
	sub sD, sF
	jump nc, skip
	
	output sF, (s5)
	sub s5, 1
	output sE, (s5)
	add s5, 1

skip:
	load sF, s5
	sub sF, 0x13
	jump nz, loop
	
	load s5, 0x10
	add s4, 1
	load sF, s4
	sub sF, 0x14
	jump nz, loop

trap: 
	halt
```
The code listing is a bubble sort, from my hardware labs

```
STACK-------------------------
034 012 078 056 014 010 000 000 000 000 000 000 000 022 078 000

RAM---------------------------
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
012 034 056 078 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000
```

The output from the picoblaze virtual machine using the bubble sort code