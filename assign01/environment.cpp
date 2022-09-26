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

void Environment::assign(std::string var, Value i)
{
  references[var] = i;
}

bool Environment::has(std::string var)
{
  return (references.count(var) != 0);
}

Environment *Environment::getParent()
{
  return m_parent;
}

Value Environment::lookup(std::string var)
{
  // F → ident
  return references[var];
}

void Environment::define(std::string var)
{
  // Stmt → var ident ;

  // use int min as a marker for now
  references.insert({var, -1});
}
// TODO: implement member functions
