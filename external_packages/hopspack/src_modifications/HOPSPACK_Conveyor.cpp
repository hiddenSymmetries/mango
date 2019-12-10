// $Id: HOPSPACK_Conveyor.cpp 169 2010-04-21 19:35:53Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-framework/HOPSPACK_Conveyor.cpp $ 

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
  \file HOPSPACK_Conveyor.cpp
  \brief Implement HOPSPACK::Conveyor
*/

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Conveyor.hpp"
#include "HOPSPACK_float.hpp"
#include "HOPSPACK_Print.hpp"
#include "HOPSPACK_SystemTimer.hpp"

#include "mango.hpp"

HOPSPACK::Conveyor::Conveyor (const ParameterList   &  cParams,
                              const bool               bHasNonlinearConstr,
                                    DataPoint * const  pInitialPoint,
			      Executor        &  cExecutor,
			      mango::problem* mango_problem_in) :
  executor(cExecutor),
  pCache(NULL),
  bIsCacheUsed(true),
  pendingList(),
  doSync(false),
  minReturn(1),
  maxReturn(1000)
{
  mango_problem = mango_problem_in;

    int  nTmp;

    //---- DEFINE THE EVALUATION REQUEST TYPE FOR THE EXECUTOR.
    if (bHasNonlinearConstr)
        evalReqType = EVALREQTYPE_FC;
    else
        evalReqType = EVALREQTYPE_F;

    bIsCacheUsed = cParams.getParameter ("Cache Enabled", bIsCacheUsed);
    if (bIsCacheUsed)
    {
        pCache = new CacheManager (cParams);
        if (pCache == NULL)
        {
            cerr << "ERROR: Could not construct Cache" << endl;
            throw INTERNAL_ERROR;
        }
    }

    doSync = cParams.getParameter ("Synchronous Evaluations", doSync);

    nTmp = cParams.getParameter ("Minimum Exchange Return", minReturn);
    if (nTmp >= 1)
    {
        minReturn = nTmp;
    }
    else
    {
        cerr << "WARNING: Illegal value 'Minimum Exchange Return' = " << nTmp
             << ", changed to " << minReturn << endl;
    }

    if (minReturn > maxReturn)
        maxReturn = minReturn;
    nTmp = cParams.getParameter ("Maximum Exchange Return", maxReturn);
    if (nTmp >= minReturn)
    {
        maxReturn = nTmp;
    }
    else
    {
        cerr << "WARNING: Illegal value 'Maximum Exchange Return' = " << nTmp
             << ", changed to " << maxReturn << endl;
    }

    //---- IF AN INITIAL POINT IS DEFINED BUT HAS NOT BEEN EVALUATED,
    //---- THEN LOOK IN THE CACHE OR IMMEDIATELY SUBMIT IT FOR EVALUATION.
    //---- IF DEFINED AND EVALUATED, THEN PUT IT IN THE CACHE.
    if (pInitialPoint != NULL)
    {
        if (Print::doPrint (Print::UNEVALUATED_POINTS))
        {
            cout << "Initial point from user parameters:" << endl;
            pInitialPoint->leftshift (cout, true);
            cout << endl;
        }
        if (pInitialPoint->getState() == DataPoint::UNEVALUATED)
        {
            Vector  cCachedF;
            Vector  cCachedEqs;
            Vector  cCachedIneqs;
            if (   (bIsCacheUsed == true)
                && (pCache->isCached (pInitialPoint->getX(),
                                      cCachedF, cCachedEqs, cCachedIneqs)) )
            {
                if (Print::doPrint (Print::QUEUE_LISTS))
                    cout << "Conveyor using point from cache for tag "
                         << pInitialPoint->getTag() << endl;
                counter.incrementCached();
                string  sMsg;
                counter.getCountString (sMsg);
                pInitialPoint->setCachedFC (cCachedF,
                                            cCachedEqs, cCachedIneqs,
                                            sMsg);
            }
            else
            {
                executor.submit (pInitialPoint->getTag(),
                                 pInitialPoint->getX(),
                                 evalReqType);

                //---- COPY THE INITIAL POINT BEFORE PLACING IT ON pendingList.
                //---- THIS IS BECAUSE pendingList WILL TAKE OWNERSHIP, BUT
                //---- cache.insert (THE ACTION TAKEN IF THE POINT IS ALREADY
                //---- EVALUATED) WILL NOT.  THUS, pendingList WILL EVENTUALLY
                //---- DELETE pCopyInitialPoint, AND THE CALLER WILL DELETE
                //---- pInitialPoint.
                DataPoint *  pCopyInitialPoint = new DataPoint (*pInitialPoint);
                pendingList.push (pCopyInitialPoint);
            }
        }
        else
        {
            if (bIsCacheUsed)
            {
                pCache->insert (pInitialPoint->getX(),
                                pInitialPoint->getVecF(),
                                pInitialPoint->getEqs(),
                                pInitialPoint->getIneqs());
            }
        }
    }

    return;
}


