// $Id: HOPSPACK_Mediator.cpp 183 2010-12-15 18:22:41Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-framework/HOPSPACK_Mediator.cpp $ 

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
  @file HOPSPACK_Mediator.cpp
  @brief Implement HOPSPACK::Mediator.
*/

#include <algorithm>     //-- FOR find
#include <iomanip>

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Conveyor.hpp"
#include "HOPSPACK_ConveyorList.hpp"
#include "HOPSPACK_DataPoint.hpp"
#include "HOPSPACK_float.hpp"
#include "HOPSPACK_LinConstr.hpp"
#include "HOPSPACK_Mediator.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Print.hpp"
#include "HOPSPACK_ProblemDef.hpp"
#include "HOPSPACK_ScaledComparison.hpp"
#include "HOPSPACK_SystemTimer.hpp"

namespace HOPSPACK
{


//----------------------------------------------------------------------
//  Static data initialization
//----------------------------------------------------------------------
static const string  szVERSION = "HOPSPACK 2.0.2";

int  Mediator::_nNextUniqueCitizenID = 0;

static const int  nMARKED_FOR_DELETION = -1;

//---- TIMER NUMBERS.
static const int  nTOTALTIME        = 0;
static const int  nPREPROCESSING    = 1;
static const int  nCTZNPROCESSING   = 2;
static const int  nPOSTPROCESSING   = 3;
static const int  nNUMBER_OF_TIMERS = 4;


//----------------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------------
Mediator::Mediator (const ParameterList &        cMediatorParams,
                    const ProblemDef    &        cProbDef,
                    const LinConstr     &        cLinConstr,
                          DataPoint     * const  pInitialPoint,
		    Executor      * const  pExecutor,
		    mango::problem* mango_problem)
    :
    _cProbDef (cProbDef),
    _cLinConstr (cLinConstr),
    _pExecutor (pExecutor),
    _pBestPoint (NULL)
{
  
    string  sDateTime;
    SystemTimer::getDateTime (sDateTime);
    cout << endl
         << "------------------------------------------------------" << endl
         << "HOPSPACK: Hybrid Optimization Parallel Pattern Search" << endl
         << "  TG Kolda, TD Plantenga, et al., Sandia National Labs" << endl
         << "Copyright 2009-2011 Sandia Corporation." << endl
         << "Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation," << endl
         << "the U.S. Government retains certain rights in this software." << endl
         << "------------------------------------------------------" << endl
         << szVERSION << ", started " << sDateTime << endl
         << endl;

    //---- NEW INSTANCE STARTS WITH ZERO CITIZENS (BUG TICKET #1, 08/26/2010).
    _nNextUniqueCitizenID = 0;

    _pTimers = new SystemTimer (nNUMBER_OF_TIMERS);

    _cProbDef.printDefinition (true);
    _cLinConstr.printDefinition (true);

    //---- DEFINE THE ScaledComparison GLOBAL INSTANCE USED FOR CACHING POINTS.
    if (cMediatorParams.isParameter ("Cache Comparison Tolerance"))
    {
        ScaledComparison::setTolerance
            (cMediatorParams.getDoubleParameter ("Cache Comparison Tolerance"));
    }
    const Vector &  cScaling = _cProbDef.getVarScaling();
    ScaledComparison::setScaling (cScaling);

    _nNumVars = cScaling.size();

    //---- CONSTRUCT A CONVEYOR, WHICH USES THE EXECUTOR.
    _pConveyor = new Conveyor (cMediatorParams,
                               _cProbDef.hasNonlinearConstr(),
                               pInitialPoint,
                               *_pExecutor,
			       mango_problem);

    _nMaxEvaluations = cMediatorParams.getParameter ("Maximum Evaluations", -1);
    if (_nMaxEvaluations < -1)
        _nMaxEvaluations = -1;

    if (cMediatorParams.isParameter ("Solution File"))
    {
        _sSolutionFileName
            = cMediatorParams.getParameter ("Solution File", "");
    }
    _nSolutionFilePrecision
        = cMediatorParams.getParameter ("Solution File Precision", 14);
    if (_nSolutionFilePrecision < 0)
    {
        cerr << "WARNING: Illegal 'Solution File Precision' value"
             << " in 'Mediator' sublist" << endl;
        cerr << "         Changing 'Solution File Precision' to zero" << endl;
        _nSolutionFilePrecision = 0;
    }

    _bMediatorHalting = false;

    return;
}


//----------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------
Mediator::~Mediator (void)
{
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        delete pNext->pWaitListWs;
        delete pNext->pCtzn;
        delete pNext;
    }
    _cCitizenList.erase (_cCitizenList.begin(), _cCitizenList.end());

    delete _pConveyor;

    if (_pBestPoint != NULL)
        delete _pBestPoint;

    if (Print::doPrint (Print::MOST_VERBOSE))
        DataPoint::debugPrintMemoryLists();

    if (_pTimers != NULL)
        delete _pTimers;

    return;
}


