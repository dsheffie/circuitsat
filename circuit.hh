#ifndef __circuithh__
#define __circuithh__

#include <cstdint>
#include <list>
#include <iostream>
#include <set>

class gate;

enum class gateType {bogus, not1, constantone, constantzero, and2, or2, nor2, xor2, in, out};

class logicmodule {
protected:
  friend class gate;
  uint64_t cnt;
  std::list<gate*> gates;
public:
  logicmodule() : cnt(1) { }
  virtual ~logicmodule() {}
  template <typename T> T* make();
  template <typename T> T* make(gate *a); 
  template <typename T> T* make(gate *a, gate*b);
  void dump(std::ostream &out) const;
  void writeCNF(std::ostream &out) const;

};

class gate {
protected:
  friend class logicmodule;
  uint64_t id;
  gateType gt;
  int n_srcs;
  gate* srcs[2] = {nullptr};
  std::set<gate*> users;
  gate(uint64_t id, logicmodule *p, int n_srcs = 0) :
    id(id), n_srcs(n_srcs), gt(gateType::bogus) {
    p->gates.push_back(this);
  }
  gate(uint64_t id, logicmodule *p, gate *a) :
    gate(id, p, 1) {
    srcs[0] = a;
    a->users.insert(this);
  }
  gate(uint64_t id, logicmodule *p, gate *a, gate *b) :
    gate(id, p, 1) {
    srcs[0] = a;
    srcs[1] = b;
    a->users.insert(this);
    b->users.insert(this);
  }
  virtual ~gate() {}
public:
  uint64_t getId() const { return id; }
  virtual int nClauses() const = 0;
  virtual void dump(std::ostream &out) const = 0;
  virtual void writeCNF(std::ostream &out) const {};  
};

class not1 : public gate {
protected:
  friend class logicmodule;
  not1(uint64_t id, logicmodule *p, gate *a) : gate(id, p, a) {
    gt = gateType::not1;
  }
public:
  int nClauses() const override {return 2;}  
  void dump(std::ostream &out) const override {
    out << getId() << " = not1(" << srcs[0]->getId() << ")\n";
  }
};

class and2 : public gate {
protected:
  friend class logicmodule;
  and2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::and2;
  }
public:
  int nClauses() const override {return 3;};    
  void dump(std::ostream &out) const override {
    out << getId() << " = and2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
  }
  void writeCNF(std::ostream &out) const override;  
};

class or2 : public gate {
protected:
  friend class logicmodule;
  or2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::or2;
  }
public:
  int nClauses() const override {return 3;}      
  void dump(std::ostream &out) const override {
    out << getId() << " = or2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
  }
};


class xor2 : public gate {
protected:
  friend class logicmodule;  
  xor2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::xor2;
  }
public:
  int nClauses() const override {return 4;}      
  void dump(std::ostream &out) const override {
    out << getId() << " = xor2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
  }
};

class constantone : public gate {
protected:
  friend class logicmodule;
  constantone(uint64_t id, logicmodule *p) : gate(id, p) {
    gt = gateType::constantone;
  }
public:
  int nClauses() const override {return 1;}
  void dump(std::ostream &out) const override {
    out << getId() << " = " << srcs[0]->getId() << "\n";
  }
  void writeCNF(std::ostream &out) const override;  
};

class constantzero : public gate {
protected:
  friend class logicmodule;
  constantzero(uint64_t id, logicmodule *p) : gate(id, p) {
    gt = gateType::constantzero;
  }
public:
  int nClauses() const override {return 1;}
  void dump(std::ostream &out) const override {
    out << getId() << " = " << srcs[0]->getId() << "\n";
  }
  void writeCNF(std::ostream &out) const override;  
};


class po : public gate {
protected:
  friend class logicmodule;
  bool val = true;
  po(uint64_t id, logicmodule *p, gate *a) : gate(id, p, a) {
    gt = gateType::in;
  }
public:
  int nClauses() const override {return 1;}
  void dump(std::ostream &out) const override {
    out << getId() << " = out(" << srcs[0]->getId() << ")\n";
  }
  void writeCNF(std::ostream &out) const override;  
};


class pi : public gate {
protected:
  friend class logicmodule;
  int nClauses() const override {return 0;}
  pi(uint64_t id, logicmodule *p) : gate(id, p) {
    gt = gateType::out;
  }
public:
  
  void dump(std::ostream &out) const override {
    out << getId() << " = in()\n";
  }
};


template <typename T> T* logicmodule::make() {
  return new T(cnt++, this);
  }
template <typename T> T* logicmodule::make(gate *a) {
  return new T(cnt++, this, a);
}
template <typename T> T* logicmodule::make(gate *a, gate*b) {
  return new T(cnt++, this, a, b);
  
}
#endif
