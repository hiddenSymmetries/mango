// $Id: HOPSPACK_Conveyor.hpp 149 2009-11-12 02:40:41Z tplante $
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-framework/HOPSPACK_Conveyor.hpp $

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
  \file HOPSPACK_Conveyor.hpp
  \brief Class declaration for HOPSPACK::Conveyor
*/
#ifndef HOPSPACK_CONVEYOR_HPP
#define HOPSPACK_CONVEYOR_HPP

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_ConveyorList.hpp"
#include "HOPSPACK_CacheManager.hpp"
#include "HOPSPACK_DataPoint.hpp"
#include "HOPSPACK_EvalCounter.hpp"
#include "HOPSPACK_Executor.hpp"
#include "HOPSPACK_ParameterList.hpp"

#include "mango.hpp"

namespace HOPSPACK
{
//! Conveys trial points through the process of being queued, cached, and evaluated.
/*!
  The conveyor's main interface is the exchange() method.  It takes a (possibly
  empty) list of unevaluated trial points from each citizen, schedules some
  points for evaluation, and returns a merged list of evaluated trial points
  from all citizens.  Scheduled points are removed from citizen queues, and
  may spend time on a "pendingList" before actual evaluation.

  The conveyor is given ownership of an Executor object to perform evaluations
  using full computing resources.

  The conveyor owns a CacheManager to avoid reevaluating "equal" points.
  The ScaledComparison object determines whether two points are "equal".

  Points that are currently in the process of being evaluated by the executor
  are stored in a "pendingList".  When a new point is considered by the conveyor,
  it is sent to the executor only if not found in the cache or pendingList.
*/
class Conveyor
{
public:

  //! Constructor
  /*!
   *  @param[in] cParams              User input from "Mediator" sublist.
   *  @param[in] bHasNonlinearConstr  True if the problem contains nonlinear
   *                                  constraints.
   *  @param[in] pInitialPoint        Initial point provided by user,
   *                                  possibly NULL.
   *  @param[in,out] cExecutor        Fully constructed executor for evaluations.
   */
  Conveyor (const ParameterList   &  cParams,
            const bool               bHasNonlinearConstr,
                  DataPoint * const  pInitialPoint,
	    Executor        &  cExecutor,
	    mango::problem* mango_problem_in);

  //! Destructor 
  ~Conveyor();

  //! Exchange a list of unevaluated trial points for a list of evaluated points.
  /*!
   *  Each citizen provides a (possibly empty) list of unevaluated points.
   *  Points are removed in order, according to the priority of the citizens.
   *  If multiple citizens have the same priority, then points are interleaved
   *  according to a randomization scheme.  Each point goes to either the
   *  pendingList (for evaluation), the evalList (if already cached),
   *  or the pendingPrime map (if already pending).
   *
   *  The cache and pendingPrime list determine a point to be "equal" by
   *  making a ScaledComparison.
   *
   *  @param[in,out]  ctznQueueLists
   *      List of new trial points to evaluate, one list per citizen.
   *      The list can be empty if citizens are waiting for points in the
   *      pending list.  On exit, each citizen list still contains and owns
   *      points not yet accepted by the conveyor.
   *  @param[out]  evalList
   *      On exit, contains a (possibly empty) list of newly evaluated points.
   *
  <br>
  <br>
  The flowchart below describes processing of new points.
    \dot
    digraph G{
      bgcolor=white
      fontcolor=blue
      label="Processing an unevaluated point"
      node[shape=ellipse, peripheries=1, color=blue]
      edge[color=red]
      rankdir = LR

      cache[label="Is point cached?"]
      yesCache[label="Copy eval info,\nmove point to evalList"]
      noCache[label="Is point pending?"]  

      cache -> yesCache [label="yes"]
      cache -> noCache [label="no"]

      yesPending[label="Move to pendingPrime map"]
      noPending[label="Submit evaluation,\nmove to pendingList"]
      noCache -> yesPending [label="yes"]
      noCache -> noPending [label="no"]
    }
    \enddot

  The flowchart below describes processing of evaluated points returned by
  the executor.
    \dot
      digraph G{
      bgcolor=white
      fontcolor=blue
      label="Processing an evaluated point"
      node[shape=ellipse, peripheries=1, color=blue]
      edge[color=red]
      rankdir = LR
	    
      tagq[label="Is point\nin pendingPrime?"]
      yestag[label="Copy eval info to all points\nin pendingPrime associated\nwith the evaluated point"]
      notag[label="Move new evaluations\nto evalList"]
      tagq -> yestag [label="yes"]
      tagq -> notag [label="no"]
      yestag -> notag
      }
    \enddot
  */
  void exchange (const vector< ConveyorList * > &  ctznQueueLists,
                       ConveyorList &              evalList);

