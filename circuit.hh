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
  void writeCNF(std::ostream &out) const;    
  virtual ~logicmodule();
  gate *make_po(gate *a, bool val);
  template <typename T> T* make();
  template <typename T> T* make(gate *a); 
  template <typename T> T* make(gate *a, gate*b);
  void dump(std::ostream &out) const;
};

class gate {
protected:
  friend class logicmodule;
  uint64_t id;
  int n_srcs;  
  gateType gt;
  bool val;
  uint64_t lp;
  bool lpval;
  gate* srcs[2] = {nullptr};
  std::set<gate*> users;
  gate(uint64_t id, logicmodule *p, int n_srcs = 0) :
    id(id), n_srcs(n_srcs), gt(gateType::bogus), val(false), lp(0), lpval(false) {
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
  void setValue(bool val) {
    this->val = val;
  }
  bool getValue() const {
    return val;
  }
  virtual ~gate() {}
public:
  uint64_t getId() const { return id; }
  virtual uint64_t longestPath()  = 0;
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
  void dump(std::ostream &out) const override;
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override;  
};

class and2 : public gate {
protected:
  friend class logicmodule;
  and2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::and2;
  }
public:
  int nClauses() const override {return 3;};    
  void dump(std::ostream &out) const override;
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override;    
};

class or2 : public gate {
protected:
  friend class logicmodule;
  or2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::or2;
  }
public:
  int nClauses() const override {return 3;}      
  void dump(std::ostream &out) const override;
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override;    
};


class xor2 : public gate {
protected:
  friend class logicmodule;  
  xor2(uint64_t id, logicmodule *p, gate *a, gate *b) : gate(id, p, a, b) {
    gt = gateType::xor2;
  }
public:
  int nClauses() const override {return 4;}      
  void dump(std::ostream &out) const override;
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override;    
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
    out << getId() << " = 1\n";
  }
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override {
    return 0;
  };    
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
    out << getId() << " = 0\n";
  }
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override {
    return 0;
  };      
};


class po : public gate {
protected:
  friend class logicmodule;
  po(uint64_t id, logicmodule *p, gate *a, bool val) : gate(id, p, a){
    gt = gateType::in;
    this->val = val;
  }
public:
  int nClauses() const override {return 1;}
  void dump(std::ostream &out) const override {
    out << getId() << " = out(" << srcs[0]->getId() << ")\n";
  }
  void writeCNF(std::ostream &out) const override;
  uint64_t longestPath() override;
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
  uint64_t longestPath() override {
    return 0;
  };        
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