HOPSPACK::Conveyor::~Conveyor()
{
  pendingList.prune();

  multimap<int, DataPoint*>::iterator it;
  for (it=pendingPrime.begin(); it!=pendingPrime.end(); it++)
    delete (it->second);

  if (pCache != NULL)
      delete pCache;

  return;
}


void HOPSPACK::Conveyor::exchange
    (const vector< ConveyorList * > &  ctznQueueLists,
           ConveyorList             &  evalList)
{
  DataPoint* ptr;
  int tag;
  Vector f;
  Vector cEqs;
  Vector cIneqs;
  string msg;

  bool  bHaveAddedCachedPoints = false;
  bool  bIsCurrentLoopIdle;

  while (   ((hasAnotherPoint_ (ctznQueueLists)) || (pendingList.size() > 0))
         && ((doSync) || (evalList.size() < minReturn)) )
  {
    bIsCurrentLoopIdle = true;

    // Submit as many points as the executor will take
    while (executor.isReadyForWork())
    {
      ptr = popNextPoint_ (ctznQueueLists);
      if (ptr == NULL)
          break;

      //---- THE NEXT POINT WILL BE ASSIGNED, SO THE LOOP WILL DO SOME WORK.
      bIsCurrentLoopIdle = false;

      int pendingTag;

      // Is point cached?
      if (bIsCacheUsed && pCache->isCached (ptr->getX(), f, cEqs, cIneqs))
      {
          if (Print::doPrint (Print::QUEUE_LISTS))
          {
              cout << "Conveyor using point from cache for tag "
                   << ptr->getTag() << endl;
          }
          counter.incrementCached();
          string  sMsg;
          counter.getCountString (sMsg);
          ptr->setCachedFC (f, cEqs, cIneqs, sMsg);
          evalList.push(ptr);
      }
      // Is point currently being evaluated?
      else if (pendingList.contains(ptr, pendingTag))
      {
          pendingPrime.insert(make_pair(pendingTag, ptr));
      }
      // Point is neither in cache nor pending.  Submit new point.
      else
      {
          executor.submit (ptr->getTag(), ptr->getX(), evalReqType);
          pendingList.push(ptr);
      }
    }
    if (Print::doPrint (Print::QUEUE_LISTS))
        pendingList.print ("Conveyor pendingList");


    // Process points received from the executor.
    // Do not call recv() if there is no room in the evalList.
    int id;
    multimap<int, DataPoint*>::iterator it;
    while (   ((doSync == true) || (evalList.size() < maxReturn))
           && ((id = executor.recv (tag,f,cEqs,cIneqs,msg)) != 0) )
    {
      bIsCurrentLoopIdle = false;

      // Process received point.
      counter.incrementEvaluated(id, msg);
      ptr = pendingList.pop(tag);

      if (ptr == NULL)
      {
          cerr << "WARNING: Conveyor received NULL as result from Executor"
               << endl;
          cerr << "         Ignoring Executor results from tag "
               << tag << endl;
      }
      else
      {
          string  sMsg;
          counter.getCountString (sMsg);
          ptr->setEvalFC (f, cEqs, cIneqs, sMsg);

	  mango_problem->write_hopspack_line_to_file(msg, f[0]);
	  /*
	  int rank;
	  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	  cout << "Conveyor: proc " << rank << "received message: " << msg << endl;
	  */
          if (Print::doPrint (Print::EVALUATED_POINTS))
          {
            cout << "Conveyor received evaluated point:" << endl;
            ptr->leftshift (cout, true);
            cout << endl;
          }

          if (bIsCacheUsed)
              pCache->insert (ptr->getX(), f, cEqs, cIneqs);
          evalList.push (ptr);

          // Assign all points in pendingPrime associated to the tag
          // of the newly evaluated point.
          for (it = pendingPrime.lower_bound(tag);
               it != pendingPrime.upper_bound(tag); it++)
          {
              DataPoint *  pPendingPoint = it->second;
              counter.incrementPendingCached();
              counter.getCountString (sMsg);
              pPendingPoint->setCachedFC (f, cEqs, cIneqs, sMsg);
              evalList.push (pPendingPoint);
          }
          // evalList now owns these points, delete ownership from pendingPrime.
          pendingPrime.erase(pendingPrime.lower_bound(tag),
                             pendingPrime.upper_bound(tag));
      }
    }     //-- WHILE EXECUTOR IS RECEIVING POINTS

    //---- AFTER GETTING EXECUTOR RESULTS, ADD ANY CACHED POINTS.
    //---- ONLY NEED TO DO IT ONCE.
    if (bHaveAddedCachedPoints == false)
    {
        addCachedPoints_ (ctznQueueLists, doSync, maxReturn, evalList);
        bHaveAddedCachedPoints = true;
    }

    //---- THE CURRENT LOOP IS IDLE IF NO POINTS WERE SENT TO OR RECEIVED FROM
    //---- THE EXECUTOR.  TO PREVENT CPU SPINNING, WASTE A LITTLE TIME.
    if (bIsCurrentLoopIdle == true)
    {
        SystemTimer::sleepMilliSecs (1);
    }

  }     //-- WHILE MORE CITIZEN POINTS TO SUBMIT
  return;
}


