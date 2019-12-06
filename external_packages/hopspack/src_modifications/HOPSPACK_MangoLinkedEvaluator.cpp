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
#include "HOPSPACK_MangoLinkedEvaluator.hpp"


//----------------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------------
ExampleLinkedEvaluator::ExampleLinkedEvaluator
    (const HOPSPACK::ParameterList &  cEvalParams)
{
    //---- THIS SIMPLE EXAMPLE DOES NOT USE EVALUATOR PARAMETERS.
    return;
}


//----------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------
ExampleLinkedEvaluator::~ExampleLinkedEvaluator (void)
{
    return;
}


//----------------------------------------------------------------------
//  Method evalF
//----------------------------------------------------------------------
void  ExampleLinkedEvaluator::evalF (const int                 nTag,
                                     const HOPSPACK::Vector &  cX,
                                           HOPSPACK::Vector &  cFns,
                                           string &            sMsg)
{
    cFns.push_back (evaluateF_ (cX));

    sMsg = "Success";
    return;
}


//----------------------------------------------------------------------
//  Method evalFC
//----------------------------------------------------------------------
void  ExampleLinkedEvaluator::evalFC (const int                 nTag,
                                      const HOPSPACK::Vector &  cX,
                                            HOPSPACK::Vector &  cFns,
                                            HOPSPACK::Vector &  cEqs,
                                            HOPSPACK::Vector &  cIneqs,
                                            string &            sMsg)
{
    cFns.push_back (evaluateF_ (cX));
    evaluateCIneqs_ (cX, cIneqs);

    sMsg = "Success";
    return;
}


//----------------------------------------------------------------------
//  Method printDebugInfo
//----------------------------------------------------------------------
void  ExampleLinkedEvaluator::printDebugInfo (void) const
{
    cout << "      ExampleLinkedEvaluator --"
         << " call compiled code for evaluations" << endl;

    return;
}


//----------------------------------------------------------------------
//  Private method evaluateF_
//----------------------------------------------------------------------
double  ExampleLinkedEvaluator::evaluateF_
    (const HOPSPACK::Vector &  cX) const
{
  //    double  f = cX[0] + (2 * cX[1]);
  double  f = (cX[0] - 1) * (cX[0] - 1) + 3 * (cX[1] - cX[0]*cX[0]) * (cX[1] - cX[0]*cX[0]);
    return( f );
}


//----------------------------------------------------------------------
//  Private method evaluateCIneqs_
//----------------------------------------------------------------------

void  ExampleLinkedEvaluator::evaluateCIneqs_
    (const HOPSPACK::Vector &  cX,
           HOPSPACK::Vector &  cIneqs) const
{
  /*    cIneqs.resize (1);
    cIneqs[0] = 1 - (cX[0] - 1)*(cX[0] - 1) - (cX[1] - 1)*(cX[1] - 1);
  */
    return;
}
