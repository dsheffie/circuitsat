#include "circuit.hh"
#include <iostream>

void logicmodule::dump(std::ostream &out) const {
  for(gate *g : gates) {
    g->dump(out);
  }
}

void logicmodule::writeCNF(std::ostream &out) const {
  uint64_t n_var = gates.size(), n_clauses = 0;
  for(gate *g : gates) {
    n_clauses += g->nClauses();
  }
  out << "p cnf " << n_var << " " << n_clauses << "\n";
  for(gate *g : gates) {
    g->writeCNF(out);
  }
}

void and2::writeCNF(std::ostream &out) const {
  //(\overline {A}\vee \overline {B}\vee C)\wedge (A\vee \overline {C})\wedge (B\vee \overline {C})
  uint64_t A = srcs[0]->getId(), B = srcs[1]->getId(), C = getId();
  out << "-" << A << " -" << B << " " << C << " 0\n";
  out << A << " -" << C << " 0\n";
  out << B << " -" << C << " 0\n";    
}

void po::writeCNF(std::ostream &out) const {
  if(val == false) {
    out << "-";
  }
  out << getId() << " 0\n";
}
  

int main() {
  logicmodule *lm = new logicmodule();

  auto i0 = lm->make<pi>();
  auto i1 = lm->make<pi>();
  auto a = lm->make<and2>(i0, i1);
  auto o = lm->make<po>(a);

  lm->writeCNF(std::cout);

  
  delete lm;
  return 0;
}
