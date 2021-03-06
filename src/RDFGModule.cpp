#include <Rcpp.h>
#include "RDFG.h"

using namespace Rcpp ;

RCPP_MODULE(phy) {
    class_<RDFG>("RDFG")

      .constructor<IntegerVector, List, List, IntegerVector>()

      .method("simulate", &RDFG::simulate)

      .method("mps", &RDFG::mps)

      .method("expect", &RDFG::expect)

      .method("expectCondData", &RDFG::expectCondData)

      .method("gamma", &RDFG::gamma) 

      .method("facExpCounts", &RDFG::facExpCounts)

      .method("resetPotentials", &RDFG::resetPotentials)

      .method("resetScores", &RDFG::resetScores)
      
      .method("getPotentials", &RDFG::getPotentials)

      .method("calcLikelihood", &RDFG::calcLikelihood)

      .method("calcLogLikelihood", &RDFG::calcLogLikelihood)
      ;

}