  //! Returns counts of evaluations, evaluation messages, etc.
  const EvalCounter& getEvalCounter() const;

  //! Returns the number of points currently being evaluated.
  int getNumPending() const;

  //! Print debug information about the instance.
  void printDebugInfo() const;

private:

  //! Users must provide inputs, so the default constructor is hidden.
  Conveyor (void);
  //! By design, there is no copy constructor.
  Conveyor (const Conveyor &);
  //! By design, there is no assignment operator.
  Conveyor & operator= (const Conveyor &);

  //! Return true if at least one citizen has at least one point in its queue.
  bool  hasAnotherPoint_ (const vector< ConveyorList * > &  queueLists) const;

  //! Return the next point, or NULL if none remain.
  /*!
   *  Choose the next point according to priority rules:
   *  <ol>
   *  <li> Order of points within a citizen is always honored
   *  <li> Citizen with highest priority sends all its points first
   *  <li> If multiple citizens have the same priority, then pop the
   *       next point according to a randomization scheme.
   *  </ol>
   *
   *  @param[in] queueLists  New trial points, one list per citizen.
   *                         A list can be empty.
   */
  DataPoint * popNextPoint_ (const vector< ConveyorList * > &  queueLists) const;

  //! Add points found in the cache.
  /*!
   *  Loop through the remaining points submitted by citizens and add any
   *  evaluations that are found in the cache.
   *
   *  @param[in,out] queueLists  Lists of new trial points to evaluate,
   *                             one list per citizen.  Points may be removed
   *                             from a list if found in the cache.
   *  @param[in] bIsNoSizeLimit  True means any number of points can be added
   *                             to evalList; otherwise, obey nMaxEvalSize.
   *  @param[in] nMaxEvalSize    Maximum evalList size.
   *  @param[in,out] evalList    The list passed in is appended with any cached
   *                             results found.
   */
  void  addCachedPoints_ (const vector< ConveyorList * > &  queueLists,
                          const bool                        bIsNoSizeLimit,
                          const int                         nMaxEvalSize,
                                ConveyorList             &  evalList);


  //! Object that is used to evaluate trial points.
  Executor& executor;

  //! The default function value cache
  CacheManager *  pCache;

  //! True if evaluations are cached.
  bool  bIsCacheUsed;

  //! List of trial points in the process of being evaluated.
  ConveyorList pendingList;

  //! Maps new trial points to those in the process of being evaluated.
  /*!
    Associates new trial points (second argument) with a point that is
    deemed "equal" and is currently being evaluted (first argument).  The
    new point will eventually be assigned evaluation results from the cache.
  */
  multimap< int, DataPoint * > pendingPrime;

  //! The type of evaluation to request.
  EvalRequestType  evalReqType;

  //! True if evaluations should be synchronous (set according to "Synchronous")
  bool doSync;

  /*! \brief The minimum number of items that should be returned on a
    call to exchange() (set according to "Minimum Exchange Return") */
  int minReturn;

  /*! \brief The maximum number of items that should be returned on a
    call to exchange() (set according to "Maximum Exchange Return") */
  int maxReturn;

  //! Counter for the number of evaluations and evaluation messages.
  EvalCounter counter;

  mango::problem* mango_problem;
};

}

#endif
