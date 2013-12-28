basic-like-interpreter
======================

This is a interpreter that runs basic-like-language in microcontrollers like LPC1114FN28/102.

This software is released under the NYSL Version 0.9982.
( see: http://www.kmonos.net/nysl/ )

How to use
----------

1. Implement _sbrk() or sbrk() to use malloc().
2. Add statements in output() to be able to output.
3. Call run() with BASIC-Like-Language instructions.

Available Instructions
---------------------

PRINT,DIM,GOSUB,RETURN,IF,WHILE,WEND

Limitation
----------

Variable type must be real number. (double)

Variable identifier must be within 15 characters.

Instruction (with argument) must be within 127 characters.

and more...
