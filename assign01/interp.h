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
  Environment *env;
  std::set<std::string> set;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:

  // Evaluate expression of a given node
  Value ex(Node *ast);

  // Perform the associated operation
  Value doOp(int tag, int op1, int op2, Node *divisor);

  // Recursively analyze AST for semantic errors
  void analyze_recurse(Node *ast);
  // TODO: private member functions
};

#endif // INTERP_H
