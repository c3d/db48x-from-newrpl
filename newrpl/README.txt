LEGAL:
------

This program in its entirety is released under the license present in the file LICENSE.txt.

WHAT THIS DEMO IS AND IS NOT:
----------------------------

This is a console application with a rudimentary user interface intended for testing purposes only.
This demo does not represent in any way the final user interface.

HOW TO USE IT:
-------------

The demo will show a familiar stack interface and a prompt. In the prompt, familiar RPL commands
can be typed. Special symbols can be typed simply expanded with normal characters per this list:

<< as two 'less than' symbols
>> as two 'greather than' symbols
-> as a 'minus' and a 'greater than'
(square root) as the command SQRT

To end a session, enter an empty line and will be asked confirmation to quit.

Operators have to be entered in the prompt and ENTER pressed, there's no single-keystroke
operation in this demo.

THINGS TO TRY IN THIS VERSION:
-----------------------------
* Basic numeric operations:
    This follows the familiar set of commands, with the exception of SQRT for the square root symbol.
* Basic stack operations
    Not all stack operations are implemented yet, but the most common ones are available.
* Global variables and directories
    The behavior follows the conventions: unquoted names will be RCL'd and EVAL'd automatically. Quoted
    names have to be explicitly RCL'd (and explicitly EVAL'd as needed).
    Variables RCL'd will be searched for in the current directory and all its parents.
    Variables STOred will be stored in the current directory only.
* Local variables
    The behavior follows the conventions: unquoted names will be RCL'd and EVAL'd automatically. Quoted
    names have to be explicitly RCL'd (and explicitly EVAL'd as needed).
    Variables vanish as they reach the end of their scope, as follows:
    - Variables created with the -> operator exist only within the defined << >> delimiters.
    - Variables created with LSTO within a secondary, only exist within that secondary and after the
      initial creation statement.
    - Variables created within loops will exist only within that loop
    - Variables created within a secondary will be available to all inner secondaries or external programs
      called by the creating program.
* Directories
    Normal behavior.
* Loops
    Normal behavior
* Flow control statements
    Nothing new here
* Secondaries
    There's two kinds of secondaries: normal programs use << >> delimiters, and will be pushed on the stack
    when reached within another secondary, then they must be EVAL'd explicitly for execution. The direct-execution
    secondaries use the :: ; delimiters, and when included within a secondary they will be executed directly
    and never pushed to the stack. They otherwise work as a normal secondary.
    Notice that normal secondaries will be EVAL'd automatically if they are stored in a variable, when that
    variable name is used unquoted.
* Lists
    List objects are created, but no list operations are implemented yet.
* Numbers
    Integers and real numbers are interchangeable. The compiler decides when a number is an integer and will
    automatically compile as an integer or real. From the user's perspective there's only numbers.
    Integers in bases other than 10 can be used:
        #10d is a decimal 10
        #10h is a hexadecimal number
        #10b is a binary number
        #10o is an octal number
    By default a number will be in decimal base. Real numbers can only be in decimal base. The base is a
    property of the number (rather than a system setting), and will be retained by operators when possible.
    This means a numbers in different bases can coexist in the stack. Operating on a number with a base will
    try to keep the base of the original number, even if the second operand is in a different base.
    Mixed based operations are performed correctly.
    A division might convert the number to a real number if the division result is not an integer. In that case
    the result will always be in base 10.
    Integers are signed 64-bit numbers, and an overflow will automatically convert to a real number, there is
    no wrap around. In that case the base will be changed to 10.

