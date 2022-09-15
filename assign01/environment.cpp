#include "environment.h"

using std::map;

Environment::Environment(Environment *parent)
    : m_parent(parent)
{
  assert(m_parent != this);
}

Environment::~Environment()
{
  references.clear();
}

void Environment::assign(std::string var, Value i)
{
  // A → ident = A
  if (lookup(var).get_ival() != -2)
  {
    references[var] = i;
  }
}

Value Environment::lookup(std::string var)
{
  // F → ident

  return references[var];

}

void Environment::define(std::string var)
{
  // Stmt → var ident ;

  // use int min for now
  references.insert({var, 0});
}

bool Environment::invalids()
{
  for (auto it = references.begin(); it != references.end(); ++it)
  {
    if (it->second.get_ival() == -1) {
      return true;
    }
  }
  return false;
}
  // TODO: implement member functions
