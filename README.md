# Sample VM with Garbage Collector

## Execution environment

The vm has a stack, initially empty, which can contain 32-bit integer numbers. The vm reads a program in the form of bytecode. Each program has a maximum length of 65536 bytes. Each byte in the input program has an address, starting from 0.

The commands of the input program vary in length. The general form of a command is one byte op-code, and in some cases one or more bytes describing an integer, with or without a sign. Bytes forming the integer are stored in little-endian byte order.

## Supported commands

* halt (0x00): terminates the execution of the vm.
* jump (0x01): sets program counter at the address described by the next 2 bytes (unsigned integer).
* jnz (0x02): removes the first element from the stack and if it is not zero jumps at the address described by the next 2 bytes (unsigned integer).
* dup (0x03): pushes in the stack, a duplicate of the element in the i-th position of the stack, where i is the unsigned integer described by the following byte. (i.e. i = 0 introduces a duplicate of the top element in the stack).
* drop (0x04): removes from the stack the top element and ignores it.
* push4 (0x05): pushes in the stack the signed integer described by the following 4 bytes.
* push2 (0x06): pushes in the stack the signed integer described by the following 2 bytes.
* push1 (0x07): pushes in the stack the signed integer described by the following 1 bytes.
* add (0x08): removes from the top of the stack two elements, a and b, and inserts the sum a + b.
* sub (0x09): similar to add, but for the difference a - b.
* mul (0x0a): similar to add, but for the product a * b.
* div (0x0b): similar to add, but for the quotient a / b.
* mod (0x0c): similar to add, but for the remainder a % b.
* eq (0x0d): removes from the top of the stack two elements, a and b, and inserts 1 if a = b and 0 otherwise.
* ne (0x0e): similar to eq, but for the condition a != b.
* lt (0x0f): similar to eq, but for the condition a < b.
* gt (0x10): similar to eq, but for the condition a > b.
* lt (0x11): similar to eq, but for the condition a <= b.
* gt (0x12): similar to eq, but for the condition a >= b.
* not (0x13): removes an element from the top of the stack and inserts at the top 1 if that was not 0, and 0 otherwise.
* and (0x14): removes two elements from the top of the stack and inserts 1 if both were non zero and 0 otherwise.
* or (0x15): removes two elements from the top the stack and inserts 0 if both were zero, and 1 otherwise.
* input (0x16): reads a character from standar input and inserts its ascii code at the top of the stack.
* output (0x17): removes an element from the top of the stack and prints the corresponding character to standard output.
* clock (0x2a): prints the time, in seconds, elapsed since the start of the program.
* cons (0x2b): removes two elements, a and b, from the top of the stack and creates in the heap a new cons cell, which contains the these two elements, and inserts the address of the cons cell at the top of the stack.
* hd (0x2c): removes the address of a cons cell from the top of the stack and inserts the first element of the cons cell at the top of the stack.
* tl (0x2d): removes the address of a cons cell from the top of the stack and inserts the second elementof the cons cell at the top of the stack.

The execution of the program is terminated when either with a halt command, or when all available commands are executed.

## How to run the vm

After downloading this repository, run make to compile the vm and then execute the vm, giving it as its first and only argument the file of the program to execute.

```
make // to compile
./vm bytecode_program // to execute
```