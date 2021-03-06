% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/dfg.R
\name{dfg}
\alias{dfg}
\title{dfg
Construct a probalistic graphical model}
\usage{
dfg(varDim, facPot, facNbs, potMap = 1:length(facPot),
  varNames = seq_along(varDim), facNames = (length(varDim) +
  seq_along(potMap)))
}
\arguments{
\item{varDim}{Vector of integers containing variable dimensions}

\item{facPot}{List of matrices containing potentials}

\item{facNbs}{List of vector containing describing neighbouring variables for each factor}

\item{potMap}{A vector of integers mapping with corresponding potential for each factor}

\item{varNames}{A character vector of names for each variable}

\item{facNames}{A character vector of names for each potential}
}
\description{
dfg
Construct a probalistic graphical model
}
\examples{
varDim <- c(2L,3L)
facPot <- list(matrix(c(1:6),2,3))
facNbs <- list( c(1L,2L))
mydfg <- dfg(varDim, facPot, facNbs)
}

