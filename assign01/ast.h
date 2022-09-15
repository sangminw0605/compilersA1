#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind {
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  AST_GREATER,
  AST_LESS,
  AST_GREATER_EQUAL,
  AST_LESS_EQUAL,
  AST_EQUAL,
  AST_NOT_EQUAL,
  AST_LOGICAL_AND,
  AST_LOGICAL_OR,
  AST_DEFINITION,
  AST_ASSIGNMENT
  // TODO: add members for other AST node kinds
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
