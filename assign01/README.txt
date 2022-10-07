My implementation is relatively simple. From the starter code, I built more
support for the requested productions by adding more tokens that we wish to
analyze, for example > and &&, many of which employ a cascading switch case
block for the special 2-character operations. From here, I added and changed
multiple parse statements to parser2.cpp, which consume the tokens accordingly
and gradually build an ast, which is to be analyzed in interp.cpp. Interp.cpp works
by both analyzing the ast, recursively searching for an undefined VARREF by adding
all of the definitions to a set of strings and checking if any VARREF is not a
member of that set and raising an error if that does happen to be the case. If not, then the ast is
semantically valid and we are free to evaluate it. Evaluation is also done recursively, attempting to
apply the specified operation to its operands, which are also recursively evaluated until we
reach the base case of an INT_LITERAL or a VARREF, of which the INT_LITERAL can be evaluated directly
and the VARREF can be searched for from the Environment object, which is a wrapper for mappings
of VARREFs to their associated values. The last statement in the ast is also printed as the
result.

As a continuation of the previous README, my implementation remains simple.
After adding the new productions associated with A2M1, I was able to build
new ASTs with control flow structures, short circuiting, and intrinsic function
calls to print() and println(). The control flow keywords (if, else, while) are very
similar, in that the condition is executed as is any Statement in the interpreter,
and a 'true' integer value (int != 0) executes the Statements in the block, and a 'false'
value (int == 0) does not. Note that non-numeric values raise an error. The same premise
occurs for while loops, but with the caveat that it executes until the condition evaluates
to false. An else statement may (only) be triggered if an if statement condition evaluates
to false. Note that for each of these, entering the body of the control flow unit creates a
new block, for which a new environment must be created. These blocks/environments may be nested
as each environment contains a pointer to its parent, so we are able to traverse up the entire
environment list should we need to. This comes in handy for identifying scope of a variable. Variables
are found by recursing up the environment parent pointers and returning the value associated with the
variable. The short circuiting is accomplished by checking the first operand of the logical operators
(&& and ||), and returning the appropriate value (0, 1) if we do not need to check the second
operand. The intrinsic functions exist as variables in the environment, and have their function calls
defined in the interpreter, and bound to the keyword in the environment. Other variables may actually
reference these functions, using the assignment operator.

For Assignment 2 Milestone 2, the user defined functions as well as readint() intrinsic function calls
were implemented. The readint reads an integer from user input using the scanf() command, and returns the
input value to the interpreter. The user defined functions store the Statement list in its value and is used/
interpreted when the function is evaluated. A function call environment is created for each function from its
parent environment, which is used during each call.