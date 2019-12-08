// $Id: HOPSPACK_main_mpi.cpp 166 2010-03-22 19:58:07Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-main/HOPSPACK_main_mpi.cpp $ 

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
  @file HOPSPACK_main_mpi.cpp
  @brief Main program that executes HOPSPACK as multiple processes using MPI
         (but no multithreading).
*/

#include <string.h>    //-- FOR strcpy() and strlen()
#include<stdexcept> // MJL

#include "HOPSPACK_common.hpp"
//#include "HOPSPACK_EvaluatorDefault.hpp" // MJL
#include "HOPSPACK_MangoEvaluator.hpp" // MJL
#include "HOPSPACK_ExecutorMpi.hpp"
#include "HOPSPACK_GenProcComm.hpp"
#include "HOPSPACK_Hopspack.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Print.hpp"
#include "HOPSPACK_utils.hpp"
#include "HOPSPACK_Vector.hpp"

#include "mango.hpp"

//----------------------------------------------------------------------
//  Static declarations
//----------------------------------------------------------------------

//---- RETURN CODES FOR main().
static const int  nSUCCESS         =  0;
static const int  nERROR           = -1;
static const int  nMPI_BUILD_ERROR = -2;
static const int  nMPI_INIT_FAILED = -3;
static const int  nWORKER_KILLED   = -4;


//----------------------------------------------------------------------
//  Internal Function startProcComm_
//----------------------------------------------------------------------
/** All processors call this to start inter-process communications.
 *  MPI may modify command line arguments.
 *  Return the rank of this processor (0, 1, ...), or -1 if startup failed.
 */
/*
static int  startProcComm_ (int &                    nCopyArgc,
                            char ** &                saCopyArgv,
                            HOPSPACK::GenProcComm &  cGPC, MPI_Comm mpi_comm_in)
*/
int  HOPSPACK_startProcComm(HOPSPACK::GenProcComm &  cGPC, MPI_Comm mpi_comm_in)
{
    int  nResult = -1;

    try
    {
      //nResult = cGPC.init (nCopyArgc, saCopyArgv, mpi_comm_in);
      nResult = cGPC.init (mpi_comm_in);
    }
    catch (const char * const)
    {
        cerr << "ERROR: Failed to initialize MPI/PVM." << endl;
        cGPC.exit();
        nResult = -1;
    }

    return( nResult );
}


//----------------------------------------------------------------------
//  Internal Function masterEndsProcComm_
//----------------------------------------------------------------------
/** The master calls this to shut down processors cleanly.
 */
static void  masterEndsProcComm_ (HOPSPACK::GenProcComm &  cGPC)
{
    using HOPSPACK::GenProcComm;

    //---- COMMAND ALL MPI PROCESSORS TO STOP, EVEN IF NOT ALL WERE USED.
    int  nNumMpiProcs = cGPC.getNumProcs();
    cGPC.initSend();
    for (int  i = 1; i < nNumMpiProcs; i++)
    {
        cGPC.pack ('T');
        cGPC.send (GenProcComm::TERMINATE, i);
    }
    cGPC.exit();

    return;
}


//----------------------------------------------------------------------
//  Internal Function allocateMpiProcesses_
//----------------------------------------------------------------------
/** Return false if MPI and user parameters do not agree there are enough
 *  processors for execution.  If true, then also fill in the number of
 *  processors to be reserved for citizen workers, understanding that
 *  remaining processes are for evaluation workers.
 */
