# 404-Assembler-and-VM

In a recent class I was asked to write an assembler and virtual machine in C for an assembly language designed for the assignment; while this language doesn't have any readily-accessible documentation available to non-students, it is effectively similar to many existing, in-use frameworks.

In lieu of providing the documentation I have included a short text file delineating a few operations, as well as a longer test file of unit tests for each instruction (intended to root out various bugs within each operation).

To translate the input into a usable bin file, simply pass the input file and the desired name of the output file (i.e. FILE_NAME.bin) respectively as command line arguments.

Finally, with the VM in the same directory as the assembler/output file, simply run the VM with the output file above as the sole command line argument.

The final modification to this assignment involved adding pipelined functionality to the VM; for the most part this functionality works, but some bugs remain in instances where the instructions are loaded out of order (particularly in the load/store/branching instructions). As such, I've also included the penultimate iteration of the VM, which does not yet include pipelining, but does perform all of the operations correctly.
