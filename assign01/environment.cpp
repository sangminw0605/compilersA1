#include "environment.h"

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
  // A → ident = A
  references[var] = i;
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
  references.insert({var, 0});
}

void Environment::clear() {
  references.clear();
}
  // TODO: implement member functions
