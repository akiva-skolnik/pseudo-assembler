# Assembler
This is an assembler for an imaginary computer and imaginary assembly language.
(OpenU course project - https://www.openu.ac.il/courses/20465.htm)

**Hardware:**
The computer has 16 registers: r0..r15 + PSW (program status word). Each register has 20 bits.
Memory size: 8192*20 bits.

**Software:**
There are 16 commands, each of which has funct and opcode (for more info, check the PDF file)

**Input:** `.as` files - Assembly-like source code. (the arguments are passed without the extension)

**Output for each file:**
- `.am` file (with no macros)
- `.obj` file, with the machine code
- `.ext` file, with details about the `.extern` labels, if any.
- `.ent` file, with details about the `.entry` labels, if any.

Score: 90%