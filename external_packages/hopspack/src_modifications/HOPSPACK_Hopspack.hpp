// $Id: HOPSPACK_Hopspack.hpp 166 2010-03-22 19:58:07Z tplante $
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-main/HOPSPACK_Hopspack.hpp $

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
  @file HOPSPACK_Hopspack.hpp
  @brief Class declaration for HOPSPACK::Hopspack.
*/

#ifndef HOPSPACK_HOPSPACK_HPP
#define HOPSPACK_HOPSPACK_HPP

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Executor.hpp"
#include "HOPSPACK_Mediator.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_LinConstr.hpp"
#include "HOPSPACK_ProblemDef.hpp"

#include "mango.hpp"

namespace HOPSPACK
{


//----------------------------------------------------------------------
//! Provides a callable API for HOPSPACK.
/*!
 *  An instance of this class sets up a Mediator and Citizens for solving
 *  an optimization problem.  The caller provides an Executor instance
 *  and the parameters that define the problem and solver configuration.
 *  The solver can be invoked and queried for the final answer.
 *
 *  From this simple API users can write their own "main" program that
 *  calls HOPSPACK, instead of using one of the main programs provided with
 *  HOPSPACK.  See HOPSPACK_main_serial.cpp for an example.
 */
//----------------------------------------------------------------------
class Hopspack
{
  public:

    //! Constructor.
    /*!
     *  @param[in,out] pExecutor   Executor instance for the Mediator to use.
     *  @return                    Instance awaiting initialization.
     */
  Hopspack (Executor * const  pExecutor);

    //! Destructor.
    ~Hopspack (void);


    //! Set configuration parameters for the optimization instance.
    /*!
     *  @param[in] cParams  Nested list of all input parameters.
     *  @return true        If arguments are OK.  If false then execution
     *                      with this instance cannot continue.
     *
     *  Parameters are parsed and used to define the optimization problem,
     *  linear constraints, the Mediator, and initial Citizens.
     */
  bool  setInputParameters (const ParameterList &  cParams, mango::problem* mango_problem);

    //! Read configuration parameters for the optimization instance.
    /*!
     *  @param[in] sFileName  File name containing all input parameters.
     *  @return true          If arguments are OK.  If false then execution
     *                        with this instance cannot continue.
     *
     *  Parameters are parsed and used to define the optimization problem,
     *  linear constraints, the Mediator, and initial Citizens.
     */
    bool  setInputParameters (const string &  sFileName);

    //! Solve the configured optimization problem.
    /*!
     *  @return state  Final optimization state indicates the best point
     *                 observed by the Mediator.  More detailed information
     *                 may be written by the citizen solvers.
     */
    FinalOptStateType  solve (void);

    //! Return the best point found after optimization.
    /*!
     *  @param[out] cBestX  Coordinates of the best point observed by the
     *                      Mediator.  The returned state of optimize()
     *                      indicates whether this point is feasible or not.
     *  @return true        If the final optimization state was not ERROR.
     *
     *  The "best" point is a feasible point with the smallest (if minimizing,
     *  largest if maximizing) objective value.  If no trial points observed
     *  by the Mediator were feasible, then the "best" point is the one with
     *  the smallest infeasibility error, as measured by the infinity norm.
     *  Linear infeasibility is given priority over nonlinear constraint
     *  infeasibility.  See the notes under HOPSPACK::DataPoint::getBestF()
     *  for determining the "best" point when there are multiple objectives.
     */
    bool  getBestX (vector< double > &  cBestX) const;

    //! Return the single objective at the best point found after optimization.
    /*!
     *  @return  Objective value at the point returned by getBestX().
     *           If no point, then return HOPSPACK::dne().
     */
    double  getBestF (void) const;

    //! Return all objectives at the best point found after optimization.
    /*!
     *  @param[out] cBestF  Multiple objective values at the point returned by
     *                      getBestX().  If no point, then return an empty
     *                      vector.  If an individual element failed, the
     *                      returned element will be HOPSPACK::dne().
     */
    void  getBestVecF (vector< double > &  cBestF) const;

    //! Return nonlinear equality constraint values at the best point found.
    /*!
     *  @param[out] cBestEqs  Nonlinear equality constraint values at the point
     *                        returned by getBestX().  If no point, then return
     *                        an empty vector.
     */
    void  getBestNonlEqs (vector< double > &  cBestEqs) const;

    //! Return nonlinear inequality constraint values at the best point found.
    /*!
     *  @param[out] cBestIneqs  Nonlinear inequality constraint values at the
     *                          point returned by getBestX().  If no point,
     *                          then return an empty vector.
     */
    void  getBestNonlIneqs (vector< double > &  cBestIneqs) const;


  private:

    //! By design, there is no copy constructor.
    Hopspack (const Hopspack &);
    //! By design, there is assignment operator.
    Hopspack & operator= (const Hopspack &);

    //! Check the parameter list for basic mistakes.
    /*!
     *  @param[in] cParams    List of user parameter inputs.
     *  @return true          If parameter list is OK.
     */
    bool  checkParameterBasics_ (const ParameterList &  cParams) const;

    //! Print error messages related to the parameter list.
    /*!
     *  @param[in] sMsg  Error message to display.
     */
    void  printParamError_ (const string &  sMsg) const;

    //! Construct an initial point from the problem definition.
    /*!
     *  @param[in] cProbDef    Problem definition.
     *  @param[in] cLinConstr  Linear constraints definition.
     *  @return                Pointer to a feasible initial point, possibly
     *                         unevaluated, or NULL if none.
     *                         Client must delete the instance when finished.
     */
    DataPoint *  makeInitialPoint_ (      ProblemDef &  cProbDef,
                                    const LinConstr  &  cLinConstr) const;

    //! Make citizens from the parameter list, and add to the mediator.
    /*!
     *  @param[in] cParams     List of user parameter inputs.
     *  @param[in] cProbDef    Problem definition.
     *  @param[in] cLinConstr  Linear constraints definition.
     *  @return true           If at least one citizen successfully created.
     */
    bool  makeCitizens_ (const ParameterList &  cParams,
                         const ProblemDef    &  cProbDef,
                         const LinConstr     &  cLinConstr);


    Executor   *      _pExecutor;

    bool              _bAreParametersSet;
    ProblemDef *      _pProbDef;
    LinConstr  *      _pLinConstr;
    DataPoint  *      _pInitialPoint;
    Mediator   *      _pMediator;

    bool              _bHaveBestPoint;
    vector< double >  _cBestX;

  #if defined(_WIN32)
    int               _nWindowsSaveTimerMin;
  #endif
};

}          //-- namespace HOPSPACK

#endif     //-- HOPSPACK_HOPSPACK_HPP
