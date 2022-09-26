#include "environment.h"
#include <iostream>

using std::map;

Environment::Environment(Environment *parent)
    : m_parent(parent)
{
  assert(m_parent != this);
}

Environment::~Environment()
{
  // Empty the map
  references.clear();
}

// Assign a value to a defined variable
void Environment::assign(std::string var, Value i)
{
  references[var] = i;
}

// Check if var is defined in this environment
bool Environment::has(std::string var)
{
  return (references.count(var) != 0);
}

// Return parent
Environment *Environment::getParent()
{
  return m_parent;
}

// Return value of var
Value Environment::lookup(std::string var)
{
  // F → ident
  return references[var];
}

// Define a var
void Environment::define(std::string var)
{
  // Stmt → var ident ;

  // use as a marker for now
  references.insert({var, -1});
}
// TODO: implement member functions