static bool  allocateMpiProcesses_
                 (const HOPSPACK::ParameterList &  cParams,
                        HOPSPACK::GenProcComm   &  cGPC,
                        int                     &  nNumProcs,
                        int                     &  nNumCtznWrkrProcs)
{
    //---- HOPSPACK NEEDS 1 MASTER PROCESSOR, AT LEAST 1 EVALUATION PROCESSOR,
    //---- AND A (POSSIBLY EMPTY) POOL OF CITIZEN WORKER PROCESSORS.
    //---- MPI MUST HAVE AT LEAST THAT MANY PROCESSORS AVAILABLE.

    HOPSPACK::ParameterList  cMedParams = cParams.sublist ("Mediator");

    //---- ACCEPT EITHER PARAMETER NAME.
    const char  sNUM_PROCS[] = "Number Processors";     //-- OFFICIAL NAME
    const char  sPROCS[] = "Processors";                //-- ALTERNATE NAME

    if (   (cMedParams.isParameterInt (sPROCS) == false)
        && (cMedParams.isParameterInt (sNUM_PROCS) == false) )
    {
        cerr << "ERROR: Need '" << sNUM_PROCS
             << "' in 'Mediator' sublist" << endl;
        return( false );
    }
    int  nNumReqProcs = -1;
    if (cMedParams.isParameterInt (sPROCS) == true)
    {
        nNumReqProcs = cMedParams.getParameter (sPROCS, -1);
    }
    else if (cMedParams.isParameterInt (sNUM_PROCS) == true)
    {
        nNumReqProcs = cMedParams.getParameter (sNUM_PROCS, -1);
    }
    if (nNumReqProcs < 2)
    {
        cerr << "ERROR: Bad '" << sNUM_PROCS << "' value " << nNumReqProcs
             << " in 'Mediator' sublist" << endl;
        cerr << "  Must be >= 2" << endl;
        return( false );
    }

    const char  sRSRV_WRKRS[] = "Reserved Citizen Workers";
    nNumCtznWrkrProcs = 0;
    if (cMedParams.isParameterInt (sRSRV_WRKRS) == true)
    {
        nNumCtznWrkrProcs = cMedParams.getParameter (sRSRV_WRKRS, -1);
    }
    if (nNumCtznWrkrProcs < 0)
    {
        cerr << "ERROR: Bad '" << sRSRV_WRKRS << "' value " << nNumCtznWrkrProcs
             << " in 'Mediator' sublist" << endl;
        return( false );
    }
    if (nNumCtznWrkrProcs > (nNumReqProcs - 2))
    {
        cerr << "ERROR: Bad '" << sRSRV_WRKRS << "' value " << nNumCtznWrkrProcs
             << "' in 'Mediator' sublist" << endl;
        cerr << "  Cannot exceed '" << sNUM_PROCS << "' = " << nNumReqProcs
             << " - 2" << endl;
        return( false );
    }

    //---- FIND THE NUMBER OF PROCESSORS MPI HAS AVAILABLE.
    try
    {
        nNumProcs = cGPC.getNumProcs();
    }
    catch (const char * const)
    {
        cerr << "ERROR: Failed to getNumProcs() from MPI/PVM." << endl;
        return( false );
    }

    //---- ARE THERE ENOUGH PROCESSORS?
    if (nNumReqProcs > nNumProcs)
    {
        cerr << "ERROR: Bad '" << sNUM_PROCS << "' value " << nNumReqProcs
             << " in 'Mediator' sublist" << endl;
        cerr << "  Too large, MPI provides only " << nNumProcs << endl;
        return( false );
    }

    //---- ARE THERE EXTRA PROCESSORS THAT WILL NOT BE USED?
    if (nNumProcs > nNumReqProcs)
    {
        if (HOPSPACK::Print::doPrint (HOPSPACK::Print::INPUT_PARAMETERS))
            cout << "MPI provides " << nNumProcs << " processors"
                 << ", but '" << sNUM_PROCS << "' requested only "
                 << nNumReqProcs << endl;
        nNumProcs = nNumReqProcs;
    }

    return( true );
}


//----------------------------------------------------------------------
//  Internal Function behaveAsMaster_
//----------------------------------------------------------------------
/** The master processor examines user input parameters, validates them,
 *  decides what role other processors will play, and initializes them
 *  by sending appropriate user parameters.  The master can decide to
 *  stop execution immediately if parameters are not correct.
 */
