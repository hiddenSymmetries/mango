// $Id: HOPSPACK_Mediator.hpp 183 2010-12-15 18:22:41Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-framework/HOPSPACK_Mediator.hpp $ 

//@HEADER
// ************************************************************************
// 
//         HOPSPACK: Hybrid Optimization Parallel Search Package
//                 Copyright 2009 Sandia Corporation
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
  @file HOPSPACK_Mediator.hpp
  @brief Class declaration for HOPSPACK::Mediator.
*/

#ifndef HOPSPACK_MEDIATOR_HPP
#define HOPSPACK_MEDIATOR_HPP

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_CallbackToMediator.hpp"
#include "HOPSPACK_Citizen.hpp"
#include "HOPSPACK_Conveyor.hpp"
#include "HOPSPACK_ConveyorList.hpp"
#include "HOPSPACK_DataPoint.hpp"
#include "HOPSPACK_Executor.hpp"
#include "HOPSPACK_LinConstr.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_ProblemDef.hpp"
#include "HOPSPACK_SystemTimer.hpp"
#include "HOPSPACK_Vector.hpp"

#include "mango.hpp"
#include "Solver.hpp"

namespace HOPSPACK
{


//----------------------------------------------------------------------
//! Manages citizen requests for evaluations.
/*! This is the "top level" class that mediates between citizens and their
    requests for evaluation of trial points.  The Mediator gathers citizen
    requests, coordinates evaluation of points, and notifies citizens
    of evaluation results.  The Mediator executes the main loop of work
    in HOPSPACK, continuing until a stop criteria is met.
 */
//----------------------------------------------------------------------
class Mediator : public CallbackToMediator
{
  public:

    //! Constructor.
    /*!
     *  @param[in] cMedParams     User input from "Mediator" sublist.
     *  @param[in] cProbDef       Problem definition, including scaling factors.
     *  @param[in] cLinConstr     Linear constraings.
     *  @param[in] pInitialPoint  Initial point provided by user, possibly NULL.
     *  @param[in,out] pExecutor  Fully constructed executor for evaluations.
     */
    Mediator (const ParameterList &        cMedParams,
              const ProblemDef    &        cProbDef,
              const LinConstr     &        cLinConstr,
                    DataPoint     * const  pInitialPoint,
	      Executor      * const  pExecutor,
	      mango::Solver* solver);

    //! Destructor.
    ~Mediator (void);


    //! Add a new citizen.
    /*!
     *  @param[in] pCitizen   Fully constructed and validated citizen.
     *                        Mediator will destroy the citizen when finished.
     *  @param[in] bIsChild   True if the new citizen is a dynamically created
     *                        child of an existing citizen.
     *  @param[in] nParentID  If bIsChild is true, then this is the ID number
     *                        of the citizen's parent.
     *  @return false         If the name already exists (and delete pCitizen).
     */
    bool  addCitizen (      Citizen * const  pCitizen,
                      const bool             bIsChild,
                      const int              nParentID);

    //! Coordinate sharing of resources among all citizen solvers.
    /*!
     *  The method loops until stopped.  Each iteration performs:
     *  <ul>
     *  <li> A call to "citizenExhange()", allowing citizens to view
     *       evaluated points and add new points to the current queue.
     *  <li> Clean up of evaluated points.
     *  <li> A call to "citizenSelectOrder()", where citizens vote on the
     *       priority of points in the queue.
     *  <li> A call to "conveyorExchange()", where computing resources are
     *       are invoked to trade unevaluated points for evaluated points.
     *       On exit, the mediator retains ownership of evaluated points and
     *       unevaluated points that are not pending evaluation.
     *  </ul>
     *
     *  The mediator has ownership of all points in the queue.
     */
    void  mediate (void);

    //! Return the best point found after optimization.
    /*!
     *  @param[out] cBestX  Coordinates of the best point observed.
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

    //! Return true if the best point found after optimization is feasible.
    bool  isBestFeasible (void) const;

    //! Return the single objective at the best point found after optimization.
    /*!
     *  @return  Objective value of the point returned by getBestX().
     *           If no point, then return HOPSPACK::dne().
     */
    double  getBestF (void) const;

    //! Return all objectives at the best point found after optimization.
    /*!
     *  @param[out] cBestF  Multiple objective values of the point returned by
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

    //! Print debug information about the instance.
    void  printDebugInfo (void) const;

    //@{ \name Implementation of CallbackToMediator interface

    //! Reserve and return a unique citizen ID.
    int  reserveUniqueCitizenID (void);

    //! Add a dynamically created child citizen and immediately preprocess it.
    /*!
     *  @param[in] pCitizen   Fully constructed and validated citizen.
     *                        Mediator will destroy the citizen when finished.
     *  @param[in] nParentID  Parent ID number.
     *  @return false         If the name already exists (and delete pCitizen).
     */
    bool  addChildCitizen (      Citizen * const  pCitizen,
                           const int              nParentID);

    //@}

  private:

    //! By design, there is no copy constructor.
    Mediator (const Mediator &);
    //! By design, there is assignment operator.
    Mediator & operator= (const Mediator &);

    //! Structure to hold citizen information.
    typedef struct
    {
        Citizen *       pCtzn;                //-- POINTER TO THE CITIZEN
        bool            bIsCtznFatalError;    //-- TRUE IF FATAL ERROR
        bool            bIsCtznChild;         //-- TRUE IF A CHILD CITIZEN
        int             nParentID;            //-- PARENT ID NUMBER
        int             nDeleteTag;           //-- TAG FOR MARKING DELETE LEVEL
        ConveyorList *  pWaitListWs;          //-- WAITING LIST OF TRIAL POINTS
    }
    CtznInfoBlockType;