//----------------------------------------------------------------------
//  Method addCitizen
//----------------------------------------------------------------------
bool  Mediator::addCitizen (      Citizen * const  pCitizen,
                            const bool             bIsChild,
                            const int              nParentID)
{
    if (Print::doPrint (Print::MOST_VERBOSE))
        cout << "  Mediator adding citizen " << pCitizen->getIdNumber()
             << ": '" << pCitizen->getName() << "'" << endl;

    //---- CITIZEN NAMES MUST BE UNIQUE (SHOULD INCLUDE THE ID).
    const string & name = pCitizen->getName();
    if (_cPtOwnerMap.find (name) != _cPtOwnerMap.end())
    {
        cerr << "ERROR: Citizen named '" << name << "' already exists"
             << " <Mediator::addCitizen>" << endl;
        delete pCitizen;
        return( false );
    }

    CtznInfoBlockType *  pNewCtznBlock = new CtznInfoBlockType;
    pNewCtznBlock->pCtzn = pCitizen;
    pNewCtznBlock->bIsCtznFatalError = false;
    pNewCtznBlock->bIsCtznChild      = bIsChild;
    pNewCtznBlock->nParentID         = nParentID;
    pNewCtznBlock->nDeleteTag        = 0;

    //---- ADD AN EMPTY WAITING LIST FOR THE CITIZEN.
    ConveyorList *  pW = new ConveyorList();
    pW->setPriority (pCitizen->getPriority());
    pNewCtznBlock->pWaitListWs = pW;

    _cCitizenList.push_back (pNewCtznBlock);

    list< int >  cEmptyPointList;
    _cPtOwnerMap[name] = cEmptyPointList;
    _cNumEvals[name] = 0;

    return( true );
}


//----------------------------------------------------------------------
//  Method mediate
//----------------------------------------------------------------------
void  Mediator::mediate (void)
{
    _pTimers->start (nTOTALTIME);

    preProcess_();

    //---- INITIALIZE AN EMPTY RETURN LIST OF EVALUATED POINTS.
    ConveyorList  cR;

    //---- MAIN MEDIATOR EXECUTION LOOP.
    int  nNumConsecutiveZeroes = 0;
    int  loopNum = 0;
    while (isTownActive_() == true)
    {
        if (Print::doPrint (Print::UNEVALUATED_POINTS))
        {
            cout << "*** Top of Mediator, iteration " << loopNum
                 << ", num evals executed so far = "
                 << _pConveyor->getEvalCounter().getNumEvaluations() << endl;
            if (Print::doPrint (Print::MOST_VERBOSE))
                printDebugCitizenInfo_();
        }
        loopNum++;

        //---- CITIZENS EXAMINE cR AND ADD TO THEIR WAIT LIST.
        //---- IF A CHILD CITIZEN IS ADDED DURING THE EXCHANGE, THEN THE
        //---- FLAG WILL BE SET.
        _bChildAddedRecently = false;
        int  nNumCtznPoints = citizenExchange_ (cR);

        //---- DELETE THE RETURN LIST.
        eraseExchangeList_ (cR);

        //---- CLEAN UP ANY CHILD CITIZENS THAT ARE FINISHED.
        endCitizens_ (true);

        if (Print::doPrint (Print::UNEVALUATED_POINTS))
        {
            CtznInfoBlockListType::const_iterator  it;
            for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
            {
                ConveyorList *  pNext = (*it)->pWaitListWs;
                pNext->print ("Wait list from " + (*it)->pCtzn->getName()
                              + " (highest at bottom)");
            }
        }

        //---- WARN IF THERE ARE NO POINT REQUESTS FROM CITIZENS.
        //---- THIS COULD BE LEGITIMATE IF ALL CITIZENS ARE BUSY DOING
        //---- ASYNCHRONOUS PROCESSING ON THEIR OWN WORKERS.
        if (nNumCtznPoints == 0)
        {
            nNumConsecutiveZeroes++;
            if (nNumConsecutiveZeroes >= 10)
            {
                cerr << "WARNING: No trial points submitted to Mediator"
                     << " for the last 10 iterations" << endl;
                nNumConsecutiveZeroes = 0;
            }
        }
        else
            nNumConsecutiveZeroes = 0;

        //---- EXCHANGE POINTS WITH THE CONVEYOR.
        vector< ConveyorList * >  cTmpWs;
        CtznInfoBlockListType::const_iterator  it;
        for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
            cTmpWs.push_back ((*it)->pWaitListWs);
        _pConveyor->exchange (cTmpWs, cR);

        //---- CHECK NUMBER OF EVALUATIONS, CURRENT BEST POINT.
        if (makeStopTest_ (nNumCtznPoints, _bChildAddedRecently, cR) == true)
            break;
    }

    //---- LOOP IS DONE.  LET CITIZENS SEE THE EVALUATION LIST FOR A FINAL
    //---- TIME, BUT DO NOT ADD NEW POINTS, AND DO NOT ALLOW NEW CHILD
    //---- CITIZENS TO BE CREATED.
    if (Print::doPrint (Print::UNEVALUATED_POINTS))
    {
        cout << "*** Final Mediator iteration " << loopNum
             << ", evals executed: "
             << _pConveyor->getEvalCounter().getNumEvaluations() << endl;
    }
    pruneAllPoints_();
    _bMediatorHalting = true;
    setEarlyExitOnCitizens_();
    citizenExchange_ (cR);
    pruneAllPoints_();
    eraseExchangeList_ (cR);

    endCitizens_ (false);

    postProcess_();

    return;
}


