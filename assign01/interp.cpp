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
}

Interpreter::~Interpreter()
{
  delete m_ast;
}

void Interpreter::analyze()
{
  // Recursively analyze nodes of the ast
  set.insert("print");
  set.insert("println");
  analyze_recurse(m_ast);
}

void Interpreter::analyze_recurse(Node *ast)
{
  // variable was referenced
  if (ast->get_tag() == AST_VARREF)
  {

    // Check if VARREF was defined
    for (auto i = set.begin(); i != set.end(); i++)
    {
      if (((*i).compare(ast->get_str())) == 0)
      {
        return;
      }
    }

    // VARREF was not defined, raise error
    const std::string err = std::string("Undefined reference to name '") + ast->get_str().c_str() + "'";
    SemanticError::raise(ast->get_loc(), err.c_str());
  }

  // We define a VARREF, insert into map
  if (ast->get_tag() == AST_DEFINITION)
  {
    set.insert(ast->get_kid(0)->get_str().c_str());
  }

  // Check all of node's children
  for (unsigned int i = 0; i < ast->get_num_kids(); i++)
  {
    analyze_recurse(ast->get_kid(i));
  }
}

Value Interpreter::execute()
{
  // TODO: implement
  Environment *global_env = new Environment();

  global_env->define("print");
  global_env->define("println");

  global_env->assign("print", &intrinsic_print);
  global_env->assign("println", &intrinsic_println);

  for (unsigned int i = 0; i < m_ast->get_num_kids() - 1; i++)
  {
    ex(m_ast->get_kid(i), global_env);
  }

  return ex(m_ast->get_last_kid(), global_env);
}

Value Interpreter::ex(Node *ast, Environment *env)
{
  if (ast->get_tag() == AST_STATEMENT_LIST)
  {
    for (unsigned int i = 0; i < ast->get_num_kids(); i++)
    {
      ex(ast->get_kid(i), env);
    }
    return 0;
  }

  if (ast->get_tag() == AST_FNCALL)
  {
    Environment *location = findEnv(ast, env);

    unsigned numargs = ast->get_num_kids();
    Value args[numargs];

    for (unsigned int i = 0; i < ast->get_num_kids(); i++)
    {
      args[i] = ex(ast->get_kid(i), env);
    }

    IntrinsicFn fn = (location->lookup(ast->get_str())).get_intrinsic_fn();
    return fn(args, numargs, ast->get_loc(), this);
  }

  if (ast->get_tag() == AST_STATEMENT)
  {
    return ex(ast->get_kid(0), env);
  }

  if (ast->get_tag() == AST_INT_LITERAL)
  {
    return atoi(ast->get_str().c_str());
  }

  if (ast->get_tag() == AST_DEFINITION)
  {
    env->define(ast->get_kid(0)->get_str());
    return 0;
  }

  if (ast->get_tag() == AST_ASSIGNMENT)
  {
    Environment *location = findEnv(ast->get_kid(0), env);
    location->assign(ast->get_kid(0)->get_str(), ex(ast->get_kid(1), env));
    return location->lookup(ast->get_kid(0)->get_str());
  }

  if (ast->get_tag() == AST_VARREF)
  {
    Environment *location = findEnv(ast, env);
    return location->lookup(ast->get_str());
  }

  if (ast->get_tag() == AST_IF)
  {
    if (non_numeric(ast->get_kid(0), env))
    {
      EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
    }
    if (ex(ast->get_kid(0), env).get_ival() != 0)
    {
      Environment *block_env = new Environment(env);
      ex(ast->get_kid(1), block_env);
    }
    else if (ast->get_last_kid()->get_tag() == AST_ELSE)
    {
      Environment *block_env = new Environment(env);
      ex(ast->get_last_kid()->get_kid(0), block_env);
    }
    return 0;
  }

  if (ast->get_tag() == AST_WHILE)
  {
    if (non_numeric(ast->get_kid(0), env))
    {
      EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
    }
    while (ex(ast->get_kid(0), env).get_ival() != 0)
    {
      Environment *block_env = new Environment(env);
      ex(ast->get_kid(1), block_env);
    }
    return 0;
  }

  int val1 = (ex(ast->get_kid(0), env)).get_ival();

  if (ast->get_tag() == AST_LOGICAL_AND)
  {
    if (val1 == 0)
    {
      return 0;
    }
  }
  else if (ast->get_tag() == AST_LOGICAL_OR)
  {
    if (val1 != 0)
    {
      return 1;
    }
  }

  int val2 = (ex(ast->get_kid(1), env)).get_ival();

  return doOp(ast->get_tag(), val1, val2, ast->get_kid(1));
}

Environment *Interpreter::findEnv(Node *ref, Environment *env)
{
  while (!env->has(ref->get_str()))
  {
    env = env->getParent();
  }

  return env;
}

Value Interpreter::doOp(int tag, int op1, int op2, Node *divisor)
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
    if (op2 == 0)
    {
      EvaluationError::raise(divisor->get_loc(), "Attempt to divide by 0");
    }
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

Value Interpreter::intrinsic_print(Value args[], unsigned num_args,
                                   const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
  printf("%s", args[0].as_str().c_str());
  return Value();
}

Value Interpreter::intrinsic_println(Value args[], unsigned num_args,
                                     const Location &loc, Interpreter *interp)
{
  if (num_args != 1)
    EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
  printf("%s\n", args[0].as_str().c_str());
  return Value();
}

bool Interpreter::non_numeric(Node *ast, Environment *env)
{
  switch (ast->get_tag())
  {
  case AST_VARREF:
  {
    Environment *location = findEnv(ast, env);
    if (location->lookup(ast->get_str()).is_numeric())
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  case AST_UNIT:
  case AST_STATEMENT:
  case AST_DEFINITION:
  case AST_ASSIGNMENT:
  case AST_IF:
  case AST_ELSE:
  case AST_STATEMENT_LIST:
  case AST_WHILE:
  case AST_FNCALL:
    return true;
  default:
    return false;
  }
}