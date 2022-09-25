#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"

class Environment {
private:
  Environment *m_parent;

  // TODO: representation of environment (map of names to values)
  std::map<std::string, Value> references;
  

  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  Environment* getParent();
  // Assign value to a VARREF
  void assign(std::string var, Value i);

  // Retrieve value of a VARREF
  Value lookup(std::string var);

  bool has(std::string var);
  // Define a VARREF
  void define(std::string var);
  // TODO: add member functions allowing lookup, definition, and assignment
};

#endif // ENVIRONMENT_H