int  HOPSPACK_behaveAsMaster(HOPSPACK::GenProcComm &  cGPC,
				    HOPSPACK::ParameterList* hopspack_parameters)
{
    using HOPSPACK::parseTextInputFile;   //-- FROM HOPSPACK_utils.hpp
    using HOPSPACK::ParameterList;
    using HOPSPACK::Print;
    using HOPSPACK::GenProcComm;
    using HOPSPACK::ExecutorMpi;
    using HOPSPACK::Hopspack;

    ParameterList  cParams = *hopspack_parameters;

    /*
    if (nArgc < 2)
    {
        cerr << "ERROR: Need the input file name." << endl;
        cout << "Usage:  HOPSPACK_main_mpi <input file>" << endl;
        cout << "  The input file contains HOPSPACK parameters in text form."
             << endl;
        masterEndsProcComm_ (cGPC);
        return( nERROR );
    }
    ParameterList  cParams = *hopspack_parameters;

    // <MJL> Edit parameters
    HOPSPACK::ParameterList*  cProblemParams = &(cParams.getOrSetSublist ("Problem Definition"));
    cProblemParams->setParameter("Objective Type","Minimize");
    cProblemParams->setParameter("Number Unknowns",2);
    HOPSPACK::Vector scaling(2,1.0);
    cProblemParams->setParameter("Scaling",scaling);
    HOPSPACK::Vector x0(2,0.0);
    cProblemParams->setParameter("Initial X",x0);
    cProblemParams->setParameter("Display",2);

    HOPSPACK::ParameterList*  cMedParams = &(cParams.getOrSetSublist ("Mediator"));
    //cout << "MJL Number Processors:" << cMedParams->getParameter("Number Processors",-17) << endl;
    cMedParams->setParameter("Number Processors",2);
    cMedParams->setParameter("Citizen Count",1);
    cMedParams->setParameter("Display",2);

    HOPSPACK::ParameterList*  cCitizenParams = &(cParams.getOrSetSublist ("Citizen 1"));
    cCitizenParams->setParameter("Type","GSS");
    cCitizenParams->setParameter("Display",1);
    cCitizenParams->setParameter("Step Tolerance",1e-5);

    // </MJL>

    
    if (parseTextInputFile (saArgv[1], cParams) == false)
    {
        masterEndsProcComm_ (cGPC);
        return( nERROR );
    }
    */

    //---- ALLOCATE MPI PROCESSES FOR VARIOUS TYPES OF WORKERS.
    int  nNumProcs = 0;
    int  nNumCtznWorkers = 0;
    if (allocateMpiProcesses_ (cParams, cGPC,
                               nNumProcs, nNumCtznWorkers) == false)
    {
        masterEndsProcComm_ (cGPC);
        return( nERROR );
    }
    int  nNumEvalWorkers = nNumProcs - nNumCtznWorkers - 1;

    //---- ASSIGN MPI PROCESS RANK NUMBERS FOR THE EVALUATION WORKERS.
    vector< int >  naEvalWorkerIDs;
    for (int  i = 1 + nNumCtznWorkers; i < nNumProcs; i++)
        naEvalWorkerIDs.push_back (i);

    //---- SEND PARAMETERS TO THE EVALUATION WORKERS, AND ASSUME THEY
    //---- INITIALIZE SUCCESSFULLY.  (BECAUSE WE SEND PARAMETERS, THE USER
    //---- ONLY HAS TO EDIT THE PARAMETER FILE ON THE MASTER MACHINE AND
    //---- CHANGES ARE AUTOMATICALLY PROPAGATED.)
    cGPC.initSend();
    for (int  i = 0; i < (int) naEvalWorkerIDs.size(); i++)
    {
        cParams.sublist ("Evaluator").pack (cGPC);
        cGPC.send (GenProcComm::INITWORKER_EVAL, naEvalWorkerIDs[i]);
    }

    //---- CONSTRUCT AN EXECUTOR THAT WILL USE MPI TO COMMUNICATE WITH
    //---- EVALUATION WORKERS.
    ExecutorMpi *  pExecutor = new ExecutorMpi();
    if (pExecutor->initialize(naEvalWorkerIDs) == false)
    {
        cerr << "ERROR: Could not construct Executor." << endl;
        masterEndsProcComm_ (cGPC);
        return( nERROR );
    }

    //---- CONSTRUCT THE OPTIMIZER, CONFIGURE IT, AND RUN IT.
    Hopspack  optimizer (pExecutor);
    if (optimizer.setInputParameters (cParams) == true)
    {
        if (Print::doPrint (Print::FINAL_SOLUTION))
        {
            cout << endl << "Begin solving using the MPI HOPSPACK executable."
                 << endl;
            if (Print::doPrint (Print::INPUT_PARAMETERS))
            {
                cout << "  1 MPI processor for the Mediator" << endl;
                if (nNumCtznWorkers > 0)
                    cout << "  " << nNumCtznWorkers
                         << " MPI processors for citizen workers" << endl;
                cout << "  " << nNumEvalWorkers
                     << " MPI processors for evaluation workers" << endl;
            }
        }
        optimizer.solve();
    }

    delete pExecutor;

    masterEndsProcComm_ (cGPC);
    return( nSUCCESS );
}


//----------------------------------------------------------------------
//  Internal Function doEvalWorkerLoop_
//----------------------------------------------------------------------
/** Make an evaluation when the master requests it.
 */