    typedef list< CtznInfoBlockType * > CtznInfoBlockListType;


    //! Run once when the mediator starts.
    /*!
     *  Calls citizen preProcess() for all non-child citizens.  A non-child
     *  citizen is allowed to add child citizens during its preProcess().
     */
    void  preProcess_ (void);

    //! Run once when the mediator stops; calls citizen postProcess().
    void  postProcess_ (void);

    //! Return true if the "town" of citizens agrees to continue.
    bool  isTownActive_ (void) const;

    //! Let citizens see new evaluations and modify their trial point queue.
    /*!
     *  This method calls Citizen::exchange() for all citizens still
     *  accepting points.  As a side effect, it may modify pWaitListWs
     *  in the citizen information block, and _cPtOwnerMap.
     *
     *  @param[in] cR  List of points with evaluation results.
     *  @return        Total number of points submitted by all citizens
     *                 after the exchange.
     */ 
    int  citizenExchange_ (const ConveyorList &  cR);

    //! Erase list members from the owner map, then erase the list members.
    /*!
     *  As a side effect, update the count of evaluations for the citizen.
     *
     *  @param[in,out] cList  List of points to be erased.
     */ 
    void  eraseExchangeList_ (ConveyorList &  cList);

    //! Return a pointer to the parent, or NULL if no parent.
    /*!
     *  @param[in] pCtznInfo  Pointer to a citizen.
     *  @return               Pointer to the parent, or NULL if no parent.
     */
    CtznInfoBlockType *  getParentInfo_ (const CtznInfoBlockType *  pCtznInfo);

    //! Call setEarlyExit() on all citizens.
    void  setEarlyExitOnCitizens_ (void);

    //! End all citizens and their offspring.
    /*!
     *  @param[in] bOnlyIfFinished  If true, then only end child citizens in
     *                              the finished state.  If false, end all.
     *
     *  In all cases, loop over the list of citizens.  When a citizen is found
     *  that needs to end, search for any offspring it might have and end
     *  those first, from youngest to oldest.
     *  When a citizen is ended, first let it postprocess, then delete it
     *  and remove it from the list of citizens.
     */
    void  endCitizens_ (const bool  bOnlyIfFinished);

    //! Recursively mark child citizens for endMarkedCitizens_.
    /*!
     *  @param[in] pCtznInfo   Pointer to current parent citizen.  This and
     *                         all its children will be marked.
     *  @param[in] nDeleteTag  The delete tag field of pCtznInfo will be
     *                         marked with this value.  Children will be
     *                         marked with a smaller value.
     */
    void  markCitizensRecursively_ (      CtznInfoBlockType *  pCtznInfo,
                                    const int                  nDeleteTag);

    //! Postprocess and delete citizens marked by endCitizenRecurse_, in order.
    /*!
     *  The recursive search for children could not actually delete because
     *  outer procedures cannot continue to iterate over _cCitizenList.
     *  Instead, markCitizensRecursively_ finds all children and marks them
     *  with a number.  The lowest number is ended first, then the next lowest,
     *  until all marked citizens are gone.
     *
     *  Each citizen is allowed to postprocess, then deleted and removed
     *  from the list of citizens.
     */
    void  endMarkedCitizens_ (void);

    //! Prune all points in all of the lists.
    void  pruneAllPoints_ (void) const;

    //! Write a point to the solution file, if there is one.
    void  writePointToSolutionFile_ (const DataPoint &  cPoint) const;

    //! Return true if mediator looping should stop due to evaluation results.
    /*!
     *  @param[in] nNumNewCtznPoints  Number of new citizen points submitted
     *                                during this iteration.
     *  @param[in] bChildAdded        True if a new child citizen was added
     *                                during this iteration.
     *  @param[in] cList              Evaluated points received during this
     *                                iteration.
     *
     *  Looping can also be stopped by isTownActive_(), which is determined
     *  by the citizens.
     */
    bool  makeStopTest_ (const int             nNumNewCtznPoints,
                         const bool            bChildAdded,
                         const ConveyorList &  cList);

    //! Examine the conveyor list and update the best point found.
    void  updateBestPoint_ (const ConveyorList &  cList);

    //! Return true if the point passes all feasibility tests.
    bool  isCompletelyFeasible_ (const DataPoint * const  pPoint) const;

    //! Print evaluation and timing statistics.
    void  printEvalTimeStats_ (void) const;

    //! Print debug information about currently running citizens.
    void  printDebugCitizenInfo_ (void) const;


    //! Counter for unique citizen ID numbers.
    static int                  _nNextUniqueCitizenID;

    //! Reference to problem definition.
    const ProblemDef &          _cProbDef;

    //! Reference to linear constraints definition.
    const LinConstr &           _cLinConstr;

    //! List of citizens and their block of mediator-related information.
    CtznInfoBlockListType       _cCitizenList;

    //! Map the citizen name to a list of point tags it owns.
    map< string, list< int > >  _cPtOwnerMap;

    //! Map the citizen name to the number of evaluations on its behalf.
    map< string, int >          _cNumEvals;

    //! Maximum number of point evaluations allowed (user parameter).
    int                         _nMaxEvaluations;

    //! True if addChildCitizen() was called in the current Mediator iteration.
    bool                        _bChildAddedRecently;

    //! True if mediator is the process of halting.
    bool                        _bMediatorHalting;

    string                      _sSolutionFileName;
    int                         _nSolutionFilePrecision;

    Executor *                  _pExecutor;
    Conveyor *                  _pConveyor;
    int                         _nNumVars;
    DataPoint *                 _pBestPoint;
    SystemTimer *               _pTimers;
};

}          //-- namespace HOPSPACK

#endif     //-- HOPSPACK_MEDIATOR_HPP
