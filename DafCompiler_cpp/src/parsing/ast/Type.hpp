#pragma once

#include <string>

class Type {
public:
  virtual ~Type();
  virtual void printSignature()=0;
};

class TypedefType : public Type {
private:
  std::string m_name;
public:
  TypedefType(const std::string& name);
  ~TypedefType();
  void printSignature();
};