//----------------------------------------------------------------------
//  Method getBestX
//----------------------------------------------------------------------
bool  Mediator::getBestX (vector< double > &  cBestX) const
{
    cBestX.clear();
    if (_pBestPoint == NULL)
        return( false );

    const Vector &  cX = _pBestPoint->getX();
    for (int  i = 0; i < cX.size(); i++)
        cBestX.push_back (cX[i]);
    return( true );
}


//----------------------------------------------------------------------
//  Method isBestFeasible
//----------------------------------------------------------------------
bool  Mediator::isBestFeasible (void) const
{
    if (_pBestPoint == NULL)
        return( false );

    return( isCompletelyFeasible_ (_pBestPoint) );
}


//----------------------------------------------------------------------
//  Method getBestF
//----------------------------------------------------------------------
double  Mediator::getBestF (void) const
{
    if (_pBestPoint == NULL)
        return( HOPSPACK::dne() );

    return( _pBestPoint->getBestF() );
}


//----------------------------------------------------------------------
//  Method getBestVecF
//----------------------------------------------------------------------
void  Mediator::getBestVecF (vector< double > &  cBestF) const
{
    cBestF.clear();
    if (_pBestPoint == NULL)
        return;

    const Vector &  cF = _pBestPoint->getVecF();
    for (int  i = 0; i < cF.size(); i++)
        cBestF.push_back (cF[i]);
    return;
}


//----------------------------------------------------------------------
//  Method getBestNonlEqs
//----------------------------------------------------------------------
void  Mediator::getBestNonlEqs (vector< double > &  cBestEqs) const
{
    cBestEqs.clear();
    if (_pBestPoint == NULL)
        return;

    const Vector &  cEqs = _pBestPoint->getEqs();
    for (int  i = 0; i < cEqs.size(); i++)
        cBestEqs.push_back (cEqs[i]);
    return;
}


//----------------------------------------------------------------------
//  Method getBestNonlIneqs
//----------------------------------------------------------------------
void  Mediator::getBestNonlIneqs (vector< double > &  cBestIneqs) const
{
    cBestIneqs.clear();
    if (_pBestPoint == NULL)
        return;

    const Vector &  cIneqs = _pBestPoint->getIneqs();
    for (int  i = 0; i < cIneqs.size(); i++)
        cBestIneqs.push_back (cIneqs[i]);
    return;
}


//----------------------------------------------------------------------
//  Method printDebugInfo
//----------------------------------------------------------------------
void  Mediator::printDebugInfo (void) const
{
    cout << "  Maximum Evaluations = " << _nMaxEvaluations;
    if (_nMaxEvaluations == -1)
        cout << "  (no limit)";
    cout << endl;
    cout << "  Solution File:           " << _sSolutionFileName << endl;
    cout << "  Solution File Precision: " << _nSolutionFilePrecision << endl;

    _pConveyor->printDebugInfo();
    ScaledComparison::printDebugInfo();
    printDebugCitizenInfo_();

    return;
}


//----------------------------------------------------------------------
//  Method reserveUniqueCitizenID implements CallbackToMediator
//----------------------------------------------------------------------
int  Mediator::reserveUniqueCitizenID (void)
{
    _nNextUniqueCitizenID++;
    return( _nNextUniqueCitizenID );
}


//----------------------------------------------------------------------
//  Method addChildCitizen implements CallbackToMediator
//----------------------------------------------------------------------
bool  Mediator::addChildCitizen (      Citizen * const  pCitizen,
                                 const int              nParentId)
{
    if (_bMediatorHalting)
    {
        cerr << "WARNING: Child citizen cannot be created because Mediator is halting" << endl;
        cerr << "         Ignoring request from parent citizen " << nParentId
             << endl;
        return( false );
    }

    bool  bResult = addCitizen (pCitizen, true, nParentId);
    _bChildAddedRecently = true;

    if (bResult)
    {
        if (Print::doPrint (Print::UNEVALUATED_POINTS))
        {
            cout << "==================================================" << endl;
            cout << "<Mediator, running preprocess commands on '"
                 << pCitizen->getName() << "'>" << endl;
            cout << "--------------------------------------------------" << endl;
        }

        pCitizen->preProcess();

        if (Print::doPrint (Print::UNEVALUATED_POINTS))
        {
            cout << "--------------------------------------------------" << endl;
            cout << "<Mediator, child citizen preprocessing complete>" << endl;
            cout << "==================================================" << endl;
        }
    }

    return( bResult );
}


