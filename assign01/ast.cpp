#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() {
}

ASTTreePrint::~ASTTreePrint() {
}

std::string ASTTreePrint::node_tag_to_string(int tag) const {
  switch (tag) {
  case AST_ADD:
    return "ADD";
  case AST_SUB:
    return "SUB";
  case AST_MULTIPLY:
    return "MULTIPLY";
  case AST_DIVIDE:
    return "DIVIDE";
  case AST_VARREF:
    return "VARREF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  case AST_UNIT:
    return "UNIT";
  case AST_STATEMENT:
    return "STATEMENT";
  // TODO: add cases for other AST node kinds
  case AST_GREATER:
    return "GREATER_THAN";
  case AST_LESS:
    return "LESS_THAN";
  case AST_GREATER_EQUAL:
    return "GREATER_THAN_OR_EQUAL";
  case AST_LESS_EQUAL:
    return "LESS_THAN_OR_EQUAL";
  case AST_EQUAL:
    return "EQUAL";
  case AST_NOT_EQUAL:
    return "NOT_EQUAL";
  case AST_LOGICAL_AND:
    return "LOGICAL_AND";
  case AST_LOGICAL_OR:
    return "LOGICAL_OR";
  case AST_DEFINITION:
    return "DEFINITION";
  case AST_ASSIGNMENT:
    return "ASSIGNMENT";
  case AST_IF:
    return "IF";
  case AST_ELSE:
    return "ELSE";
  case AST_STATEMENT_LIST:
    return "AST_STATEMENT_LIST";
  case AST_WHILE:
    return "WHILE";
  case AST_FNCALL:
    return "FNCALL";
  case AST_ARGUMENT_LIST:
    return "ARGLIST";
  case AST_FUNCTION:
    return "FUNCTION";
  case AST_P_LIST:
    return "PARAMETER_LIST";
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
