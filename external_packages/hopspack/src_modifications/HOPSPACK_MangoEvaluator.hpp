// $Id: ExampleLinkedEvaluator.hpp 166 2010-03-22 19:58:07Z tplante $
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/examples/linked-evaluator-example/ExampleLinkedEvaluator.hpp $

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
  @brief Declaration for ExampleLinkedEvaluator, subclass of Evaluator.
*/

#ifndef HOPSPACK_MANGOEVALUATOR_HPP
#define HOPSPACK_MANGOEVALUATOR_HPP

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Evaluator.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Vector.hpp"

#include "mango.hpp"
#include "Solver.hpp"
//----------------------------------------------------------------------
//! Implements HOPSPACK::Evaluator as a linked application.
/*!
 *  This example illustrates one way to link an application directly with
 *  HOPSPACK, instead of accessing it through a system call from HOPSPACK.
 *  Evaluation of the objective function and nonlinear constraints occurs
 *  directly in this class.
 *
 *  To use this class, modify source code where the Evaluator is instantiated:
 *    serial version         - HOPSPACK_main_serial.cpp
 *    MPI version            - HOPSPACK_main_mpi.cpp
 *    multi-threaded version - HOPSPACK_ExecutorMultiThreaded.cpp
 *
 *  See README_linked_evaluator.txt for more information.
 */
//----------------------------------------------------------------------

namespace HOPSPACK {
class MangoEvaluator : public HOPSPACK::Evaluator
{
  public:

    //! Constructor.
    /*!
     *  New evaluator parameters can be defined for this implementation
     *  and passed in the configuration parameters file.
     *
     *  @param[in] cEvalParams  Parameters in the "Evaluator" sublist.
     *                          Parameter value "Evaluator Type" determines
     *                          the particular implementation.
     */
  MangoEvaluator (const HOPSPACK::ParameterList &  cEvalParams, mango::Solver*);

    //! Destructor.
    ~MangoEvaluator (void);


    //! Evaluate the objective function(s) at a point x.
    /*!
     *  @param[in] nTag   Contains a unique tag for the evaluation which can be
     *                    used to name files, etc.
     *  @param[in] cX     The point at which to evaluate the function(s).
     *  @param[out] cFns  On output, contains a vector of objective function
     *                    values computed at X.  Multiple objectives are allowed.
     *                    If an evaluation failed, return an empty vector or set
     *                    individual elements of the vector to HOPSPACK::dne().
     *  @param[out] sMsg  On output, contains a message about the evaluation;
     *                    typically the word "Success" or an error message.
     */
    void  evalF (const int                 nTag,
                 const HOPSPACK::Vector &  cX,
                       HOPSPACK::Vector &  cFns,
                       string &            sMsg);

    //! Evaluate the objective functions and nonlinear constraints at a point x.
    /*!
     *  @param[in] nTag     Contains a unique tag for the evaluation which can be
     *                      used to name files, etc.
     *  @param[in] cX       The point at which to evaluate the function(s).
     *  @param[out] cFns    On output, contains a vector of objective function
     *                      values computed at X.  Multiple objectives are
     *                      allowed.  If an evaluation failed, return an empty
     *                      vector or set individual function elements of the
     *                      vector to HOPSPACK::dne().
     *  @param[out] cEqs    On output, contains a vector of nonlinear equality
     *                      constraint function values computed at X.  If an
     *                      evaluation failed, return an empty vector or set
     *                      individual elements of the vector to HOPSPACK::dne().
     *  @param[out] cIneqs  On output, contains a vector of nonlinear inequality
     *                      constraint function values computed at X.  If an
     *                      evaluation failed, return an empty vector or set
     *                      individual elements of the vector to HOPSPACK::dne().
     *  @param[out] sMsg    On output, contains a message about the evaluation;
     *                      typically the word "Success" or an error message.
     */
    void  evalFC (const int                 nTag,
                  const HOPSPACK::Vector &  cX,
                        HOPSPACK::Vector &  cFns,
                        HOPSPACK::Vector &  cEqs,
                        HOPSPACK::Vector &  cIneqs,
                        string &            sMsg);

    //! Print debug information about the Evaluator instance.
    void  printDebugInfo (void) const;


  private:

    //! By design, there is no copy constructor.
    MangoEvaluator (const MangoEvaluator &);
    //! By design, there is no assignment operator.
    MangoEvaluator & operator= (const MangoEvaluator &);

  double  evaluateF_ (const HOPSPACK::Vector &  cX, string & sMsg) const;
    void    evaluateCIneqs_ (const HOPSPACK::Vector &  cX,
                                   HOPSPACK::Vector &  cIneqs) const;
  mango::Solver* solver;

};
} // namespace HOPSPACK

#endif     //-- HOPSPACK_MANGOEVALUATOR_HPP