//----------------------------------------------------------------------
//  Private Method preProcess_
//----------------------------------------------------------------------
void  Mediator::preProcess_ (void)
{
    if (Print::doPrint (Print::UNEVALUATED_POINTS))
    {
        cout << "==================================================" << endl;
        cout << "<Mediator, begin citizen preprocessing>" << endl;
        cout << "--------------------------------------------------" << endl;
    }

    _pTimers->start (nPREPROCESSING);

    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        if (   (pNext->bIsCtznFatalError == false)
            && (pNext->bIsCtznChild == false) )
        {
            if (Print::doPrint (Print::UNEVALUATED_POINTS))
            {
                cout << "* Running preprocess commands on '"
                     << pNext->pCtzn->getName() << "'" << endl;
                cout << "--------------------------------------------------"
                     << endl;
            }

            pNext->pCtzn->preProcess();

            if (Print::doPrint (Print::UNEVALUATED_POINTS))
                cout << "--------------------------------------------------"
                     << endl;
        }
    }

    _pTimers->stop (nPREPROCESSING);

    if (Print::doPrint (Print::UNEVALUATED_POINTS))
    {
        cout << "--------------------------------------------------" << endl;
        cout << "<Mediator, citizen preprocessing complete>" << endl;
        cout << "==================================================" << endl;
    }

    return;
}


//----------------------------------------------------------------------
//  Private Method postProcess_
//----------------------------------------------------------------------
void  Mediator::postProcess_ (void)
{
    if (Print::doPrint (Print::UNEVALUATED_POINTS))
    {
        cout << "==================================================" << endl;
        cout << "<Mediator, begin citizen postprocessing>" << endl;
        cout << "--------------------------------------------------" << endl;
    }

    _pTimers->start (nPOSTPROCESSING);

    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        if (pNext->bIsCtznFatalError == false)
        {
            if (Print::doPrint (Print::UNEVALUATED_POINTS))
            {
                cout << "* Running postprocess commands on '"
                     << pNext->pCtzn->getName() << "'" << endl;
                cout << "--------------------------------------------------"
                     << endl;
            }

            pNext->pCtzn->postProcess();

            if (Print::doPrint (Print::UNEVALUATED_POINTS))
                cout << "--------------------------------------------------"
                     << endl;
        }
    }

    _pTimers->stop (nPOSTPROCESSING);
 
    if (Print::doPrint (Print::UNEVALUATED_POINTS))
    {
        cout << "--------------------------------------------------" << endl;
        cout << "<Mediator, citizen postprocessing complete>" << endl;
        cout << "==================================================" << endl;
        cout << endl;
        cout << "* Mediator exiting, all points processed" << endl << endl;
    }

    _pTimers->stop (nTOTALTIME);

    //---- PRINT THE FINAL SOLUTION POINT.
    if (Print::doPrint (Print::FINAL_SOLUTION))
    {
        cout << endl;
        cout << "Mediator best point found:";
        if ((_pBestPoint != NULL) && !isCompletelyFeasible_ (_pBestPoint))
            cout << " (infeasible)";
        cout << endl;
        if (_pBestPoint == NULL)
            cout << "  none" << endl;
        else
        {
            _pBestPoint->leftshift (cout, true, true);
            cout << endl;
            cout.setf (ios::scientific);
            cout << "  Linear constr infeasibility: "
                 << "  |c(x)|_2 = "
                 << _cLinConstr.getL2Norm (_pBestPoint->getX())
                 << "  |c(x)|_inf = "
                 << _cLinConstr.getLInfNorm (_pBestPoint->getX())
                 << endl;
            cout.unsetf (ios::scientific);

            if (_cProbDef.hasNonlinearConstr())
            {
                cout.setf (ios::scientific);
                cout << "  Nonlinear constr infeas:     "
                     << "  |c(x)|_2 = "
                     << _pBestPoint->getNonlConstrL2Norm()
                     << "  |c(x)|_inf = "
                     << _pBestPoint->getNonlConstrLInfNorm()
                     << endl;
                cout.unsetf (ios::scientific);
            }

            if (   (_cProbDef.getObjType() != ProblemDef::FIND_FEASIBLE_PT)
                && (_pBestPoint->getBestF() == dne()) )
            {
                cout << "Note: best point has no objective,"
                     << " check for Evaluator errors." << endl;
            }
        }

        #if defined(HAVE_REALTIME_CLOCK)
            int  nWidth = 8;
            int  nCurrPrec  = cout.precision (3);
            cout.setf (ios::fixed | ios::right);
            cout << "Total wall clock time since start: "
                 << setw (nWidth) << _pTimers->getTotalTime (nTOTALTIME)
                 << " secs" << endl;
            cout.precision (nCurrPrec);
        #endif

        CtznInfoBlockListType::const_iterator  it;
        for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
        {
            CtznInfoBlockType *  pNext = (*it);
            if (pNext->bIsCtznFatalError)
            {
                cout << "Note: '" << pNext->pCtzn->getName()
                     << "' threw an exception and was halted." << endl;
            }
        }
    }
    writePointToSolutionFile_ (*_pBestPoint);

    if (Print::doPrint (Print::INPUT_PARAMETERS))
        printEvalTimeStats_();


    if (Print::doPrint (Print::FINAL_SOLUTION))
    {
        cout << endl
             << "------------------------------------------------------" << endl
             << "Thank you!" << endl
             << "HOPSPACK: Hybrid Optimization Parallel Pattern Search" << endl
             << "  TG Kolda, TD Plantenga, et al., Sandia National Labs" << endl
             << "For more information visit" << endl
             << "  https://software.sandia.gov/trac/hopspack" << endl
             << "------------------------------------------------------" << endl
             << endl;
    }

    return;
}


