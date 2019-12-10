// $Id: ExampleLinkedEvaluator.cpp 166 2010-03-22 19:58:07Z tplante $
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/examples/linked-evaluator-example/ExampleLinkedEvaluator.cpp $

//@HEADER
// ************************************************************************
// 
//         HOPSPACK: Hybrid Optimization Parallel Search Package
//                 Copyright 2009-2010 Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// This file is part of HOPSPACK.
//
// HOPSPACK is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see http://www.gnu.org/licenses/.
// 
// Questions? Contact Tammy Kolda (tgkolda@sandia.gov)
//                 or Todd Plantenga (tplante@sandia.gov) 
// 
// ************************************************************************
//@HEADER

/*!
  @file ExampleLinkedEvaluator.hpp
  @brief Implement ExampleLinkedEvaluator.

  This example provides an evaluator that compiles and links with the
  HOPSPACK framework.  Code must be thread-safe, and all virtual methods
  from HOPSPACK::Evaluator are implemented.

  The problem is:

    maximize    f(x,y) = x + 2y

    subject to  x + y >= 1
                1 - (x - 1)^2 - (y - 1)^2 >= 0

  The problem constrains points to lie within a circle centered at (1,1).
  HOPSPACK expects nonlinear inequalities to return a negative value if in
  violation of the constraint.  A nonnegative value means there is no violation.

  The exact solution point is x = 1.44721 (1 +  sqrt(5)/5),
                              y = 1.89443 (1 + 2sqrt(5)/5),
                        where f = 5.23607
  Only the nonlinear inequality is active at the solution, with sensitivity
  value  1.11803 (sqrt(5)/2).
*/

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_MangoEvaluator.hpp"
#include "HOPSPACK_float.hpp"

#include "mango.hpp"

//----------------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------------
HOPSPACK::MangoEvaluator::MangoEvaluator
(const HOPSPACK::ParameterList &  cEvalParams, mango::problem* this_problem_in)
{
    //---- THIS SIMPLE EXAMPLE DOES NOT USE EVALUATOR PARAMETERS.
  this_problem = this_problem_in;
    return;
}


//----------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------
HOPSPACK::MangoEvaluator::~MangoEvaluator (void)
{
    return;
}


//----------------------------------------------------------------------
//  Method evalF
//----------------------------------------------------------------------
void  HOPSPACK::MangoEvaluator::evalF (const int                 nTag,
                                     const HOPSPACK::Vector &  cX,
                                           HOPSPACK::Vector &  cFns,
                                           string &            sMsg)
{
  cFns.push_back (evaluateF_ (cX, sMsg));

  //sMsg = "Success";
  //sMsg = "Mango! evalF";
  return;
}


//----------------------------------------------------------------------
//  Method evalFC
//----------------------------------------------------------------------
void  HOPSPACK::MangoEvaluator::evalFC (const int                 nTag,
                                      const HOPSPACK::Vector &  cX,
                                            HOPSPACK::Vector &  cFns,
                                            HOPSPACK::Vector &  cEqs,
                                            HOPSPACK::Vector &  cIneqs,
                                            string &            sMsg)
{
  cFns.push_back (evaluateF_ (cX, sMsg));
    evaluateCIneqs_ (cX, cIneqs);

    //sMsg = "Success";
    sMsg = "Mango! evalFC";
    return;
}


//----------------------------------------------------------------------
//  Method printDebugInfo
//----------------------------------------------------------------------
void  HOPSPACK::MangoEvaluator::printDebugInfo (void) const
{
    cout << "      HOPSPACK::MangoEvaluator --"
         << " call compiled code for evaluations" << endl;

    return;
}


//----------------------------------------------------------------------
//  Private method evaluateF_
//----------------------------------------------------------------------
double  HOPSPACK::MangoEvaluator::evaluateF_
(const HOPSPACK::Vector &  cX, string & sMsg) const
{
  //    double  f = cX[0] + (2 * cX[1]);
  //  double  f = (cX[0] - 1) * (cX[0] - 1) + 3 * (cX[1] - cX[0]*cX[0]) * (cX[1] - cX[0]*cX[0]);
  //  return( f );
  double f;
  bool failed;

  // The next 2 lines effectively convert a HOPSPACK::Vector to a double array.
  std::vector<double> vec = cX.getStlVector();
  double* x = &vec[0];

  // Call Mango objective function
  this_problem->objective_function_wrapper(x, &f, &failed);
  if (failed) f = HOPSPACK::dne();

  // Prepare the line to write to the output file. This string will be passed by MPI to proc 0 to write to the output file,
  // since only proc 0 knows the global # of function evaluations.
  this_problem->compose_x_f_string(sMsg, x, f);
  if (this_problem->least_squares) {
    string residuals_string;
    // this_problem->residuals has been set by least_squares_to_single_objective(), which is objective_function(), which was called by objective_function_wrapper().
    this_problem->compose_residuals_string(residuals_string, this_problem->residuals);
    sMsg += residuals_string;
  }

  //std::cout << "Here comes sMsg from HOPSPACK::MangoEvaluator:" << sMsg << endl;
  return(f);
}


//----------------------------------------------------------------------
//  Private method evaluateCIneqs_
//----------------------------------------------------------------------

void  HOPSPACK::MangoEvaluator::evaluateCIneqs_
    (const HOPSPACK::Vector &  cX,
           HOPSPACK::Vector &  cIneqs) const
{
  /*    cIneqs.resize (1);
    cIneqs[0] = 1 - (cX[0] - 1)*(cX[0] - 1) - (cX[1] - 1)*(cX[1] - 1);
  */

  // For now, Mango does not support nonlinear constraints, so do nothing here.
    return;
}
