// $Id: HOPSPACK_Hopspack.cpp 182 2010-12-10 22:56:11Z briadam $
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-main/HOPSPACK_Hopspack.cpp $

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
  @file HOPSPACK_Hopspack.cpp
  @brief Implement HOPSPACK::Hopspack.
*/

#include <sstream>

#if defined(_WIN32)
  #include <windows.h>
#endif

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Citizen.hpp"
#include "HOPSPACK_DataPoint.hpp"
#include "HOPSPACK_Executor.hpp"
#include "HOPSPACK_float.hpp"
#include "HOPSPACK_Hopspack.hpp"
#include "HOPSPACK_LinConstr.hpp"
#include "HOPSPACK_Mediator.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Print.hpp"
#include "HOPSPACK_ProblemDef.hpp"
#include "HOPSPACK_utils.hpp"
#include "HOPSPACK_Vector.hpp"

#include "mango.hpp"

namespace HOPSPACK
{


//----------------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------------
  Hopspack::Hopspack (Executor * const  pExecutor)
    :
    _pExecutor (pExecutor),
    _bAreParametersSet (false),
    _pProbDef (NULL),
    _pLinConstr (NULL),
    _pInitialPoint (NULL),
    _pMediator (NULL),
    _bHaveBestPoint (false)
{
    #if defined(HAVE_REALTIME_CLOCK)
        //---- NEED TO SET UP THE WINDOWS REAL TIME RESOLUTION
        //---- FOR VARIOUS UTILITIES.
        #if defined(_WIN32)
            //---- MSDN SAYS TO SET THE RESOLUTION OF timeGetTime ONCE BEFORE
            //---- USING IT, AND REMEMBER TO UNSET WHEN FINISHED
            //---- (CALL timeBeginPeriod AND timeEndPeriod).
            //---- I FIND THAT IT TAKES timeBeginPeriod AT LEAST ONE SYSTEM TICK
            //---- (~16 MSEC) TO TAKE EFFECT; SLEEP TO SAFELY SET IT UP.
            _nWindowsSaveTimerMin = -1;
            TIMECAPS  tTimeCaps;
            if (timeGetDevCaps (&tTimeCaps,
                                sizeof (tTimeCaps)) != TIMERR_NOERROR)
            {
                _nWindowsSaveTimerMin = (int) tTimeCaps.wPeriodMin;
                timeBeginPeriod (_nWindowsSaveTimerMin);
                Sleep (35);     //-- 35 milliseconds
            }
            else
            {
                //---- SOMETIMES THE WIN32 API CALL FAILS FOR NO GOOD REASON.
                //---- TRY FOR THE USUAL 1 MSEC MINIMUM; OTHERWISE,
                //---- MULTITHREADING CAN BE SLOWED SIGNIFICANTLY.
                timeBeginPeriod (1);
                Sleep (35);     //-- 35 milliseconds
            }
        #endif
    #endif

    return;
}


//----------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------
Hopspack::~Hopspack (void)
{
    if (_pProbDef != NULL)
        delete _pProbDef;
    if (_pLinConstr != NULL)
        delete _pLinConstr;
    if (_pInitialPoint != NULL)
        delete _pInitialPoint;
    if (_pMediator != NULL)
        delete _pMediator;

    #if defined(HAVE_REALTIME_CLOCK)
        #if defined(_WIN32)
            //---- UNSET THE TIMER FOR WINDOWS.
            if (_nWindowsSaveTimerMin > 0)
                timeEndPeriod (_nWindowsSaveTimerMin);
        #endif
    #endif

    return;
}


//----------------------------------------------------------------------
//  Method setInputParameters
//----------------------------------------------------------------------
  bool  Hopspack::setInputParameters (const ParameterList &  cParams, mango::problem* mango_problem)
{
    if (_bAreParametersSet == true)
    {
        cerr << "ERROR: Cannot call Hopspack::setInputParameters twice" << endl;
        return( false );
    }

    if (checkParameterBasics_ (cParams) == false)
        return( false );

    //---- DEBUG LEVEL IS KNOWN AFTER CHECKING PARAMETERS.
    bool  bIsVerboseDebugging = Print::doPrint (Print::MOST_VERBOSE);


    if (bIsVerboseDebugging)
    {
        cout << "##################################################" << endl;
        cout << "###      Begin HOPSPACK Initialization         ###" << endl;
    }

    _pProbDef = new ProblemDef();
    if (_pProbDef->initialize (cParams.sublist ("Problem Definition")) == false)
        return( false );

    _pLinConstr = new LinConstr (*_pProbDef);
    bool  bIsOK = true;
    if (cParams.isParameterSublist ("Linear Constraints") == true)
        bIsOK = _pLinConstr->initialize (cParams.sublist ("Linear Constraints"));
    else
    {
        ParameterList  emptyList;
        bIsOK = _pLinConstr->initialize (emptyList);
    }
    if (bIsOK == false)
        return( false );

    _pInitialPoint = makeInitialPoint_ (*_pProbDef, *_pLinConstr);
    if (_pInitialPoint != NULL)
        _pProbDef->resetInitialX (_pInitialPoint->getX());

    //---- CONSTRUCT A MEDIATOR, WHICH ALSO CONSTRUCTS THE CONVEYOR.
    _pMediator = new Mediator (cParams.sublist ("Mediator"),
                               *_pProbDef,
                               *_pLinConstr,
                               _pInitialPoint,
                               _pExecutor,
			       mango_problem);

    if (makeCitizens_ (cParams, *_pProbDef, *_pLinConstr) == false)
        return( false );

    if (bIsVerboseDebugging)
    {
        cout << "###       End HOPSPACK Initialization          ###" << endl;
        cout << "##################################################" << endl;
        cout << endl;
    }

    _bAreParametersSet = true;
    return( true );
}


//----------------------------------------------------------------------
//  Method setInputParameters
//----------------------------------------------------------------------
bool  Hopspack::setInputParameters (const string &  sFileName)
{
    if (_bAreParametersSet == true)
    {
        cerr << "ERROR: Cannot call Hopspack::setInputParameters twice" << endl;
        return( false );
    }

    //---- THE PARSER IS IN HOPSPACK_utils.hpp.
    ParameterList  cParams;
    if (parseTextInputFile (sFileName, cParams) == false)
        return( false );

    return( setInputParameters (cParams, NULL) );
}


//----------------------------------------------------------------------
//  Method solve
//----------------------------------------------------------------------
FinalOptStateType  Hopspack::solve (void)
{
    if (_bAreParametersSet == false)
    {
        cerr << "ERROR: Must call Hopspack::setInputParameters first"
             << "  <Hopspack::optimize>" << endl;
        return( FINALSTATE_ERROR );
    }

    try
    {
        _pMediator->mediate();
    }
    catch (const char * const)
    {
        //---- ASSUME THE EXCEPTION ALREADY PRINTED AN ERROR MESSAGE.
        cerr << "HOPSPACK ENDING due to fatal error" << endl;
        return( FINALSTATE_ERROR );
    }

    //---- QUERY THE MEDIATOR FOR THE BEST SOLUTION POINT.
    if (_pMediator->getBestX (_cBestX) == false)
    {
        _bHaveBestPoint = false;
        return( FINALSTATE_ERROR );
    }
    _bHaveBestPoint = true;

    if (_pMediator->isBestFeasible())
        return( FINALSTATE_FEASIBLE );
    else
        return( FINALSTATE_INFEASIBLE );
}


//----------------------------------------------------------------------
//  Method getBestX
//----------------------------------------------------------------------
bool  Hopspack::getBestX (vector< double > &  cBestX) const
{
    cBestX.clear();
    if ((_bAreParametersSet == false) || (_bHaveBestPoint == false))
        return( false );

    for (int  i = 0; i < (int) _cBestX.size(); i++)
        cBestX.push_back (_cBestX[i]);
    return( true );
}


//----------------------------------------------------------------------
//  Method getBestF
//----------------------------------------------------------------------
double  Hopspack::getBestF (void) const
{
    if ((_bAreParametersSet == false) || (_bHaveBestPoint == false))
        return( HOPSPACK::dne() );

    return( _pMediator->getBestF() );
}


//----------------------------------------------------------------------
//  Method getBestVecF
//----------------------------------------------------------------------
void  Hopspack::getBestVecF (vector< double > &  cBestF) const
{
    cBestF.clear();
    if ((_bAreParametersSet == false) || (_bHaveBestPoint == false))
        return;

    _pMediator->getBestVecF (cBestF);
    return;
}


//----------------------------------------------------------------------
//  Method getBestNonlEqs
//----------------------------------------------------------------------
void  Hopspack::getBestNonlEqs (vector< double > &  cBestEqs) const
{
    cBestEqs.clear();
    if ((_bAreParametersSet == false) || (_bHaveBestPoint == false))
        return;

    _pMediator->getBestNonlEqs (cBestEqs);
    return;
}


//----------------------------------------------------------------------
//  Method getBestNonlIneqs
//----------------------------------------------------------------------
void  Hopspack::getBestNonlIneqs (vector< double > &  cBestIneqs) const
{
    cBestIneqs.clear();
    if ((_bAreParametersSet == false) || (_bHaveBestPoint == false))
        return;

    _pMediator->getBestNonlIneqs (cBestIneqs);
    return;
}


//----------------------------------------------------------------------
//  Private Method checkParameterBasics_
//----------------------------------------------------------------------
bool  Hopspack::checkParameterBasics_ (const ParameterList &  cParams) const
{
    //---- MUST HAVE A MEDIATOR SUBLIST.
    if (cParams.isParameterSublist ("Mediator") == false)
    {
        printParamError_ ("Cannot find 'Mediator' sublist");
        return( false );
    }

    //---- SET THE DISPLAY LEVEL FROM MEDIATOR PARAMETERS.
    //---- IF NONE, THEN A DEFAULT IS SET IN HOPSPACK::Print.
    if (cParams.sublist("Mediator").isParameterInt("Display") == true)
    {
        int  nTmp = cParams.sublist("Mediator").getParameter ("Display", -1);
        Print::setDisplayParameter ((Print::PrintType) nTmp);
    }
    if (cParams.sublist("Mediator").isParameterInt("Precision") == true)
    {
        int  nTmp = cParams.sublist("Mediator").getParameter ("Precision", -1);
        if (nTmp >= 0)
            Print::setPrecisionParameter (nTmp);
    }

    //---- DUMP THE PARAMETER LIST IF DEBUGGING.
    bool  bIsVerboseDebugging = Print::doPrint (Print::MOST_VERBOSE);
    if (bIsVerboseDebugging == true)
    {
        cout << "---------- Checking parsed input parameters ----------"
             << endl;
        cout << "<begin printing an alphabetized list of input parameters>"
             << endl;
        cParams.print (cout, 2);
        cout << "<end printing an alphabetized list of input parameters>"
             << endl;
    }

    //---- MUST HAVE A PROBLEM DEFINITION SUBLIST.
    if (cParams.isParameterSublist ("Problem Definition") == false)
    {
        printParamError_ ("Cannot find 'Problem Definition' sublist");
        return( false );
    }

    //---- CANNOT HAVE A LINEAR CONSTRAINT SUBLIST IF LAPACK IS NOT
    //---- PART OF THE BUILD.
    #if !defined(HAVE_LAPACK)
        if (cParams.isParameterSublist ("Linear Constraints") == true)
        {
            printParamError_ ("Cannot use 'Linear Constraints' sublist,"
                             " no LAPACK in build");
            return( false );
        }
    #endif

    //---- CITIZENS MUST BE NAMED CORRECTLY.
    if (cParams.sublist("Mediator").isParameter("Citizen Count") == false)
    {
        printParamError_ ("Need 'Citizen Count' parameter in 'Mediator' sublist");
        return( false );
    }
    int  nCtzns = cParams.sublist("Mediator").getParameter("Citizen Count", -1);
    if (nCtzns < 1)
    {
        printParamError_ ("'Citizen count' must be greater than 0.");
        return( false );
    }
    for (int  i = 1; i <= nCtzns; i++)
    {
        stringstream  tmp;
        tmp << "Citizen " << i;
        string sNextCtznName = tmp.str();
        if (cParams.isParameterSublist (sNextCtznName) == false)
        {
            printParamError_ ("Cannot find sublist named '"
                             + sNextCtznName + "'");
            return( false );
        }
    }
    
    if (bIsVerboseDebugging == true)
    {
        cout << "---------- Finished basic check of input parameters ----------"
             << endl << endl;
    }

    return( true );
}


//--------------------------------------------------------------------
//  Private Method printParamError_
//--------------------------------------------------------------------
void  Hopspack::printParamError_ (const string &  sMsg) const
{
    cerr << endl;
    cerr << "ERROR found in input parameter list:" << endl;
    cerr << " " << sMsg << endl;
    return;
}


//----------------------------------------------------------------------
//  Private Method makeInitialPoint_
//----------------------------------------------------------------------
DataPoint *  Hopspack::makeInitialPoint_ (      ProblemDef &  cProbDef,
                                          const LinConstr  &  cLinConstr) const
{
    Vector  initialX = cProbDef.getInitialX();
    if (initialX.empty())
        return( NULL );

    //---- ANY INITIAL POINT FROM THE PROBLEM DEFINITION IS ALWAYS
    //---- FEASIBLE WITH RESPECT TO VARIABLE BOUNDS.

    if (cLinConstr.isFeasible (initialX) == true)
    {
        //---- USER POINT IS FEASIBLE.
        DataPoint *  pResult = new DataPoint (cProbDef.getObjType(),
                                              initialX);
        //---- ATTACH EVALUATION DATA IF PRESENT.
        Vector  initialF = cProbDef.getInitialF();
        Vector  initialEqs = cProbDef.getInitialEqs();
        Vector  initialIneqs = cProbDef.getInitialIneqs();
        if (   (initialF.empty() == false)
            || (initialEqs.empty() == false)
            || (initialIneqs.empty() == false) )
        {
            pResult->setEvalFC (initialF,
                                initialEqs, initialIneqs,
                                "(User Initial Point)");
        }
        return( pResult );
    }
    else
    {
        cerr << "WARNING: The point 'Initial X' violates"
             << " a linear constraint" << endl;
        cerr << "         Modifying 'Initial X' to be feasible" << endl;
        //---- MODIFY THE INITIAL POINT TO MAKE IT FEASIBLE.
        if (cLinConstr.projectToFeasibility (initialX) == false)
        {
            cerr << "WARNING: Unable to make initial point feasible" << endl;
            cerr << "         Ignoring 'Initial X'" << endl;
            Vector  emptyX;
            cProbDef.resetInitialX (emptyX);
            return( NULL );
        }
        DataPoint *  pResult = new DataPoint (cProbDef.getObjType(),
                                              initialX);
        return( pResult );
    }
}


//----------------------------------------------------------------------
//  Private Method makeCitizens_
//----------------------------------------------------------------------
bool  Hopspack::makeCitizens_ (const ParameterList &  cParams,
                               const ProblemDef    &  cProbDef,
                               const LinConstr     &  cLinConstr)
{
    if (Print::doPrint (Print::MOST_VERBOSE))
        cout << endl << "<Begin construction of citizens>" << endl;

    //---- CONSTRUCT EACH CITIZEN BASED ON ITS PARAMETER SUBLIST.
    //---- ASSUME checkParameterBasics() HAS ALREADY BEEN CALLED TO VERIFY
    //---- THAT THE CITIZENS EXIST.
    int  nCtzns = cParams.sublist("Mediator").getParameter("Citizen Count", -1);
    int  nNumDefinedCtzns = 0;
    for (int  i = 1; i <= nCtzns; i++)
    {
        int  nNextID = _pMediator->reserveUniqueCitizenID();
        stringstream  tmp;
        tmp << "Citizen " << nNextID;
        string sNextCtznName = tmp.str();

        //---- IF THE CITIZEN COULD NOT BE CONSTRUCTED, ASSUME THAT IT PRINTED
        //---- ERROR MESSAGES.  SKIP THE BAD ONE AND KEEP GOING.
        Citizen *  pNext = NULL;
        try
        {
            pNext = Citizen::newInstance (nNextID,
                                          sNextCtznName,
                                          cParams.sublist (sNextCtznName),
                                          cProbDef,
                                          cLinConstr,
                                          (CallbackToMediator *) _pMediator,
                                          NULL);
            if (pNext != NULL)
            {
                //---- MEDIATOR WILL DESTROY THE CITIZEN WHEN IT IS FINISHED.
                if (_pMediator->addCitizen (pNext, false, -1) == false)
                {
                    cerr << "WARNING: Citizen '" << sNextCtznName << "'"
                         << " not added." << endl;
                    pNext = NULL;
                }
                else
                    nNumDefinedCtzns++;
            }
            else
                cerr << "WARNING: Citizen '" << sNextCtznName << "'"
                     << " not constructed." << endl;
        }
        catch (...)
        {
            cerr << "WARNING: Citizen '" << sNextCtznName << "'"
                 << " threw exception during construction." << endl;
            pNext = NULL;
        }
        if (pNext == NULL)
        {
            cout << " Ignoring the citizen and continuing." << endl;
        }
    }
    if (Print::doPrint (Print::MOST_VERBOSE))
        cout << "<End construction of citizens>" << endl;

    if (nNumDefinedCtzns == 0)
    {
        cerr << "ERROR: Could not construct any initial Citizens." << endl;
        return( false );
    }

    if (Print::doPrint (Print::INPUT_PARAMETERS) == true)
    {
        cout << "<begin printing information about the Mediator>" << endl;
        _pMediator->printDebugInfo();
        cout << "<end printing information about the Mediator>" << endl;
    }
    if (Print::doPrint (Print::MOST_VERBOSE))
    {
        cout << "---------- Showing processed input parameters ----------"
             << endl;
        cout << "<begin printing an alphabetized list of input parameters>"
             << endl;
        cParams.print (cout, 2);
        cout << "<end printing an alphabetized list of input parameters>"
             << endl;
        cout << "---------- Finished showing input parameters ----------"
             << endl;
    }

    return( true );
}


}     //-- namespace HOPSPACK
