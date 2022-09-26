#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"

#include <set>

class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:

  // Evaluate expression of a given node
  Value ex(Node *ast, Environment *env);

  // Find associated environment for a var
  Environment* findEnv(Node *ref, Environment *env);
  
  // Perform the associated operation
  Value doOp(int tag, int op1, int op2, Node *divisor);

  // Recursively analyze AST for semantic errors
  void analyze_recurse(Node *ast, Environment *env);
  // TODO: private member functions

  // Intrinsic function calls
  static Value intrinsic_print(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_println(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);

  // Check if node is non numeric
  bool non_numeric(Node *ast, Environment *env);
};

#endif // INTERP_H