const HOPSPACK::EvalCounter& HOPSPACK::Conveyor::getEvalCounter() const
{
  return counter;
}


int HOPSPACK::Conveyor::getNumPending() const
{
  return pendingList.size();
}


void HOPSPACK::Conveyor::printDebugInfo() const
{
    cout << "  HOPSPACK_Conveyor" << endl;
    cout << "    Minimum Exchange Return = " << minReturn << endl;
    cout << "    Maximum Exchange Return = " << maxReturn << endl;
    cout << "    Synchronous Evaluations = " << doSync << endl;
    cout << "    eval req type       = " << evalReqType << endl;
    cout << "    pendingList length  = " << pendingList.size() << endl;
    cout << "    pendingPrime length = " << pendingPrime.size() << endl;

    if (bIsCacheUsed)
        pCache->printDebugInfo();
    else
        cout << "    cache is disabled" << endl;

    executor.printDebugInfo();

    return;
}


//----------------------------------------------------------------------
//  Private methods
//----------------------------------------------------------------------

bool  HOPSPACK::Conveyor::hasAnotherPoint_
    (const vector< ConveyorList * > &  queueLists) const
{
    bool  bResult = false;
    for (int  i = 0; i < (int) queueLists.size(); i++)
    {
        if (queueLists[i]->size() > 0)
        {
            bResult = true;
            break;
        }
    }
    return bResult;
}


HOPSPACK::DataPoint *  HOPSPACK::Conveyor::popNextPoint_
    (const vector< ConveyorList * > &  queueLists) const
{
    if (queueLists.size() == 0)
        return( NULL );

    //---- IF JUST ONE CITIZEN QUEUE, THEN POP ITS NEXT POINT.
    if (queueLists.size() == 1)
    {
        ConveyorList *  pW = queueLists[0];
        if (pW->isEmpty())
            return( NULL );
        return( pW->pop() );
    }


    //---- THERE ARE MULTIPLE QUEUES TO SEARCH.  FIND THE HIGHEST PRIORITY
    //---- QUEUES, CHOOSE ONE, AND POP ITS NEXT POINT.

    int  nHiPriority = 9999999;
    for (int  i = 0; i < (int) queueLists.size(); i++)
        if (queueLists[i]->isEmpty() == false)
            nHiPriority = min (nHiPriority, queueLists[i]->getPriority());
    if (nHiPriority == 9999999)
        //---- ALL QUEUES ARE EMPTY.
        return( NULL );

    int  nNumHiPriorityCitizens = 0;
    for (int  i = 0; i < (int) queueLists.size(); i++)
    {
        if (   (queueLists[i]->isEmpty() == false)
            && (queueLists[i]->getPriority() == nHiPriority) )
        {
            nNumHiPriorityCitizens++;
        }
    }

    int  k = (int) (nNumHiPriorityCitizens * genRandomNumber());
    int  j = 0;
    for (int  i = 0; i < (int) queueLists.size(); i++)
    {
        if (   (queueLists[i]->isEmpty() == false)
            && (queueLists[i]->getPriority() == nHiPriority) )
        {
            if (k == j)
                return( queueLists[i]->pop() );
            j++;
        }
    }

    //---- SHOULD NOT REACH HERE.
    throw INTERNAL_ERROR;
}