//----------------------------------------------------------------------
//  Private Method isTownActive_
//----------------------------------------------------------------------
bool  Mediator::isTownActive_ (void) const
{
    Citizen::State  nState;
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        if (pNext->bIsCtznFatalError == false)
        {
            nState = pNext->pCtzn->getState();

            //---- STOP IF ANY CITIZEN INSISTS.
            if (nState == Citizen::MUST_STOP)
                return( false );

            //---- CONTINUE IF ANY CITIZEN IS STILL ACTIVE, OR A CHILD IS
            //---- WAITING FOR postProcess() SO IT CAN RETURN TO ITS PARENT.
            if (   (nState == Citizen::CONTINUE)
                || (nState == Citizen::CHILD_WAITING) )
                return( true );
        }
    }

    return( false );
}


//----------------------------------------------------------------------
//  Private Method citizenExchange_
//----------------------------------------------------------------------
int  Mediator::citizenExchange_ (const ConveyorList &  cR)
{
    bool  bTraceDebug = Print::doPrint (Print::UNEVALUATED_POINTS);

    if (bTraceDebug)
    {
        cout << "  ================================================" << endl;
        cout << "  <Exchanging points with citizens>" << endl;
    }

    _pTimers->start (nCTZNPROCESSING);

    int  nNumCtznPoints = 0;
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        if (pNext->bIsCtznFatalError)
            continue;

        Citizen::State  nState = pNext->pCtzn->getState();
        string          sName  = pNext->pCtzn->getName();

        if (bTraceDebug)
        {
            cout << "  Name: '" << sName << "' state = " << nState << endl;
            cout << "  ------------------------------------------------" << endl;
        }

        //---- CALL EXCHANGE IF THE CITIZEN IS STILL RUNNING.
        if ((nState == Citizen::CONTINUE) || (nState == Citizen::WAITING))
        {
            ConveyorList *  pW = pNext->pWaitListWs;
            list< DataPoint * > &  cNewPoints = pW->getMutablePointList();

            try
            {
                pNext->pCtzn->exchange (cR.getPointList(),
                                        _cPtOwnerMap[sName],
                                        cNewPoints);
            }
            catch (...)
            {
                //---- ANY UNHANDLED EXCEPTION CAUSES THE MEDIATOR TO
                //---- HENCEFORTH IGNORE THE CITIZEN AND ITS ANCESTORS.
                pNext->bIsCtznFatalError = true;
                cerr << "WARNING: Citizen '" << pNext->pCtzn->getName()
                     << "' threw an exception, caught by Mediator" << endl;
                cerr << "         Ignoring the citizen" << endl;

                CtznInfoBlockType *  pNextParent;
                while ((pNextParent = getParentInfo_ (pNext)) != NULL)
                {
                    pNextParent->bIsCtznFatalError = true;
                    cerr << "WARNING: Also ignoring parent citizen '"
                         << pNextParent->pCtzn->getName() << "'" << endl;
                    pNext = pNextParent;
                }
                continue;
            }

            list< DataPoint * >::iterator  it;
            for (it = cNewPoints.begin(); it != cNewPoints.end(); it++)
            {
                if ((*it)->getX().size() != _nNumVars)
                {
                    cerr << "WARNING: Citizen '" << pNext->pCtzn->getName()
                         << "' submitted a point of incorrect size" << endl;
                    cerr << "         Ignoring the point" << endl;
                    cNewPoints.erase (it);
                }
            }
            nNumCtznPoints += cNewPoints.size();

            //---- UPDATE THE MAPPING OF CITIZENS TO THEIR POINTS.
            vector< int >  cTags;
            pW->getTagList (cTags);
            _cPtOwnerMap[sName].insert (_cPtOwnerMap[sName].end(),
                                        cTags.begin(), cTags.end());

            //---- UPDATE THE CITIZEN'S PRIORITY.
            pW->setPriority (pNext->pCtzn->getPriority());
        }

        if (bTraceDebug)
            cout << "  ------------------------------------------------" << endl;
    }

    _pTimers->stop (nCTZNPROCESSING);

    if (bTraceDebug)
    {
        cout << "  <Exchange of points complete>" << endl;
        cout << "  ================================================" << endl;
    }

    return( nNumCtznPoints );
}


//----------------------------------------------------------------------
//  Private Method eraseExchangeList_
//----------------------------------------------------------------------
void  Mediator::eraseExchangeList_ (ConveyorList &  cList)
{
    vector< int >  cTags;
    cList.getTagList (cTags);


    //---- ERASE TAGS FROM THE OWNER MAP.
    map< string, list< int > >::iterator  itMap;
    for (itMap = _cPtOwnerMap.begin(); itMap != _cPtOwnerMap.end(); itMap++)
    {
        list< int > &  nextCtznTagList = itMap->second;

        //---- LOOP THRU THE LIST AND SEARCH FOR TAGS OWNED BY THE CITIZEN.
        list< DataPoint * >::const_iterator  itPt;
        for (itPt = cList.begin(); itPt != cList.end(); itPt++)
        {
            list< int >::iterator  itFind = find (nextCtznTagList.begin(),
                                                  nextCtznTagList.end(),
                                                  (*itPt)->getTag());
            if (itFind != nextCtznTagList.end())
            {
                //---- FOUND ONE.  AS A SIDE EFFECT, UPDATE THE COUNT OF
                //---- POINTS EVALUATED FOR THE CITIZEN.
                nextCtznTagList.erase (itFind);
                _cNumEvals[itMap->first]++;
            }
        }
    }

    cList.prune();

    return;
}