static void  doEvalWorkerLoop_ (const HOPSPACK::ParameterList &  cEvalParams,
                                const int                        nProcRank,
				HOPSPACK::GenProcComm   &  cGPC,
				mango::problem* this_problem)
{
    using HOPSPACK::EvalRequestType;
    //using HOPSPACK::EvaluatorDefault; // MJL
    using HOPSPACK::GenProcComm;
    using HOPSPACK::Vector;

    int  nMsgTag;
    int  nTmp;
    bool  bDebug = (cEvalParams.getParameter ("Debug Eval Worker", 0) == 1);


    /*    EvaluatorDefault *  pEvaluator
	  = EvaluatorDefault::newInstance (cEvalParams);  //MJL */
    HOPSPACK_MangoEvaluator *  pEvaluator = new HOPSPACK_MangoEvaluator (cEvalParams, this_problem); // MJL
    if (pEvaluator == NULL)
    {
        cerr << "ERROR: Could not construct Evaluator." << endl;
        return;
    }
    if (bDebug)
    {
        cout << "[np=" << nProcRank << "] Evaluator:" << endl;
        pEvaluator->printDebugInfo();
    }

    //---- LOOP UNTIL THE MASTER SENDS A TERMINATE MESSAGE.
    while (true)
    {
        cGPC.recv();
        cGPC.bufinfo (nMsgTag, nTmp);
        if (nMsgTag == GenProcComm::TERMINATE)
        {
            if (bDebug)
                cout << "[np=" << nProcRank << "]"
                     << " told by master to TERMINATE" << endl;
            break;
        }
        else if (nMsgTag == GenProcComm::EVALUATE)
        {
            int     nTag;
            Vector  cX;
            int     nRequest;
            cGPC.unpack (nTag);
            cGPC.unpack (cX);
            cGPC.unpack (nRequest);

            if (bDebug)
                cout << "[np=" << nProcRank << "]"
                     << " told by master to EVALUATE tag " << nTag << endl;

            if (nRequest == HOPSPACK::EVALREQTYPE_F)
            {
                Vector  cResultF;
                Vector  cEmpty;
                string  sResultMsg;
                pEvaluator->evalF (nTag, cX, cResultF, sResultMsg);

                //---- SEND THE RESULT BACK TO THE MASTER.
                cGPC.initSend();
                cGPC.pack (nTag);
                cGPC.pack (cResultF);
                cGPC.pack (cEmpty);
                cGPC.pack (cEmpty);
                cGPC.pack (sResultMsg);
                cGPC.send (GenProcComm::EVALUATE, 0);
            }
            else if (nRequest == HOPSPACK::EVALREQTYPE_FC)
            {
                Vector  cResultF;
                Vector  cResultEqs;
                Vector  cResultIneqs;
                string  sResultMsg;
                pEvaluator->evalFC (nTag, cX, cResultF,
                                    cResultEqs, cResultIneqs, sResultMsg);

                //---- SEND THE RESULT BACK TO THE MASTER.
                cGPC.initSend();
                cGPC.pack (nTag);
                cGPC.pack (cResultF);
                cGPC.pack (cResultEqs);
                cGPC.pack (cResultIneqs);
                cGPC.pack (sResultMsg);
                cGPC.send (GenProcComm::EVALUATE, 0);
            }
            else
            {
                cerr << "ERROR: Evaluator request type " << nRequest
                     << " not implemented <main_mpi::doEvalWorkerLoop>" << endl;
                throw HOPSPACK::INTERNAL_ERROR;
            }
        }
        else
        {
            cerr << "[np=" << nProcRank << "]"
                 << " ERROR: recv unknown msg tag " << nMsgTag
                 << ", exiting" << endl;
            break;
        }
    }

    delete pEvaluator;
    return;
}


//----------------------------------------------------------------------
//  Internal Function behaveAsWorker_
//----------------------------------------------------------------------
/** A worker is either a member of the reserved citizen worker pool,
 *  or is a member of the evaluation pool.
 *  When finished, call GenProcComm.exit() and return the status that
 *  will be returned by main().
 */
