#include "circuit.hh"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include <vector>
#include <unistd.h>

gate *logicmodule::make_po(gate *in, bool val) {
  return new po(cnt++, this, in, val);
}

void logicmodule::dump(std::ostream &out) const {
  for(gate *g : gates) {
    g->dump(out);
  }
}

logicmodule::~logicmodule() {
  for(gate *g : gates) {
    delete g;
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

void not1::writeCNF(std::ostream &out) const {
  uint64_t A = srcs[0]->getId(), C = getId();
  out <<  "-" << A << " -" << C << " 0\n";
  out << A << " " << C << " 0\n";  
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

uint64_t not1::longestPath() {
  if(not(lpval)) {
    lp = srcs[0]->longestPath()+ 1;
    lpval = true;
  }
  return lp;
}

uint64_t or2::longestPath() {
  if(not(lpval)) {
    lp = std::max(srcs[0]->longestPath(), srcs[1]->longestPath()) + 1;
    lpval = true;
  }
  return lp;
}

uint64_t and2::longestPath() {
  if(not(lpval)) {
    lp = std::max(srcs[0]->longestPath(), srcs[1]->longestPath()) + 1;
    lpval = true;
  }
  return lp;
}

uint64_t xor2::longestPath() {
  if(not(lpval)) {
    lp = std::max(srcs[0]->longestPath(), srcs[1]->longestPath()) + 1;
    lpval = true;
  }
  return lp;
}

uint64_t po::longestPath() {
  if(not(lpval)) {
    lp = srcs[0]->longestPath();
    lpval = true;
  }
  return lp;
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

inline size_t log2(size_t y) {
  size_t x = 0;
  while( (1UL << x) < y ) {
    x++;
  }
  return x;
}

std::vector<gate*> make_parallel_prefix_adder(logicmodule *lm, const std::vector<gate*> & a, const std::vector<gate*> & b) {
  std::vector<gate*> y,gg,pp;
  size_t n_bits = a.size();
  size_t lg_n_bits = log2(n_bits);
  assert((n_bits & (n_bits-1)) == 0);
  assert(n_bits == b.size());

  for(size_t i = 0; i < n_bits; i++) {
    gg.push_back(lm->make<and2>(a.at(i), b.at(i)));
    pp.push_back(lm->make<xor2>(a.at(i), b.at(i)));
  }
  std::vector<gate*> p = pp;
  
  for(size_t l = 0; l < lg_n_bits; l++) {
    ssize_t d = 1L<<l;
    std::vector<gate*> tgg,tpp;
    for(ssize_t i = 0; i < static_cast<ssize_t>(n_bits); i++) {
      gate *n = pp.at(i);
      gate *g = gg.at(i);
      if ((i-d) >= 0L) {
	n = lm->make<and2>(pp.at(i), pp.at(i-d));
	g = lm->make<and2>(gg.at(i-d), pp.at(i));
	g = lm->make<or2>(g, gg.at(i));
      }
      tgg.push_back(g);
      tpp.push_back(n);
    }
    pp = tpp;
    gg = tgg;
  }
  y.push_back(pp.at(0));
  for(size_t i = 1; i < n_bits; i++) {
    y.push_back(lm->make<xor2>(p.at(i), gg.at(i-1)));
  }
  assert(y.size() == n_bits);
  return y;
}

gate *mk_eq2(logicmodule *lm, gate* a,  gate* b) {
  gate *n_a = lm->make<not1>(a);
  gate *n_b = lm->make<not1>(b);
  gate *t0 = lm->make<and2>(n_a, n_b);
  gate *t1 = lm->make<and2>(a, b);
  return lm->make<or2>(t0, t1);
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

int main(int argc, char *argv[]) {
  int c;  
  logicmodule *lm = new logicmodule();
  std::vector<gate*> a, b;
  size_t n_bits = 32;
  
  while ((c = getopt(argc, argv, "n:")) != -1) {
    switch(c)
      {
      case 'n':
	n_bits = static_cast<size_t>(atoi(optarg));
	break;
      default:
	break;
      }
  }

  for(int i = 0; i < n_bits; i++) {
    a.push_back(lm->make<pi>());
  }
  for(int i = 0; i < n_bits; i++) {  
    b.push_back(lm->make<pi>());
  }

  auto y = make_ripple_carry_adder(lm,a,b);
  auto x = make_parallel_prefix_adder(lm,a,b);

  auto t = make_not_equal(lm, y, x);
  auto o = lm->make_po(t, true);

  //lm->runMiniSAT();
  std::ofstream out("foo.cnf");
  lm->writeCNF(out);
  
  delete lm;
  return 0;
}