//----------------------------------------------------------------------
//  Private Method getParentInfo_
//----------------------------------------------------------------------
Mediator::CtznInfoBlockType *  Mediator::getParentInfo_
                                   (const CtznInfoBlockType *  pCtznInfo)
{
    if (pCtznInfo->bIsCtznChild == false)
        return( NULL );

    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        if (pCtznInfo->nParentID == (*it)->pCtzn->getIdNumber())
        {
            if (it != _cCitizenList.end())
                return( *it );
        }
    }

    return( NULL );
}


//----------------------------------------------------------------------
//  Private Method setEarlyExitOnCitizens_
//----------------------------------------------------------------------
void  Mediator::setEarlyExitOnCitizens_ (void)
{
    CtznInfoBlockListType::iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        pNext->pCtzn->setEarlyExit();
    }
    return;
}


//----------------------------------------------------------------------
//  Private Method endCitizens_
//----------------------------------------------------------------------
void  Mediator::endCitizens_ (const bool  bOnlyIfFinished)
{
    while (true)
    {
        //---- START WITH A NEW ITERATOR BECAUSE endMarkedCitizens_
        //---- WILL MODIFY _cCitizenList.
        CtznInfoBlockListType::iterator  it;
        for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
        {
            CtznInfoBlockType *  pNext = (*it);
            if (pNext->bIsCtznFatalError)
                continue;

            //---- ORIGINAL CITIZENS ARE NOT ENDED UNTIL THE MEDIATOR IS
            //---- DONE LOOPING.
            if (bOnlyIfFinished && (pNext->bIsCtznChild == false))
                continue;

            if (bOnlyIfFinished)
            {
                if (   (pNext->pCtzn->getState() != Citizen::CHILD_FINISHED)
                    && (pNext->pCtzn->getState() != Citizen::CHILD_WAITING) )
                {
                    //---- DO NOT END THE CHILD CITIZEN.
                    continue;
                }
            }

            //---- MARK pNext AND ANY CHILDREN IT HAS.
            markCitizensRecursively_ (pNext, nMARKED_FOR_DELETION);
            break;
        }
        if (it == _cCitizenList.end())
            return;

        //---- NOW END THE MARKED CITIZENS, AS ORDERED BY THE MARKING TAGS.
        endMarkedCitizens_();
    }
}


//----------------------------------------------------------------------
//  Private Method markCitizensRecursively_
//----------------------------------------------------------------------
void  Mediator::markCitizensRecursively_ (      CtznInfoBlockType *  pCtznInfo,
                                          const int                  nDeleteTag)
{
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        if (pCtznInfo->pCtzn->getIdNumber() == (*it)->nParentID)
        {
            if (it != _cCitizenList.end())
            {
                //---- THE CITIZEN HAS CHILDREN, SO CALL RECURSIVELY TO END THEM.
                markCitizensRecursively_ ((*it), nDeleteTag - 1);
            }
        }
    }

    pCtznInfo->nDeleteTag = nDeleteTag;

    return;
}


//----------------------------------------------------------------------
//  Private Method endMarkedCitizens_
//----------------------------------------------------------------------
void  Mediator::endMarkedCitizens_ (void)
{
    //---- DELETE IN ORDER, FROM LOWEST MARK UP TO nMARKED_FOR_DELETION.

    CtznInfoBlockListType::const_iterator  cit;
    int  nLowestMark = nMARKED_FOR_DELETION + 1;
    for (cit = _cCitizenList.begin(); cit != _cCitizenList.end(); cit++)
        if ((*cit)->nDeleteTag < nLowestMark)
            nLowestMark = (*cit)->nDeleteTag;

    for (int  i = nLowestMark; i <= nMARKED_FOR_DELETION; i++)
    {
        CtznInfoBlockListType::iterator  it = _cCitizenList.begin();
        while (it != _cCitizenList.end())
        {
            CtznInfoBlockType *  pNextInfo = (*it);

            if (pNextInfo->nDeleteTag == i)
            {
                if (Print::doPrint (Print::UNEVALUATED_POINTS))
                {
                    cout << "=================================================="
                         << endl;
                    cout << "<Mediator, running postprocess commands on '"
                         << pNextInfo->pCtzn->getName() << "'>" << endl;
                    cout << "--------------------------------------------------"
                         << endl;
                }

                pNextInfo->pCtzn->postProcess();

                if (Print::doPrint (Print::UNEVALUATED_POINTS))
                {
                    cout << "--------------------------------------------------"
                         << endl;
                    cout << "<Mediator, citizen postprocessing complete>"
                         << endl;
                    cout << "=================================================="
                         << endl;
                }

                //---- ALL CHILDREN ARE ENDED, SO IT IS SAFE TO END THIS CITIZEN.
                delete pNextInfo->pWaitListWs;
                delete pNextInfo->pCtzn;
                delete pNextInfo;
                it = _cCitizenList.erase (it);
            }
            else
            {
                it++;
            }
        }
    }

    return;
}


