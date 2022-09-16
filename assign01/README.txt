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