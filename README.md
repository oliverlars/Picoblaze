# A picoblaze virtual machine and compiler


##How to use the compiler and virtual machine
```
start:
	load sE, 0x10
	load sF, 0x5
	output sF, (sE)
	input sF, (sE)
	halt
```
A simple picoblaze example program

To use the compiler run
`picoblaze <name of source> <name of binary output file>`

This will produce a binary file which will be interpreted by the picoblaze virtual machine.

To run the program in the virtual machine, run the following command

`picoblaze_vm <name of binary output file>`

this will then run the program and once finished display the stack and ram locations