//----------------------------------------------------------------------
//  Private Method pruneAllPoints_
//----------------------------------------------------------------------
void  Mediator::pruneAllPoints_ (void) const
{
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
        (*it)->pWaitListWs->prune();
    return;
}


//----------------------------------------------------------------------
//  Private Method writePointToSolutionFile_
//----------------------------------------------------------------------
void  Mediator::writePointToSolutionFile_ (const DataPoint &  cPoint) const
{
    if (_sSolutionFileName.empty())
        return;

    ofstream  fp;

    //---- APPEND TO ANY EXISTING FILE.
    fp.open (_sSolutionFileName.c_str(), ios::app);
    if (!fp)
    {
        cerr << "WARNING: Could not open solution file '"
             << _sSolutionFileName << "'" << endl;
        return;
    }

    //---- THIS OUTPUT FORMAT IS IDENTICAL TO THAT OF THE CacheMananger.
    fp << "f=[ ";
    (cPoint.getVecF()).leftshift (fp, _nSolutionFilePrecision);
    fp << " ]";

    fp << " x=[ ";
    (cPoint.getX()).leftshift (fp, _nSolutionFilePrecision);
    fp << " ]" << endl;

    return;
}


//----------------------------------------------------------------------
//  Private Method makeStopTest_
//----------------------------------------------------------------------
bool  Mediator::makeStopTest_ (const int             nNumNewCtznPoints,
                               const bool            bChildAdded,
                               const ConveyorList &  cList)
{
    updateBestPoint_ (cList);

    //---- STOP IF OBJECTIVE GOAL IS REACHED.
    if (_pBestPoint != NULL)
    {
        double  dPercentAchieved;
        if (   isCompletelyFeasible_ (_pBestPoint)
            && (_cProbDef.isObjTargetReached (_pBestPoint->getBestF(),
                                              dPercentAchieved)) )
        {
            if (Print::doPrint (Print::FINAL_SOLUTION))
            {
                cout << endl;
                if (dPercentAchieved == 0.0)
                {
                    cout << "Mediator stopping - objective target reached "
                         << "(target = " << _cProbDef.getObjTarget() << ")"
                         << endl;
                }
                else
                {
                    cout << "Mediator stopping - within "
                         << dPercentAchieved << "% of objective target "
                         << "(threshold = "
                         << _cProbDef.getObjPercentErrorThreshold() << "%)"
                         << endl;
                }
                cout << endl;
            }
            return( true );
        }
    }

    //---- STOP IF MAX EVALUATIONS REACHED.
    if (_nMaxEvaluations != -1)
    {
        int  nNumEvals = _pConveyor->getEvalCounter().getNumEvaluations();
        if (nNumEvals >= _nMaxEvaluations)
        {
            if (Print::doPrint (Print::FINAL_SOLUTION))
            {
                cout << endl;
                cout << "Mediator stopping - used the max number of evaluations "
                     << "(max = " << _nMaxEvaluations << ")" << endl;
                cout << endl;
            }
            return( true );
        }
    }

    //---- STOP IF CONVEYOR IS COMPLETELY IDLE.
    //---- A CHILD CITIZEN MAY STOP, CAUSING ITS PARENT TO ADD A NEW CHILD,
    //---- BUT NO NEW CITIZEN POINTS ARE INITIALLY AVAILABLE; THEREFORE,
    //---- KEEP GOING IF bChildAdded IS TRUE.
    if (   (isTownActive_() == false)
        && (nNumNewCtznPoints == 0)
        && (bChildAdded == false)
        && (cList.size() == 0)
        && (_pConveyor->getNumPending() == 0))
    {
        if (Print::doPrint (Print::FINAL_SOLUTION))
        {
            cout << endl;
            cout << "Mediator stopping - no more citizen points to evaluate"
                 << endl;
            cout << endl;
        }
        return( true );
    }

    return( false );
}


