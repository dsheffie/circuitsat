#include "circuit.hh"
#include <iostream>

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
  

int main() {
  logicmodule *lm = new logicmodule();

  auto i0 = lm->make<pi>();
  auto i1 = lm->make<constantzero>();
  auto a = lm->make<and2>(i0, i1);
  auto o = lm->make_po(a, false);

  lm->runMiniSAT();

  
  delete lm;
  return 0;
}
