#include <cassert>
#include <map>
#include <string>
#include <memory>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
////////////////////////////////////////////////////////////////////////

// This is the grammar (Unit is the start symbol):
//
// Unit -> Stmt
// Unit -> Stmt Unit
// Stmt -> E ;
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F -> ident
// F -> ( E )

Parser2::Parser2(Lexer *lexer_to_adopt)
    : m_lexer(lexer_to_adopt), m_next(nullptr)
{
}

Parser2::~Parser2()
{
  delete m_lexer;
}

Node *Parser2::parse()
{
  return parse_Unit();
}
Node *Parser2::parse_Unit()
{
  // note that this function produces a "flattened" representation
  // of the unit

  // Unit -> TStmt
  // Unit -> TStmt Unit
  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;)
  {
    unit->append_kid(parse_TStmt());
    if (m_lexer->peek() == nullptr)
      break;
  }

  return unit.release();
}

Node *Parser2::parse_TStmt()
{
  // TStmt -> Stmt
  return parse_Stmt();
}

Node *Parser2::parse_Stmt()
{
  // Stmt -> ^ A ;

  std::unique_ptr<Node> s(new Node(AST_STATEMENT));

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr)
  {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }

  int next_tok_tag = next_tok->get_tag();
  // Stmt -> ^ if ( A ) { SList }
  // Stmt -> ^ if ( A ) { SList } else { SList }
  if (next_tok_tag == TOK_IF)
  {
    // Stmt -> ^ ( A ) { SList }
    m_lexer->next();

    Node *ast = new Node(AST_IF);
    s->append_kid(ast);

    // Stmt -> ^ A ) { SList }
    expect_and_discard(TOK_LPAREN);

    // Stmt -> ^ ) { SList }
    ast->append_kid(parse_A());

    // Stmt -> ^ { SList }
    expect_and_discard(TOK_RPAREN);

    // Stmt -> ^ SList }
    expect_and_discard(TOK_LBRACK);

    // Stmt -> ^ }
    ast->append_kid(parse_SList());

    // Stmt -> ^ else { SList }
    expect_and_discard(TOK_RBRACK);

    next_tok = m_lexer->peek();

    if (next_tok != nullptr)
    {
      ast = new Node(AST_ELSE);
      s->append_kid(ast);

      // Stmt -> ^ { SList }
      expect_and_discard(TOK_ELSE);

      // Stmt -> ^ SList }
      expect_and_discard(TOK_LBRACK);

      // Stmt -> ^ }
      ast->append_kid(parse_SList());

      // Stmt -> ^
      expect_and_discard(TOK_RBRACK);
    }

    return s.release();
  }
  // Stmt -> ^ var ident ;
  else if (next_tok_tag == TOK_DEFINITION)
  {
    m_lexer->next();
    // Stmt -> ^ ident ;

    next_tok = expect(TOK_IDENTIFIER);

    Node *ast = new Node(AST_DEFINITION);
    ast->append_kid(new Node(AST_VARREF, next_tok->get_str()));
    s->append_kid(ast);
  }
  else
  {
    s->append_kid(parse_A());
  }

  expect_and_discard(TOK_SEMICOLON);
  return s.release();
}

Node *Parser2::parse_SList()
{
  std::unique_ptr<Node> s(new Node(AST_STATEMENT_LIST));

  Node *ast = parse_Stmt();

  s->append_kid(ast);

  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr && next_tok->get_tag() != TOK_RBRACK) {
    s->append_kid(parse_SList());
  }

  return s.release();
}