//----------------------------------------------------------------------
//  Private Method updateBestPoint_
//----------------------------------------------------------------------
void  Mediator::updateBestPoint_ (const ConveyorList &  cList)
{
    bool  bIsBestFeasible = false;
    if (_pBestPoint != NULL)
        bIsBestFeasible = isCompletelyFeasible_ (_pBestPoint);

    //---- LOOP THRU THE NEWLY EVALUATED POINTS AND UPDATE "BEST POINT".
    //---- PRIORITY IS GIVEN TO FEASIBILITY (BOTH LINEAR AND NONLINEAR
    //---- CONSTRAINTS) BEFORE COMPARING OBJECTIVES.
    list< DataPoint * >::const_iterator  it;
    for (it = cList.begin(); it != cList.end(); it++)
    {
        if (_pBestPoint == NULL)
        {
            //---- THIS MUST BE THE VERY FIRST POINT EVALUATED.
            _pBestPoint = new DataPoint (**it);
            bIsBestFeasible = isCompletelyFeasible_ (_pBestPoint);
            continue;
        }

        bool  bIsNextFeasible = isCompletelyFeasible_ ((*it));

        if (bIsNextFeasible && !bIsBestFeasible)
        {
            //---- THIS POINT IS FEASIBLE, SO IT WINS.
            delete _pBestPoint;
            _pBestPoint = new DataPoint (**it);
            bIsBestFeasible = true;
        }
        else if (bIsNextFeasible && bIsBestFeasible)
        {
            //---- BOTH POINTS ARE FEASIBLE, SO COMPARE OBJECTIVES.
            bool  bDummy;
            if ((*it)->isBetterObjThan (*_pBestPoint, bDummy))
            {
                delete _pBestPoint;
                _pBestPoint = new DataPoint (**it);
            }
        }
        else if (!bIsNextFeasible && !bIsBestFeasible)
        {
            //---- NEITHER POINT IS FEASIBLE, SO CHOOSE THE LEAST INFEASIBLE.
            bool  bIsBestLinFeas
                = (   _cProbDef.isBndsFeasible (_pBestPoint->getX())
                   && _cLinConstr.isFeasible (_pBestPoint->getX()));
            bool  bIsNextLinFeas = (   _cProbDef.isBndsFeasible ((*it)->getX())
                                    && _cLinConstr.isFeasible ((*it)->getX()));
            if (bIsNextLinFeas && !bIsBestLinFeas)
            {
                delete _pBestPoint;
                _pBestPoint = new DataPoint (**it);
                bIsBestLinFeas = true;
            }
            else if (!bIsNextLinFeas && !bIsBestLinFeas)
            {
                if (_cLinConstr.getLInfNorm ((*it)->getX()) <
                        _cLinConstr.getLInfNorm (_pBestPoint->getX()))
                {
                    delete _pBestPoint;
                    _pBestPoint = new DataPoint (**it);
                }
            }
            else if (bIsNextLinFeas && bIsBestLinFeas)
            {
                if ((*it)->getNonlConstrLInfNorm() <
                        _pBestPoint->getNonlConstrLInfNorm())
                {
                    delete _pBestPoint;
                    _pBestPoint = new DataPoint (**it);
                }
            }
        }
    }

    return;
}


//----------------------------------------------------------------------
//  Private Method isCompletelyFeasible_
//----------------------------------------------------------------------
bool  Mediator::isCompletelyFeasible_ (const DataPoint * const  pPoint) const
{
    if (pPoint == NULL)
        return( false );

    bool  bResult = (   _cProbDef.isBndsFeasible (pPoint->getX())
                     && _cLinConstr.isFeasible (pPoint->getX())
                     && _cProbDef.isNonlinearlyFeasible (pPoint->getEqs(),
                                                         pPoint->getIneqs()) );
    return( bResult );
}


//----------------------------------------------------------------------
//  Private Method printEvalTimeStats_
//----------------------------------------------------------------------
void  Mediator::printEvalTimeStats_ (void) const
{
    //---- PRINT EVALUATION STATISTICS.
    if (Print::doPrint (Print::EVALUATED_POINTS))
    {
        cout << endl;
        _pConveyor->getEvalCounter().print (true);
    }
    else if (Print::doPrint (Print::FINAL_SOLUTION))
    {
        cout << endl;
        _pConveyor->getEvalCounter().print (false);
    }

    //---- PRINT TIMING REPORTS.
    #if defined(HAVE_REALTIME_CLOCK)
        if (Print::doPrint (Print::INPUT_PARAMETERS))
        {
            //---- FORMAT WILL BE "XXXX.XXX".
            int  nWidth = 8;
            int  nCurrPrec = cout.precision (3);
            cout.setf (ios::fixed | ios::right);
            cout << "Total wall clock time in Mediator: "
                 << _pTimers->getTotalTime (nTOTALTIME) << " secs" << endl;
            cout << "  Citizen preprocessing   "
                 << setw (nWidth) << _pTimers->getTotalTime (nPREPROCESSING)
                 << endl;
            cout << "  Citizen processing      "
                 << setw (nWidth) << _pTimers->getTotalTime (nCTZNPROCESSING)
                 << endl;
            cout << "  Citizen postprocessing  "
                 << setw (nWidth) << _pTimers->getTotalTime (nPOSTPROCESSING)
                 << endl;
            cout.precision (nCurrPrec);
            _pExecutor->printTimingInfo();
        }
    #endif

    return;
}


//----------------------------------------------------------------------
//  Private Method printDebugCitizenInfo_
//----------------------------------------------------------------------
void  Mediator::printDebugCitizenInfo_ (void) const
{
    CtznInfoBlockListType::const_iterator  it;
    for (it = _cCitizenList.begin(); it != _cCitizenList.end(); it++)
    {
        CtznInfoBlockType *  pNext = (*it);
        cout << "  Mediator citizen " << pNext->pCtzn->getIdNumber()
             << ": " << pNext->pCtzn->getName();

        if (pNext->bIsCtznFatalError)
            cout << ", fatal error";
        else
            cout << ", alive";

        if (pNext->bIsCtznChild)
        {
            cout << ", child (parent=" << pNext->nParentID <<")";
        }
        else
            cout << ", not child";
        cout << ", DelTag=" << pNext->nDeleteTag;

        cout << endl;
    }

    return;
}


}     //-- namespace HOPSPACK