int  HOPSPACK_behaveAsWorker(const int                nProcRank,
                             HOPSPACK::GenProcComm &  cGPC,
			     mango::problem* this_problem)
{
    using HOPSPACK::GenProcComm;
    using HOPSPACK::ParameterList;

    int  nMsgTag;
    int  nTmp;

    //---- BLOCK UNTIL THE MASTER SENDS THE INITIAL MESSAGE.
    cGPC.recv();
    cGPC.bufinfo (nMsgTag, nTmp);
    ParameterList  cWorkerParams;
    if (nMsgTag == GenProcComm::TERMINATE)
    {
        //---- MASTER MUST HAVE DETECTED BAD SETUP.
        cGPC.exit();
        return( nWORKER_KILLED );
    }
    else if (nMsgTag == GenProcComm::INITWORKER_EVAL)
    {
        //---- THIS WORKER DOES EVALUATIONS.
        cWorkerParams.unpack (cGPC);
        if (cWorkerParams.getParameter ("Debug Eval Worker", 0) == 1)
        {
            cout << "[np=" << nProcRank << "]"
                 << " <begin printing input parameters from the master>"
                 << endl;
            cWorkerParams.print (cout, 2);
            cout << "[np=" << nProcRank << "]"
                 << " <end printing input parameters>" << endl;
        }
        doEvalWorkerLoop_ (cWorkerParams, nProcRank, cGPC, this_problem);
        cGPC.exit();
    }
    else if (nMsgTag == GenProcComm::INITWORKER_CTZN)
    {
        //---- THIS WORKER IS IN THE CITIZEN WORKER POOL.
        cWorkerParams.unpack (cGPC);
        if (cWorkerParams.getParameter ("TBD some debug parameter", 0) == 1)
        {
            cout << "[np=" << nProcRank << "]"
                 << " <begin printing input parameters from the master>"
                 << endl;
            cWorkerParams.print (cout, 2);
            cout << "[np=" << nProcRank << "]"
                 << " <end printing input parameters>" << endl;
        }
        //TBD...do loop goes here, not coded yet
        cGPC.exit();
    }
    else
    {
        cerr << "ERROR: Worker process not initialized by master." << endl;
        cGPC.exit();
        return( nMPI_INIT_FAILED );
    }

    return( nSUCCESS );
}


//--------------------------------------------------------------------
//! Main routine for MPI version of HOPSPACK.
/*!
 *  MPI starts the same executable on each node.
 *  Processes start, discover their MPI processor rank, and then behave
 *  differently depending whether they are
 *  - The master process that runs the Mediator in Hopspack::solve().
 *    Rank = 0
 *  - A citizen worker that performs parallel work for a citizen.
 *    Rank = 1 thru "Reserved Citizen Workers"
 *  - An evaluation worker that evaluates one point at a time.
 *    Rank = "Reserved Citizen Workers" + 1 thru "Number Processors"
 *
 *  Note that MPI starts this executable and does not pass MPI command line
 *  arguments.
 *
 *  @param[in] nArgc   Number of command line arguments (typically, 1).
 *  @param[in] saArgv  Command line arguments (typically, parameters file name).
 */
//--------------------------------------------------------------------

/*
int  main (const int           nArgc,
           const char * const  saArgv[])
{
    using HOPSPACK::GenProcComm;


    #if !defined(HAVE_MPI)
        //---- THIS INDICATES THE BUILD SYSTEM IS MESSED UP.
        cerr << "ERROR: Executable was compiled without MPI." << endl;
        return( nMPI_BUILD_ERROR );
    #endif

    //---- COPY THE COMMAND LINE ARGUMENTS AS MPI MAY ALTER THEM.
    //---- CANNOT DELETE THE MEMORY AFTER MPI MAKES ALTERATIONS.
    char **  saCopyArgv = new char*[nArgc];
    for (int  i = 0; i < nArgc; i++)
    {
        saCopyArgv[i] = new char[strlen (saArgv[i]) + 1];
        strcpy (saCopyArgv[i], saArgv[i]);
        saCopyArgv[i][strlen (saArgv[i])] = 0;
    }
    int  nCopyArgc = nArgc;

    // <MJL> Move MPI_Init to the driver.
    int e = MPI_Init(&nCopyArgc, &saCopyArgv);
    if (e != MPI_SUCCESS) throw std::runtime_error("Error initializing MPI");
    // </MJL>

    GenProcComm &  cGPC = GenProcComm::getTheInstance();

    //---- START INTER-PROCESS COMMUNICATION.
    int  nProcRank = startProcComm_ (nCopyArgc, saCopyArgv, cGPC, MPI_COMM_WORLD);
    if (nProcRank == -1)
        return( nMPI_INIT_FAILED );

    //---- BRANCH AND EXECUTE APPROPRIATE CODE FOR THIS PROCESSOR'S ROLE.
    //---- ALL PROCESSORS SHOULD USE GenProcComm::Exit() TO FINISH THIS
    //---- BLOCK OF CODE AT THE SAME TIME.
    int  nReturnValue;
    if (nProcRank == 0)
    {
        nReturnValue = behaveAsMaster_ (nCopyArgc, saCopyArgv, cGPC);
    }
    else
    {
        nReturnValue = behaveAsWorker_ (nProcRank, cGPC);
    }

    MPI_Finalize();

    return( nReturnValue );
}
*/
