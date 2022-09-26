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

  // Create environment for checking
  Environment *env = new Environment();

  // Define intrinsic functions
  env->define("print");
  env->define("println");

  // Recurse over tree to search for semantic error
  analyze_recurse(m_ast, env);
}

void Interpreter::analyze_recurse(Node *ast, Environment *env)
{
  // variable was referenced
  if (ast->get_tag() == AST_VARREF)
  {

    // Check if VARREF was defined
    if (findEnv(ast, env)->has(ast->get_str())) {
      return;
    }

    // VARREF was not defined, raise error
    const std::string err = std::string("Undefined reference to name '") + ast->get_str().c_str() + "'";
    SemanticError::raise(ast->get_loc(), err.c_str());
  }

  // We define a VARREF, insert into map
  if (ast->get_tag() == AST_DEFINITION)
  {
    env->define(ast->get_kid(0)->get_str());
  }

  // Check all of node's children
  for (unsigned int i = 0; i < ast->get_num_kids(); i++)
  {
    analyze_recurse(ast->get_kid(i), env);
  }
}

Value Interpreter::execute()
{
  // TODO: implement

  // Global environment
  Environment *global_env = new Environment();

  // Define intrinsic functions
  global_env->define("print");
  global_env->define("println");

  // Bind intrinsic functions
  global_env->assign("print", &intrinsic_print);
  global_env->assign("println", &intrinsic_println);

  // Evaluates each statement
  for (unsigned int i = 0; i < m_ast->get_num_kids() - 1; i++)
  {
    ex(m_ast->get_kid(i), global_env);
  }

  // Return for result
  return ex(m_ast->get_last_kid(), global_env);
}

Value Interpreter::ex(Node *ast, Environment *env)
{
  // Execute each statement in a block of statements
  if (ast->get_tag() == AST_STATEMENT_LIST)
  {
    for (unsigned int i = 0; i < ast->get_num_kids(); i++)
    {
      ex(ast->get_kid(i), env);
    }
    return 0;
  }

  // Call function
  if (ast->get_tag() == AST_FNCALL)
  {
    // Find the function definition
    Environment *location = findEnv(ast, env);

    // Get args the the program entered
    unsigned numargs = ast->get_kid(0)->get_num_kids();
    Value args[numargs];

    // Evaluate each arg
    for (unsigned int i = 0; i < ast->get_kid(0)->get_num_kids(); i++)
    {
      args[i] = ex(ast->get_kid(0)->get_kid(i), env);
    }

    // Retrieve the function
    IntrinsicFn fn = (location->lookup(ast->get_str())).get_intrinsic_fn();

    // Execute the function
    return fn(args, numargs, ast->get_loc(), this);
  }

  // Is statement
  if (ast->get_tag() == AST_STATEMENT)
  {
    return ex(ast->get_kid(0), env);
  }

  // Is int literal
  if (ast->get_tag() == AST_INT_LITERAL)
  {
    return atoi(ast->get_str().c_str());
  }

  // Vardef
  if (ast->get_tag() == AST_DEFINITION)
  {
    env->define(ast->get_kid(0)->get_str());
    return 0;
  }

  // Var assignment
  if (ast->get_tag() == AST_ASSIGNMENT)
  {
    // Find correct environment
    Environment *location = findEnv(ast->get_kid(0), env);

    // Assign in the appropriate environment
    location->assign(ast->get_kid(0)->get_str(), ex(ast->get_kid(1), env));

    // Return assignment value
    return location->lookup(ast->get_kid(0)->get_str());
  }

  // Var reference
  if (ast->get_tag() == AST_VARREF)
  {
    // Find correct environment
    Environment *location = findEnv(ast, env);

    // Retrieve variable value
    return location->lookup(ast->get_str());
  }

  // If statement
  if (ast->get_tag() == AST_IF)
  {
    // Check if the condition actually produces a number
    if (non_numeric(ast->get_kid(0), env))
    {
      EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
    }

    // Check if condition evaluates to true
    if (ex(ast->get_kid(0), env).get_ival() != 0)
    {
      // Create new block
      Environment *block_env = new Environment(env);
      ex(ast->get_kid(1), block_env);
    }

    // Execute else block if above not trigered
    else if (ast->get_last_kid()->get_tag() == AST_ELSE)
    {
      // Create new block
      Environment *block_env = new Environment(env);
      ex(ast->get_last_kid()->get_kid(0), block_env);
    }
    return 0;
  }

  // While loop
  if (ast->get_tag() == AST_WHILE)
  {
    // Check if the condition actually produces a number
    if (non_numeric(ast->get_kid(0), env))
    {
      EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
    }

    // Execute body while condition evaluates to true
    while (ex(ast->get_kid(0), env).get_ival() != 0)
    {
      // Create new block
      Environment *block_env = new Environment(env);
      ex(ast->get_kid(1), block_env);
    }
    return 0;
  }

  // Check if operand1 is numeric
  if (non_numeric(ast->get_kid(0), env))
  {
    EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
  }

  // Retrieve first operand
  int val1 = (ex(ast->get_kid(0), env)).get_ival();

  // Short circuit &&
  if (ast->get_tag() == AST_LOGICAL_AND)
  {
    if (val1 == 0)
    {
      return 0;
    }
  }
  // Short circuit ||
  else if (ast->get_tag() == AST_LOGICAL_OR)
  {
    if (val1 != 0)
    {
      return 1;
    }
  }

  // Check if operand2 is numeric
  if (non_numeric(ast->get_kid(1), env))
  {
    EvaluationError::raise(ast->get_loc(), "Non-numeric condition");
  }

  // Retrieve second operand
  int val2 = (ex(ast->get_kid(1), env)).get_ival();

  // Perform associated operation
  return doOp(ast->get_tag(), val1, val2, ast->get_kid(1));
}

Environment *Interpreter::findEnv(Node *ref, Environment *env)
{
  while (!env->has(ref->get_str()) && env->getParent() != nullptr)
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
    EvaluationError::raise(loc, "Wrong number of arguments passed to println function");
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