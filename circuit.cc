#include "circuit.hh"
#include <iostream>
#include <fstream>
#include <cassert>

gate *logicmodule::make_po(gate *in, bool val) {
  return new po(cnt++, this, in, val);
}

void logicmodule::dump(std::ostream &out) const {
  for(gate *g : gates) {
    g->dump(out);
  }
}

void not1::dump(std::ostream &out) const {
  out << getId() << " = not1(" << srcs[0]->getId() << ")\n";
}

void and2::dump(std::ostream &out) const {
  out << getId() << " = and2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
}

void or2::dump(std::ostream &out) const {
  out << getId() << " = or2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
}

void xor2::dump(std::ostream &out) const {
  out << getId() << " = xor2(" << srcs[0]->getId() << "," << srcs[1]->getId() << ")\n";
}

void logicmodule::writeCNF(std::ostream &out) const {
  uint64_t n_var = gates.size()-1, n_clauses = 0;
  for(gate *g : gates) {
    n_clauses += g->nClauses();
  }
  out << "p cnf " << n_var << " " << n_clauses << "\n";
  for(gate *g : gates) {
    g->writeCNF(out);
  }
}

void constantone::writeCNF(std::ostream &out) const {
  out << getId() << " 0\n";
}

void constantzero::writeCNF(std::ostream &out) const {
  out << "-" << getId() << " 0\n";
}

void and2::writeCNF(std::ostream &out) const {
  uint64_t A = srcs[0]->getId(), B = srcs[1]->getId(), C = getId();
  out << "-" << A << " -" << B << " " << C << " 0\n";
  out << A << " -" << C << " 0\n";
  out << B << " -" << C << " 0\n";    
}

void or2::writeCNF(std::ostream &out) const {
  uint64_t A = srcs[0]->getId(), B = srcs[1]->getId(), C = getId();
  out << A << " " << B << " -" << C << " 0\n";
  out << C << " -" << A << " 0\n";
  out << C << " -" << B << " 0\n";    
}

void xor2::writeCNF(std::ostream &out) const {
  uint64_t A = srcs[0]->getId(), B = srcs[1]->getId(), C = getId();
  out << "-" << A << " -" << B << " -" << C << " 0\n";
  out << "-" << A << " " << B << " " << C << " 0\n";
  out << A << " -" << B << " " << C << " 0\n";
  out << A << " " << B << " -" << C << " 0\n";
}

void po::writeCNF(std::ostream &out) const {
  if(val == false) {
    out << "-";
  }
  out << srcs[0]->getId() << " 0\n";
}


std::vector<gate*> make_ripple_carry_adder(logicmodule *lm, const std::vector<gate*> & a, const std::vector<gate*> & b) {
  std::vector<gate*> y;
  assert(a.size() == b.size());
  gate *cin = lm->make<constantzero>();
  for(size_t i = 0; i < a.size(); i++) {
    gate *a_x_b = lm->make<xor2>(a.at(i), b.at(i));
    gate *s = lm->make<xor2>(a_x_b, cin);
    y.push_back(s);
    gate *cout = lm->make<and2>(a.at(i), b.at(i));
    a_x_b = lm->make<and2>(a_x_b, cin);
    cout = lm->make<or2>(a_x_b, cout);
    cin = cout;
  }
  return y;
}

gate* make_equal(logicmodule *lm, const std::vector<gate*> & a, const std::vector<gate*> & b) {
  std::vector<gate*> t;
  gate *y = lm->make<constantone>();
  assert(a.size() == b.size());
  for(size_t i = 0; i < a.size(); i++) {
    gate *a_x_b = lm->make<not1>(lm->make<xor2>(a.at(i), b.at(i)));
    t.push_back(a_x_b);
  }
  for(size_t i = 0; i < t.size(); i++) {
    y = lm->make<and2>(y, t.at(i));
  }
  return y;
}

gate* make_not_equal(logicmodule *lm, const std::vector<gate*> & a, const std::vector<gate*> & b) {
  return lm->make<not1>(make_equal(lm, a, b));
}

int main() {
  logicmodule *lm = new logicmodule();
  std::vector<gate*> a, b;
  for(int i = 0; i < 4; i++) {
    a.push_back(lm->make<pi>());
    b.push_back(lm->make<pi>());
  }
  auto y = make_ripple_carry_adder(lm,a,b);

  std::vector<gate*> c;
  c.push_back(lm->make<constantone>());
  c.push_back(lm->make<constantone>());
  c.push_back(lm->make<constantone>());
  c.push_back(lm->make<constantone>());  
  
  auto t = make_equal(lm, y, c);
  auto o = lm->make_po(t, true);

  //lm->runMiniSAT();
  std::ofstream out("foo.cnf");
  lm->writeCNF(out);
  
  delete lm;
  return 0;
}
