#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;
  Environment *env;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  Value ex(Node *ast);
  Value doOp(int tag, int op1, int op2);
  // TODO: private member functions
};

#endif // INTERP_H
