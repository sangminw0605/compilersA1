#include <cassert>
#include <algorithm>
#include <memory>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Interpreter::Interpreter(Node *ast_to_adopt)
    : m_ast(ast_to_adopt)
{
  env = new Environment();
}

Interpreter::~Interpreter()
{
  delete m_ast;
  delete env;
}

void Interpreter::analyze()
{
  // TODO: implement
  
}

Value Interpreter::execute()
{
  // TODO: implement
  for (unsigned int i = 0; i < m_ast->get_num_kids() - 1; i++) {
    ex(m_ast->get_kid(i));
  }

  return ex(m_ast->get_last_kid());
}

Value Interpreter::ex(Node *ast)
{
  if (ast->get_tag() == AST_STATEMENT) {
    return ex(ast->get_kid(0));
  }

  if (ast->get_tag() == AST_DEFINITION) {
    env->define(ast->get_str());
    return 0;
  }

  if (ast->get_tag() == AST_ASSIGNMENT) {

    env->assign(ast->get_kid(0)->get_str(), ex(ast->get_kid(1)));
    return env->lookup(ast->get_kid(0)->get_str());
  }

  if (ast->get_tag() == AST_INT_LITERAL) {
    return atoi(ast->get_str().c_str());
  }

  if (ast->get_tag() == AST_VARREF) {
    return env->lookup(ast->get_str());
  }

  int val1 = (ex(ast->get_kid(0))).get_ival();
  int val2 = (ex(ast->get_kid(1))).get_ival();

  return doOp(ast->get_tag(), val1, val2);
}

Value Interpreter::doOp(int tag, int op1, int op2)
{

  switch (tag)
  {
  case AST_ADD:
    return op1 + op2;
  case AST_SUB:
    return op1 - op2;
  case AST_MULTIPLY:
    return op1 * op2;
  case AST_DIVIDE:
    return op1 / op2;
  case AST_GREATER_EQUAL:
    if (op1 == op2)
    {
      return 1;
    }
  case AST_GREATER:
    return op1 > op2 ? 1 : 0;
  case AST_LESS_EQUAL:
    if (op1 == op2)
    {
      return 1;
    }
  case AST_LESS:
    return op1 < op2 ? 1 : 0;
  case AST_EQUAL:
    return op1 == op2 ? 1 : 0;
  case AST_NOT_EQUAL:
    return op1 != op2 ? 1 : 0;
  case AST_LOGICAL_AND:
    return (op1 != 0 && op2 != 0) ? 1 : 0;
  case AST_LOGICAL_OR:
    return (op1 != 0 || op2 != 0) ? 1 : 0;
  }
  return 0;
}