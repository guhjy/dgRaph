#include <Rcpp.h>
#include <cmath>
#include <ctime>
#include "RDFG.h"
#include "rToCpp.h"
#include "PhyDef.h"
#include "DiscreteFactorGraph.h"
#include "StateMask.h"

using namespace Rcpp;

//Helper functions
phy::DFG rToDFG(IntegerVector varDimensions, List facPotentials, List facNeighbors, IntegerVector potentialMap){
    //Make conversions
  std::vector<unsigned> varDim(varDimensions.begin(), varDimensions.end() );
  std::vector<unsigned> potMap(potentialMap.begin(), potentialMap.end() );
  
  std::vector<phy::matrix_t> facPot;
  for(int k = 0; k < facPotentials.size(); ++k){
    facPot.push_back( rMatToMat( facPotentials[k] ));
  }

  std::vector< std::vector<unsigned> > facNbs = rNbsToNbs( facNeighbors);

  return phy::DFG(varDim, facPot, facNbs, potMap);
} 

//Function definitions
RDFG::RDFG(IntegerVector varDimensions, List facPotentials, List facNeighbors, IntegerVector potentialMap) 
  : dfg(rToDFG(varDimensions, facPotentials, facNeighbors, potentialMap)) {}

NumericVector RDFG::expect(List facScores){
  // Convert to matrix
  std::vector<phy::matrix_t> facScoresVec;
  for(int k = 0; k < facScores.size(); ++k){
    facScoresVec.push_back( rMatToMat( facScores[k] ));
  }

  // Generate statemasks
  phy::stateMaskVec_t stateMasks( dfg.variables.size() );

  // Set factor scores
  dfg.resetScores( facScoresVec);
  std::pair<phy::number_t, phy::number_t> res = dfg.calcExpect(stateMasks);

  NumericVector ret(2);
  ret(0) = res.first;
  ret(1) = res.second;
  return ret;
}

IntegerMatrix RDFG::simulate(int N){
  // Setup
  boost::mt19937 gen(std::time(0));

  // Calculate marginals
  phy::stateMaskVec_t stateMasks( dfg.variables.size() );
  dfg.runSumProduct(stateMasks);
  dfg.calcVariableMarginals(stateMasks);
  dfg.calcFactorMarginals();
  const std::vector<phy::vector_t> & varMarginals = dfg.getVariableMarginals();
  const std::vector<phy::matrix_t> & facMarginals = dfg.getFactorMarginals();

  // Sample
  IntegerMatrix samples( N, dfg.variables.size() );
  for(int i = 0; i < N; ++i){
    std::vector<unsigned> sample(dfg.variables.size());
    dfg.sample(gen, varMarginals, facMarginals, sample);
    
    // Process
    for(int j = 0; j < sample.size(); ++j)
      samples(i,j) = sample[j] + 1; // C++ to R conversion
  }

  return samples;
}


// Preconditions
// observations either empty or length==variables.size()
// observed either empty or length==variables.size()
Rcpp::IntegerVector RDFG::maxProbState(Rcpp::IntegerVector observations, Rcpp::LogicalVector observed){
  //Create empty statemasks
  phy::stateMaskVec_t stateMasks( dfg.variables.size() );

  //Observed variables
  if(observed.size() == dfg.variables.size()){
    for(int i = 0; i < dfg.variables.size(); ++i){
      if(observed[i]){
	stateMasks.at(i) = phy::stateMaskPtr_t( new phy::StateMaskObserved(observations[i] -1) );
      }
    }
  }

  //Create return vector
  std::vector<unsigned> maxVarStates( dfg.variables.size() );

  //Calculate most probable state
  dfg.runMaxSum(stateMasks, maxVarStates);
  
  // 0-1 C++-R index conversion
  for(std::vector<unsigned>::iterator it = maxVarStates.begin(); it != maxVarStates.end(); ++it )
    (*it)++;

  return Rcpp::wrap(maxVarStates);
}

Rcpp::List RDFG::facExpCounts(Rcpp::IntegerMatrix observations ){
  if(observations.ncol() != dfg.variables.size() )
    phy::errorAbort("ncol != variables.size");

  dfg.clearCounts();

  //Each row in the matrix is an observation
  for(int i = 0; i < observations.nrow(); ++i){
    //Create statemasks
    phy::stateMaskVec_t stateMasks( dfg.variables.size());

    for(int j = 0; j < observations.ncol(); ++j){
      if( ! IntegerMatrix::is_na(observations(i, j))){
	stateMasks.at(j) = phy::stateMaskPtr_t( new phy::StateMaskObserved(observations(i, j) -1) );
      }
    }

    //calculation
    std::vector<phy::matrix_t> tmpFacMar;
    dfg.initFactorMarginals( tmpFacMar );
    dfg.runSumProduct( stateMasks );
    dfg.calcFactorMarginals( tmpFacMar );
    dfg.submitCounts( tmpFacMar );
  }

  //Convert to list of matrices
  std::vector<phy::matrix_t> potCountsVec;
  dfg.getCounts(potCountsVec);
  Rcpp::List ret(potCountsVec.size());

  for(int p = 0; p < potCountsVec.size(); ++p){
    phy::matrix_t & potCounts = potCountsVec.at(p);
    Rcpp::NumericMatrix rPotCounts(potCounts.size1(), potCounts.size2());
    for(int i = 0; i < potCounts.size1(); ++i)
      for(int j = 0; j < potCounts.size2(); ++j)
	rPotCounts(i,j) = potCounts(i,j);

    ret[p] = rPotCounts;
  }


  return ret;
}

// Accessors
void RDFG::resetPotentials(List facPotentials){
  // Convert to matrix
  std::vector<phy::matrix_t> facPot;
  for(int k = 0; k < facPotentials.size(); ++k){
    facPot.push_back( rMatToMat( facPotentials[k] ));
  }

  // Set factor potentials
  dfg.resetPotentials( facPot);
}

void RDFG::resetScores(List facScores){
  // Convert to matrix
  std::vector<phy::matrix_t> facScoresVec;
  for(int k = 0; k < facScores.size(); ++k){
    facScoresVec.push_back( rMatToMat( facScores[k] ));
  }

  // Set factor potentials
  dfg.resetPotentials( facScoresVec);
}

List RDFG::getPotentials(){
  // Loop over factor nodes
  std::vector<phy::matrix_t> ret;
  dfg.getPotentials( ret);
  return facPotToRFacPot( ret);
}


// Preconditions
// observations either empty or length==variables.size()
// observed either empty or length==variables.size()
double RDFG::calcLikelihood(Rcpp::IntegerVector observations, Rcpp::LogicalVector observed){
  return std::exp( calcLogLikelihood(observations, observed));
}

double RDFG::calcLogLikelihood(Rcpp::IntegerVector observations, Rcpp::LogicalVector observed){
  // Create empty statemasks
  phy::stateMaskVec_t stateMasks( dfg.variables.size() );

  // Observed variables
  if(observed.size() == dfg.variables.size()){
    for(int i = 0; i < dfg.variables.size(); ++i){
      if(observed[i]){
	stateMasks.at(i) = phy::stateMaskPtr_t( new phy::StateMaskObserved(observations[i] -1) );
      }
    }
  }

  return dfg.calcLogNormConst(stateMasks);
}