Node *Parser2::parse_E()
{
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast_)
{
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)
    {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      Node *term_ast = parse_T();
      ast.reset(new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast.release(), term_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_EPrime(ast.release());
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_T()
{
  // T -> F T'

  // Parse primary expression
  Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_)
{
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)
    {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next primary expression, incorporate into current AST
      Node *primary_ast = parse_F();
      ast.reset(new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast.release(), primary_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_TPrime(ast.release());
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_F()
{
  // F -> ^ number
  // F -> ^ ident
  // F -> ^ ( A )

  Node *next_tok = m_lexer->peek();
  if (next_tok == nullptr)
  {
    error_at_current_loc("Unexpected end of input looking for primary expression");
  }

  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER)
  {
    // F -> ^ number
    // F -> ^ ident
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  }
  else if (tag == TOK_LPAREN)
  {
    // F -> ^ ( E )
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  }
  else
  {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
  }
}

Node *Parser2::parse_A()
{
  // A -> ^ ident = A
  if (m_lexer->peek()->get_tag() == TOK_IDENTIFIER && m_lexer->peek(2)->get_tag() == TOK_ASSIGNMENT)
  {
    Node *ref = m_lexer->next();
    // A -> ^ = A

    std::unique_ptr<Node> ast(new Node(AST_VARREF, ref->get_str()));
    expect_and_discard(TOK_ASSIGNMENT);

    // A -> ^ A
    Node *rhs = parse_A();
    ast.reset(new Node(AST_ASSIGNMENT, {ast.release(), rhs}));

    return ast.release();
  }
  // A -> ^ L
  return parse_L();
}

Node *Parser2::parse_L()
{
  std::unique_ptr<Node> ast(parse_R());

  // L -> ^ || R
  // L -> ^ && R
  // L -> ^
  Node *next_tok = m_lexer->peek();

  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    int ast_tag;

    switch (next_tok_tag)
    {
    case TOK_LOGICAL_AND:
      ast_tag = AST_LOGICAL_AND;
      break;
    case TOK_LOGICAL_OR:
      ast_tag = AST_LOGICAL_OR;
      break;
    }

    if (next_tok_tag == TOK_LOGICAL_AND || next_tok_tag == TOK_LOGICAL_OR)
    {
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // L -> ^ R
      Node *rhs = parse_R();
      ast.reset(new Node(ast_tag, {ast.release(), rhs}));
      ast->set_loc(op->get_loc());
    }
  }

  return ast.release();
}

Node *Parser2::parse_R()
{

  // Get ast corresponding to E
  std::unique_ptr<Node> ast(parse_E());

  // peek at next token
  Node *next_tok = m_lexer->peek();

  // R -> ^ E
  if (next_tok != nullptr)
  {
    int next_tok_tag = next_tok->get_tag();
    int ast_tag;

    switch (next_tok_tag)
    {
    case TOK_GREATER:
      ast_tag = AST_GREATER;
      break;
    case TOK_LESS:
      ast_tag = AST_LESS;
      break;
    case TOK_GREATER_EQUAL:
      ast_tag = AST_GREATER_EQUAL;
      break;
    case TOK_LESS_EQUAL:
      ast_tag = AST_LESS_EQUAL;
      break;
    case TOK_EQUAL:
      ast_tag = AST_EQUAL;
      break;
    case TOK_NOT_EQUAL:
      ast_tag = AST_NOT_EQUAL;
      break;
    }

    // R -> ^ > E
    // R -> ^ < E
    // R -> ^ E >= E
    // R -> ^ E <= E
    // R -> ^ E == E
    // R -> ^ E != E
    if (next_tok_tag == TOK_GREATER || next_tok_tag == TOK_LESS || next_tok_tag == TOK_GREATER_EQUAL || next_tok_tag == TOK_LESS_EQUAL || next_tok_tag == TOK_EQUAL || next_tok_tag == TOK_NOT_EQUAL)
    {
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // R -> ^ E
      Node *rhs = parse_E();
      ast.reset(new Node(ast_tag, {ast.release(), rhs}));
      ast->set_loc(op->get_loc());
    }
  }

  return ast.release();
}

Node *Parser2::expect(enum TokenKind tok_kind)
{
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (next_terminal->get_tag() != tok_kind)
  {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal.release();
}

void Parser2::expect_and_discard(enum TokenKind tok_kind)
{
  Node *tok = expect(tok_kind);
  delete tok;
}

void Parser2::error_at_current_loc(const std::string &msg)
{
  SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}