void  HOPSPACK::Conveyor::addCachedPoints_
          (const vector< ConveyorList * > &  queueLists,
           const bool                        bIsNoSizeLimit,
           const int                         nMaxEvalSize,
                 ConveyorList             &  evalList)
{
    if (bIsCacheUsed == false)
        return;

    if (queueLists.size() == 0)
        return;

    //---- INITIALIZE TRACKING FLAGS.
    bool *  baPopNextFailed = new bool[queueLists.size()];
    for (int  i = 0; i < (int) queueLists.size(); i++)
    {
        if (queueLists[i]->isEmpty() == true)
            baPopNextFailed[i] = true;
        else
            baPopNextFailed[i] = false;
    }

    while (bIsNoSizeLimit || (evalList.size() < nMaxEvalSize))
    {
        int   nQueueIndex;
        if (queueLists.size() == 1)
        {
            //---- JUST ONE CITIZEN QUEUE.

            if (baPopNextFailed[0] == true)
            {
                //---- NO POINTS FROM THE CITIZEN WERE FOUND IN THE CACHE.
                delete[] baPopNextFailed;
                return;
            }
            nQueueIndex = 0;
        }
        else
        {
            //---- THERE ARE MULTIPLE QUEUES TO SEARCH.  FIND THE HIGHEST
            //---- PRIORITY QUEUES, CHOOSE ONE, AND POP ITS NEXT CACHED POINT.

            int  nHiPriority = 9999999;
            for (int  i = 0; i < (int) queueLists.size(); i++)
                if (baPopNextFailed[i] == false)
                    nHiPriority = min (nHiPriority,
                                       queueLists[i]->getPriority());
            if (nHiPriority == 9999999)
            {
                //---- ALL QUEUES HAVE BEEN CLEARED OF CACHED POINTS.
                delete[] baPopNextFailed;
                return;
            }

            int  nNumHiPriorityCitizens = 0;
            for (int  i = 0; i < (int) queueLists.size(); i++)
            {
                if (   (baPopNextFailed[i] == false)
                    && (queueLists[i]->getPriority() == nHiPriority) )
                {
                    nNumHiPriorityCitizens++;
                }
            }

            int  k = (int) (nNumHiPriorityCitizens * genRandomNumber());
            int  j = 0;
            for (int  i = 0; i < (int) queueLists.size(); i++)
            {
                if (   (baPopNextFailed[i] == false)
                    && (queueLists[i]->getPriority() == nHiPriority) )
                {
                    if (k == j)
                    {
                        nQueueIndex = i;
                        break;
                    }
                    j++;
                }
            }
        }

        //---- SEARCH FOR A CACHED POINT IN THE CHOSEN CITIZEN QUEUE.
        ConveyorList *  pList = queueLists[nQueueIndex];
        DataPoint *  nextPt = pList->popNextCached (pCache);
        if (nextPt != NULL)
        {
            Vector  cF;
            Vector  cEqs;
            Vector  cIneqs;
            pCache->isCached (nextPt->getX(), cF, cEqs, cIneqs);
            if (Print::doPrint (Print::QUEUE_LISTS))
            {
                cout << "Conveyor using point from cache for tag "
                     << nextPt->getTag() << endl;
            }
            counter.incrementCached();
            string  sMsg;
            counter.getCountString (sMsg);
            nextPt->setCachedFC (cF, cEqs, cIneqs, sMsg);
            evalList.push (nextPt);
        }
        else
        {
            //---- NO POINTS FROM THE CITIZEN WERE FOUND IN THE CACHE.
            baPopNextFailed[nQueueIndex] = true;
        }
    }

    delete[] baPopNextFailed;
    return;
}
