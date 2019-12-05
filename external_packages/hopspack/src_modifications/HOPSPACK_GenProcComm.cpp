// $Id: HOPSPACK_GenProcComm.cpp 166 2010-03-22 19:58:07Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-shared/HOPSPACK_GenProcComm.cpp $ 

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
  \file HOPSPACK_GenProcComm.cpp
  \brief Implement HOPSPACK::GenProcComm.
*/

#include <string.h>    //-- FOR strlen()

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_GenProcComm.hpp"

#if defined(HAVE_MPI) || defined(HAVE_PVM)     //-- CLASS IS ONLY FOR MPI OR PVM

using namespace HOPSPACK;

//---- UNCOMMENT TO ENABLE GPC DEBUGGING THAT TRACES MPI MESSAGES.
//#define DEBUGGPC


#if defined(DEBUGGPC)
  static int  _nDebugRank = -1;
#endif


// ----------------------------------------------------------------------
// Static method:  getTheInstance
// ----------------------------------------------------------------------
GenProcComm &  GenProcComm::getTheInstance (void)
{
    static GenProcComm  theInstance;
    return( theInstance );
}


// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
GenProcComm::GenProcComm (void)
{
#if defined(HAVE_MPI)
  sendBuffer = NULL;
  recvBuffer = NULL;
  sendPosition = 0;
  recvPosition = 0; 
  msgSize = 0;
  HOPS_COMM = MPI_COMM_NULL;
#endif

#if defined(HAVE_PVM)
  lastBufferId = 0;
  pvmHostInfo = NULL;
  nHosts = 0;
  nArch = 0;
#endif

    return;
}


// ----------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------
GenProcComm::~GenProcComm (void)
{
    return;
}


// ----------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Initialize & Exit
// ----------------------------------------------------------------------

int GenProcComm::init(int &argc, char ** &argv, MPI_Comm mpi_comm_in)
{ 

  #if defined(DEBUGGPC)
    cout << "GPC init entered " << endl;  cout.flush();
  #endif

#if defined(HAVE_PVM)

  // PVM is initialized by calling any PVM command
  return GenProcComm::getMyTid();

#endif

#if defined(HAVE_MPI)

  // This routine puts all processes in a communicator called
  // HOPS_COMM.  Users who wish to divide the processes into more than
  // one communicator, in order to do multi-level parallelism for
  // instance, should add code to pare down worldgroup to just the set
  // of processes HOPS should use. This function returns the process'
  // rank.

  if (argc == -1) 
  {
    cerr << "ERROR: GenProcComm::init - must specify argc and argv when using MPI" << endl;
    throw MPI_PVM_ERROR;
  }

  MPI_Group worldGroup, appsGroup;
  int e;
  int groupSize;
  int* ranksInOldGroup;

  e = MPI_Init(&argc, &argv);

  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::init - error initializing MPI." << endl;
    throw MPI_PVM_ERROR;
  }
  
  // Get the set of processes (group, in MPI lingo) associated with 
  // MPI_COMM_WORLD.
  e = MPI_Comm_group(mpi_comm_in, &worldGroup);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::init - error calling MPI_Comm_group" << endl;
    throw MPI_PVM_ERROR;
  }

  // Find out how many process there are in the group.
  e = MPI_Group_size(worldGroup, &groupSize);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::init - error calling MPI_Group_size" << endl;
    throw MPI_PVM_ERROR;
  }

  // Build an array of ranks to include in the new communicator.  
  // The process with rank ranksInOldGroup[0] will have rank 0 in the 
  // new groupd, ranksInOldGroup[1] will have rank 1, etc.
  // In this case, since we want the new group to be the same as the
  // old, we include all ranks in the original order.
  ranksInOldGroup = new int[groupSize];
  for (int i = 0; i < groupSize; i++)
    ranksInOldGroup[i] = i;

  // Create the new group.
  e = MPI_Group_incl(worldGroup, groupSize, ranksInOldGroup, &appsGroup);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::init - error calling MPI_Group_incl" << endl;
    throw MPI_PVM_ERROR;
  }

  // Create a new communicator which has the processes in group
  // appsGroup, drawn from MPI_COMM_WORLD.
  e = MPI_Comm_create(mpi_comm_in, appsGroup, &HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::init - error calling MPI_Comm_create" << endl;
    throw MPI_PVM_ERROR;
  }

  delete[] ranksInOldGroup;

  #if defined(DEBUGGPC)
    _nDebugRank = GenProcComm::getMyTid();
  #endif

  return GenProcComm::getMyTid();

#endif     //-- HAVE_MPI
}


#if defined(HAVE_PVM)

int GenProcComm::init()
{
  int argc = -1;
  char** argv = NULL;
  return GenProcComm::init(argc, argv);
}

void GenProcComm::catchOutput(bool flag)
{
  // Set standard output to pipe through this process ONLY IF this
  // process HAS NOT been launched from the PVM console. If flag
  // (default true) is false, then disable output redirection.

  if (flag) 
  {
    if (pvm_parent() == PvmParentNotSet) // running from PVM console
      return;
    pvm_catchout(stdout);
    pvm_setopt(PvmShowTids, 0);
  }
  else
    pvm_catchout(0);
}

int GenProcComm::spawn(const string name, const string host, char* argv[])
{
  // Spawn a SINGLE task with the given name on a specified host. If
  // the host name is unspecified (""), then the task is spawned on
  // any available machine. Default argv is NULL. Return 0 if the
  // spawn failed. Otherwise, return the taskid.

  int taskId;
  int info;

  if (host == "") 
    info = pvm_spawn(const_cast<char*>(name.c_str()), argv, PvmTaskDefault, 
		     NULL, 1, &taskId);
  else 
    info = pvm_spawn(const_cast<char*>(name.c_str()), argv, PvmTaskHost, 
		     const_cast<char*>(host.c_str()), 1, &taskId);

  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::spawn - invalid parameter" << endl;
    throw MPI_PVM_ERROR;
  }

  if (info == PvmNoHost) 
  {
    cerr << "ERROR: GenProcComm::spawn - requested host is not in virtual machine" << endl;
    throw MPI_PVM_ERROR;
  }

  if (info == PvmNoFile) 
  {
    cerr << "ERROR: GenProcComm::spawn - executable cannot be found" << endl;
    return 0;
  }

  if (info == PvmNoMem) 
  {
    cerr << "ERROR: GenProcComm::spawn - not enough memory on host" << endl;
    return 0;
  }

  if (info == PvmSysErr) 
  {
    cerr <<  "ERROR: GenProcComm::spawn - pvmd not responding" << endl;
    return 0;
  }

  if (info == PvmOutOfRes) 
  {
    cerr <<  "ERROR: GenProcComm::spawn - out of resources" << endl;
    return 0;
  }

  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::spawn - unrecgonized error" << endl;
    throw MPI_PVM_ERROR;
  }

  if (info < 1)
    return 0;

  return taskId;
}

void GenProcComm::kill(int taskId)
{
  // Kill the process with the given task id.

  int info = pvm_kill(taskId);

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::kill -  pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::kill - invalid taskId" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::kill - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
}

#endif     //-- HAVE_PVM


void GenProcComm::exit()
{
  // Exit communication interface.

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] exit entered" << endl;  cout.flush();
  #endif

#if defined(HAVE_PVM)

  // In PVM it's very important to call this before exiting.

  int info = pvm_exit();

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::exit - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::exit - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

#endif

#if defined(HAVE_MPI)

  delete[] sendBuffer;
  delete[] recvBuffer;

  int e;

  if (HOPS_COMM == MPI_COMM_NULL)
  {
    cerr << "ERROR: GenProcComm::exit - MPI never initialized" << endl;
    throw MPI_PVM_ERROR;
  }

  // Wait for all processes to finish.
  e = MPI_Barrier(HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::exit - error in MPI barrier" << endl;
    throw MPI_PVM_ERROR;
  }
  // End MPI.
  e = MPI_Finalize();
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::exit - error in MPI finalize" << endl;
    throw MPI_PVM_ERROR;
  }

#endif

}

// ----------------------------------------------------------------------
// Status & Notify
// ----------------------------------------------------------------------

int GenProcComm::getMyTid() 
{
  // Return my taskId (or rank in MPI lingo).

#if defined(HAVE_PVM)

  int info = pvm_mytid();

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::getMyTid - pvmd is not running" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::mytid - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
  return info;

#endif

#if defined(HAVE_MPI)

  int rank;

  int e = MPI_Comm_rank(HOPS_COMM, &rank);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::mytid - error getting MPI rank." << endl;
    throw MPI_PVM_ERROR;
  }

  return rank;

#endif
}


#if defined(HAVE_MPI)

int GenProcComm::getNumProcs() 
{ 
  // Returns number of processes

  int nProcs;

  int e = MPI_Comm_size(HOPS_COMM, &nProcs);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::getNumProcs - error getting number of processes." << endl;
    throw MPI_PVM_ERROR;
  }
	
  return nProcs;
}

#endif     //-- HAVE_MPI


#if defined(HAVE_PVM)

bool GenProcComm::isOrphan()
{
  // Check if the process is an orphan, i.e., the process was spawned
  // from the command line or the PVM console.

  int info = pvm_parent();

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::isorphan - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if ((info < 0) && (info != PvmNoParent) && (info != PvmParentNotSet)) 
  {
    cerr << "ERROR: GenProcComm::isorphan - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

  return ((info == PvmNoParent) || (info == PvmParentNotSet));
}

int GenProcComm::parent()
{
  // Return the task id for the parent task. This will throw an error
  // if there is not a parent, so call if necessary isOrphan() first
  // to check.

  int info = pvm_parent();

  if (info == PvmNoParent) 
  {
    cerr << "ERROR: GenProcComm::parent - no PVM parent" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::parent - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::parent - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

  return info;
}

int GenProcComm::tidToHost(int t)
{
  // Find the host id for the given task.

  int info = pvm_tidtohost(t);

  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::tidtohost - invalid task id" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::tidtohost - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

  return info;
}

void GenProcComm::notify(int msgtag, int taskId)
{
  // Set notify for a SINGLE task 

  int info = pvm_notify(PvmTaskExit, msgtag, 1, &taskId);

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::notify - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::notify - invalid argument to pvm_notify" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::notify - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
}

void GenProcComm::notify(int msgtag)
{
  // Set notify for ALL host additions

  int info = pvm_notify(PvmHostAdd, msgtag, -1, NULL);

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::notify - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::notify - invalid argument to pvm_notify" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::notify - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
}

#endif     //-- HAVE_PVM


// ----------------------------------------------------------------------
// Send, Recv, and Related Commands
// ----------------------------------------------------------------------

void GenProcComm::initSend()
{
  // Initialize send.

#if defined(HAVE_PVM)

  int info = pvm_initsend(PvmDataDefault); // XDR encoding

  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::initsend - invalid encoding value" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmNoMem) 
  {
    cerr << "ERROR: GenProcComm::initsend - out of memory" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::initsend - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

#endif

#if defined(HAVE_MPI)

  delete[] sendBuffer;
  sendBuffer = NULL;
  sendPosition = 0;

#endif
}

void GenProcComm::send(int msgtag, int taskId)
{
  // Send a message with the specified message tag to the given task
  // id.

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] send entered with msgtag " << msgtag
         << " to " << taskId << endl;  cout.flush();
  #endif

#if defined(HAVE_PVM)

  int info = pvm_send(taskId, msgtag);

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::send - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::send - invalid argument" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmNoBuf) 
  {
    cerr << "ERROR: GenProcComm::send - no active send buffer" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::send - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

#endif

#if defined(HAVE_MPI)

  int e = MPI_Send(sendBuffer, sendPosition, MPI_PACKED, taskId, msgtag, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::send - error in MPI packed send" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}

bool GenProcComm::recv(int msgtag, int msgtid)
{
  // Blocking receive for a message with the given tag and task id.
  // The inputs msgtag and msgtid default to -1 (wildcard).  Returns
  // the buffer id of the message (PVM) or 1 (MPI), if any, and 0
  // otherwise.

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] recv entered for msgtag "
         << msgtag << endl;  cout.flush();
  #endif

#if defined(HAVE_PVM)

  int bufid = pvm_recv(msgtid, msgtag); 

  if (bufid == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::recv - invalid tid value or msgtag < -1" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::recv - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid < 0) 
  {
    cerr << "ERROR: GenProcComm::recv - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

  lastBufferId = bufid;
  return (bufid != 0) ? true : false;

#endif

#if defined(HAVE_MPI)

  if (msgtid == -1) 
    msgtid = MPI_ANY_SOURCE;
  if (msgtag == -1) 
    msgtag = MPI_ANY_TAG;

  // First, do a probe to find out how big to make the receive buffer
  int e = MPI_Probe(msgtid, msgtag, HOPS_COMM, &status);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::recv - error calling probe" << endl;
    throw MPI_PVM_ERROR;
  }
  msgSize = mpiGetMsgSize(status, MPI_PACKED);

  // Create the buffer.
  delete[] recvBuffer;
  recvBuffer = new char[msgSize];

  // Receive the message
  MPI_Recv(recvBuffer, msgSize, MPI_PACKED, msgtid, msgtag, HOPS_COMM, &status);

  // QUESTION: Should the value of status or some error code be
  // checked here?

  // The value recvPosition is incremented as the message is read via
  // calls to unpack. Set it to 0 so we can start reading at the
  // beginning of the message.
  recvPosition = 0;

  return true;

#endif
}

bool GenProcComm::probe(int msgtag, int msgtid)
{
  // Non-blocking probe for a message with the specified message tag
  // and task id. Returns the buffer id of the message for PVM or 1
  // for MPI, if any, and 0 otherwise. The inputs msgtag and msgtid
  // default to -1 (wildcard).

#if defined(HAVE_PVM)

  int bufid = pvm_probe(msgtid, msgtag);

  if (bufid == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::probe - invalid msgtid or msgtag" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::probe - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid < 0) 
  {
    cerr << "ERROR: GenProcComm::probe - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
  
  lastBufferId = bufid;
  return (bufid != 0) ? true : false;

#endif

#if defined(HAVE_MPI)

  int iflag;

  if (msgtid == -1) 
    msgtid = MPI_ANY_SOURCE;
  if (msgtag == -1)
    msgtag = MPI_ANY_TAG;

  // MPI sets iflag = 1 if there is a matching message, 0 if not.
  int e = MPI_Iprobe(msgtid, msgtag, HOPS_COMM, &iflag, &status);

  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::probe - error in MPI iprobe" << endl;
    throw MPI_PVM_ERROR;
  }

  return (iflag == 1) ? true : false;

#endif
}

void GenProcComm::bufinfo(int& msgtag, int& msgtid)
{
  // Determine the msgtag and msgtid for the most recently probed or
  // received message.
  
#if defined(HAVE_PVM)

  if (lastBufferId == 0) 
  {
    cerr << "ERROR: GenProcComm::bufinfo called without first doing recv or probe" << endl;
    throw MPI_PVM_ERROR;
  }

  int bytes;
  int info = pvm_bufinfo(lastBufferId, &bytes, &msgtag, &msgtid);
  
  if (info == PvmNoSuchBuf) 
  {
    cerr << "ERROR: GenProcComm::bufinfo - specified PVM buffer does not exist" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::bufinfo - invalid argument to pvm_bufinfo" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::bufinfo - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

#endif

#if defined(HAVE_MPI)

  msgtag = status.MPI_TAG;
  msgtid = status.MPI_SOURCE;

#endif
}


#if defined(HAVE_MPI)
void GenProcComm::bprobe(int msgtag, int msgtid) 
{ 
  // Do a blocking probe for a message with tag msgtag and from
  // process msgtid. The default values of msgtag and msgtid are both
  // -1.

  if (msgtid == -1) 
    msgtid = MPI_ANY_SOURCE;
  if (msgtag == -1) 
    msgtag = MPI_ANY_TAG;

  int e = MPI_Probe(msgtid, msgtag, HOPS_COMM, &status);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::bprobe - error in MPI probe" << endl;
    throw MPI_PVM_ERROR;
  }
}
#endif     //-- HAVE_MPI


#if defined(HAVE_PVM)

void GenProcComm::broadcast (int msgtag, const vector<int>& taskId)
{
  // Broadcast message in buffer using tag msgtag to the n tasks
  // listed in the taskId array.

  // MAY WANT TO FIX THIS??

  int n = taskId.size();
  int* taskIdcpy = new int[n];
  for (int i = 0; i < n; i ++)
    taskIdcpy[i] = taskId[i];


  int info = pvm_mcast(taskIdcpy, n, msgtag);

  if (info == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::broadcast - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::broadcast -  msgtag < 0" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmNoBuf) 
  {
    cerr << "ERROR: GenProcComm::broadcast - no active send buffer" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::broadcast - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }

  delete taskIdcpy;
}

bool GenProcComm::nrecv(int msgtag, int msgtid)
{
  // Non-blocking receive for a message with the given tag and task
  // id.  The inputs msgtag and msgtid default to -1 (wildcard).
  // Returns the buffer id of the message, if any, and 0 otherwise.

  int bufid = pvm_nrecv(msgtid, msgtag);

  if (bufid == PvmBadParam) 
  {
    cerr << "ERROR: GenProcComm::nrecv - invalid parameter" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid == PvmSysErr) 
  {
    cerr << "ERROR: GenProcComm::nrecv - pvmd not responding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (bufid < 0) 
  {
    cerr << "ERROR: GenProcComm::nrecv - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
 
  lastBufferId = bufid;
  return (bufid != 0) ? true : false;
}

#endif     //-- HAVE_PVM


// ----------------------------------------------------------------------
// Pack & Unpack
// ----------------------------------------------------------------------

void GenProcComm::pack(int i)
{
 // Pack a single integer.

#if defined(HAVE_PVM)

  pvmPackCheck(pvm_pkint(&i, 1, 1));

#endif
#if defined(HAVE_MPI)

  int bufsize = sendPosition + sizeof(int);
  mpiStretchSendBuffer(bufsize);
  int e = MPI_Pack(&i, 1, MPI_INT, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}

void GenProcComm::pack(char i)
{
 // Pack a single chareger.

#if defined(HAVE_PVM)

  pvmPackCheck(pvm_pkbyte(&i, 1, 1));

#endif
#if defined(HAVE_MPI)

  int bufsize = sendPosition + sizeof(char);
  mpiStretchSendBuffer(bufsize);
  int e = MPI_Pack(&i, 1, MPI_CHAR, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}

void GenProcComm::pack(double d)
{
  // Pack a single double.

#if defined(HAVE_PVM)

  pvmPackCheck(pvm_pkdouble(&d, 1, 1));

#endif
#if defined(HAVE_MPI)

  int bufsize = sendPosition + sizeof(double);
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack(&d, 1, MPI_DOUBLE, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}

void GenProcComm::pack(float d)
{
  // Pack a single float.

#if defined(HAVE_PVM)

  pvmPackCheck(pvm_pkfloat(&d, 1, 1));

#endif
#if defined(HAVE_MPI)

  int bufsize = sendPosition + sizeof(double);
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack(&d, 1, MPI_FLOAT, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}

void GenProcComm::pack(const char* s)
{
  // Pack character array WITH its length first

#if defined(HAVE_PVM)

  pack((int)strlen(s));
  pvmPackCheck(pvm_pkstr(const_cast<char*>(s)));

#endif
#if defined(HAVE_MPI)

  // Calculate length of string including terminator '\O'
  int n = strlen(s) + 1;

  // Pack string length
  pack(n);

  // Pack the string itself
  int bufsize = sendPosition + sizeof(char) * n; 
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack(const_cast<char*>(s), n, MPI_CHAR, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}

void GenProcComm::pack(bool b)
{
  // Pack a single bool. Bools are packed as ints because neither PVM
  // nor MPI can handle the bool datatype.

  int fakebool = (int) b;

#if defined(HAVE_PVM)

  pvmPackCheck(pvm_pkint(&fakebool, 1, 1));

#endif
#if defined(HAVE_MPI)

  pack(fakebool);
#endif
}

void GenProcComm::pack(const string s)
{

  // Pack a string.

  pack(s.c_str());
}


void GenProcComm::pack(const vector<int>& v)
{
    int n = v.size();
    pack(n);

#if defined(HAVE_PVM)
    //---- THIS IS THE OLD WAY, SLOW IF n IS LARGE.
    for (int i = 0; i < n; i ++)
        pack(v[i]);
#endif

#if defined(HAVE_MPI)
    int  bufsize = sendPosition + (n * sizeof(double));
    mpiStretchSendBuffer (bufsize);

    for (int i = 0; i < n; i ++)
    {
        int  k = v[i];
        int  e = MPI_Pack (&k, 1, MPI_INT, sendBuffer, bufsize,
                           &sendPosition, HOPS_COMM);
        if (e != MPI_SUCCESS)
        {
            cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
            throw MPI_PVM_ERROR;
        }
    }
#endif

    return;
}


void GenProcComm::pack(const Vector& v)
{
    int n = v.size();
    pack (n);

#if defined(HAVE_PVM)
    //---- THIS IS THE OLD WAY, SLOW IF n IS LARGE.
    for (int i = 0; i < n; i ++)
        pack(v[i]);
#endif

#if defined(HAVE_MPI)
    int  bufsize = sendPosition + (n * sizeof(double));
    mpiStretchSendBuffer (bufsize);

    for (int i = 0; i < n; i ++)
    {
        double  d = v[i];
        int  e = MPI_Pack (&d, 1, MPI_DOUBLE, sendBuffer, bufsize,
                           &sendPosition, HOPS_COMM);
        if (e != MPI_SUCCESS)
        {
            cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
            throw MPI_PVM_ERROR;
        }
    }
#endif

    return;
}


void GenProcComm::pack(const vector< Vector >& v)
{
    int n = v.size();
    pack(n);

    //---- THIS WILL BE SLOW IF n IS LARGE.
    for (int i = 0; i < n; i ++)
        pack(v[i]);
    return;
}


void GenProcComm::pack(const vector<string>& v)
{
    int n = v.size();
    pack(n);

    //---- THIS WILL BE SLOW IF n IS LARGE.
    for (int i = 0; i < n; i ++)
        pack(v[i]);

    return;
}


void GenProcComm::pack(const vector<bool>& v)
{
  // Bool vectors must be handled specially because they are stored is
  // a compacted format.
  int n = v.size();
  pack(n);
  int tmp;
  for (int i = 0; i < n; i ++) 
  {
    tmp = (int) v[i];
    pack(tmp);
  }
}    


void GenProcComm::pack(int length, const char* array)
{
  // Pack array WITH its length first

#if defined(HAVE_PVM)

  pack(length);
  pvmPackCheck(pvm_pkbyte(const_cast<char*>(array), length, 1));

#endif
#if defined(HAVE_MPI)

  // Pack array length
  pack(length);

  // Pack the array itself
  int bufsize = sendPosition + sizeof(char) * length; 
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack(const_cast<char*>(array), length, MPI_CHAR, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}


void GenProcComm::pack(int length, const int* array)
{
  // Pack array WITH its length first

#if defined(HAVE_PVM)

  pack(length);
  pvmPackCheck(pvm_pkint(const_cast<int*>(array), length, 1));

#endif
#if defined(HAVE_MPI)

  // Pack array length
  pack(length);

  // Pack the array itself
  int bufsize = sendPosition + sizeof(int) * length; 
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack((int*)array, length, MPI_INT, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}


void GenProcComm::pack(int length, const double* array)
{
  // Pack array WITH its length first

#if defined(HAVE_PVM)

  pack(length);
  pvmPackCheck(pvm_pkdouble((double*) array, length, 1));

#endif
#if defined(HAVE_MPI)

  // Pack array length
  pack(length);

  // Pack the array itself
  int bufsize = sendPosition + sizeof(double) * length; 
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack((double*)array, length, MPI_DOUBLE, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }
#endif
}


void GenProcComm::pack(int length, const float* array)
{
  // Pack array WITH its length first

#if defined(HAVE_PVM)

  pack(length);
  pvmPackCheck(pvm_pkfloat((float*) array, length, 1));

#endif
#if defined(HAVE_MPI)

  // Pack array length
  pack(length);

  // Pack the array itself
  int bufsize = sendPosition + sizeof(float) * length; 
  mpiStretchSendBuffer(bufsize);

  int e = MPI_Pack((float*)array, length, MPI_FLOAT, sendBuffer, bufsize, &sendPosition, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::pack - error in MPI pack" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(int& i)
{
  // Unpack a single integer

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkint(&i, 1, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack int" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, &i, 1, MPI_INT, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack int"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(char& i)
{
  // Unpack a single integer

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkbyte(&i, 1, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack char" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, &i, 1, MPI_CHAR, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack char"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(double& d)
{
  // Unpack a single double

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkdouble(&d, 1, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack double" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, &d, 1, MPI_DOUBLE, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack double"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(float& d)
{
  // Unpack a single float

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkfloat(&d, 1, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack float" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, &d, 1, MPI_FLOAT, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack float"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(char*& s)
{
  // Unpack char array s. If s is NULL, memory is allocated for it. If
  // not, the length of s is checked to be sure it's large enough.

  int len;
  unpack(len);

  if (!s) 
    s = new char[len];
  else if (((int)strlen(s)) < len) 
  {
    cerr << "ERROR: GenProcComm::unpack - string is too small for unpack" << endl;
    throw MPI_PVM_ERROR;
  }

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkstr(s));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack string" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, s, len, MPI_CHAR, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack string"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(bool& b)
{
  // Unpack a single boolean.

#if defined(HAVE_PVM)

  int bfake;
  pvmUnpackCheck(pvm_upkint(&bfake, 1, 1));
  b = (bool) bfake;

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack bool" << endl;
    cout.flush();
  #endif
  int bfake;
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, &bfake, 1, MPI_INT, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack bool"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

  b = (bool) bfake;

#endif
}


void GenProcComm::unpack(string& s)
{

  char* tmp = NULL;
  unpack(tmp);

  s = tmp;


  try
  {
    delete [] tmp;
  }
  catch(...)
  {
  }
}


void GenProcComm::unpack(vector<int>& v)
{
  int n;
  unpack(n);
  v.resize(n);
  for (int i = 0; i < n; i ++)
    unpack(v[i]);
}


void GenProcComm::unpack(Vector& v)
{
  int n;
  unpack(n);
  v.resize(n);
  for (int i = 0; i < n; i ++)
    unpack(v[i]);
}


void GenProcComm::unpack(vector< Vector >& v)
{
  int n;
  unpack(n);
  v.resize(n);
  for (int i = 0; i < n; i ++)
    unpack(v[i]);
}


void GenProcComm::unpack(vector<string>& v)
{
  int n;
  unpack(n);
  v.resize(n);
  for (int i = 0; i < n; i ++)
    unpack(v[i]);
}


void GenProcComm::unpack(vector<bool>& v)
{
  // Bool vectors must be handled specially because they are stored is
  // a compacted format.
  int n;
  unpack(n);
  v.resize(n);
  int tmp;
  for (int i = 0; i < n; i ++) 
  {
    unpack(tmp);
    v[i] = (bool) tmp;
  }
}    


void GenProcComm::unpack(int& length, char*& array)
{
  // Unpack array s. If s is NULL, memory is allocated for it. If
  // not, the length of s is checked to be sure it's large enough.

  unpack(length);
  array = new char[length];

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkbyte(array, length, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack char array" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, array, length, MPI_CHAR, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack char array"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(int& length, int*& array)
{
  // Unpack array s. If s is NULL, memory is allocated for it. If
  // not, the length of s is checked to be sure it's large enough.

  unpack(length);
  array = new int[length];

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkint(array, length, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack int array" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, array, length, MPI_INT, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack int array"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(int& length, double*& array)
{
  // Unpack array s. If s is NULL, memory is allocated for it. If
  // not, the length of s is checked to be sure it's large enough.

  unpack(length);
  array = new double[length];

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkdouble(array, length, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack double array" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, array, length, MPI_DOUBLE, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack double array"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


void GenProcComm::unpack(int& length, float*& array)
{
  // Unpack array s. If s is NULL, memory is allocated for it. If
  // not, the length of s is checked to be sure it's large enough.

  unpack(length);
  array = new float[length];

#if defined(HAVE_PVM)

  pvmUnpackCheck(pvm_upkfloat(array, length, 1));

#endif
#if defined(HAVE_MPI)

  #if defined(DEBUGGPC)
    cout << "GPC [" << _nDebugRank << "] calling MPI_Unpack float array" << endl;
    cout.flush();
  #endif
  int e = MPI_Unpack(recvBuffer, msgSize, &recvPosition, array, length, MPI_FLOAT, HOPS_COMM);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::unpack - error in MPI unpack float array"
         << " [" << GenProcComm::getMyTid() << "]" << endl;
    throw MPI_PVM_ERROR;
  }

#endif
}


// ----------------------------------------------------------------------
// Host Info
// ----------------------------------------------------------------------

#if defined(HAVE_PVM)

int GenProcComm::resetHostInfo() 
{
  // Reset host information by asking PVM to update pvmHostInfo - the data
  // structure that contains the host information. Return the number
  // of hosts.

  pvmHostInfo = NULL;

  if ((pvm_config(&nHosts, &nArch, &pvmHostInfo)) < 0) 
  {
    cerr << "ERROR: GenProcComm::resetHostInfo - error from pvm_config" << endl;
    throw MPI_PVM_ERROR;
  }

  return nHosts;
}

int GenProcComm::getHostTid(int i) 
{
  // Get the host id of the ith host.

  return pvmHostInfo[i].hi_tid;
}

int GenProcComm::getHostSpeed(int i) 
{
  // Get the speed of the ith host.

  return pvmHostInfo[i].hi_speed;
}

char* GenProcComm::getHostName(int i) 
{
  // Get the name of the ith host.

  return pvmHostInfo[i].hi_name;
}

#endif     //-- HAVE_PVM


// ----------------------------------------------------------------------
// Helper functions for MPI
// ----------------------------------------------------------------------

#if defined(HAVE_MPI)

int GenProcComm::mpiGetMsgSize(MPI_Status localstatus, MPI_Datatype datatype)
{
  // Returns the size of the message localstatus is associated with.
  // "localstatus" is so named to separate it from the global variable
  // "status."

  int count;

  // This MPI call sets count = the number of items of type datatype
  // in the message. 
  int e = MPI_Get_count(&localstatus, datatype, &count);
  if (e != MPI_SUCCESS) 
  {
    cerr << "ERROR: GenProcComm::mpiGetMsgSize - error" << endl;
    throw MPI_PVM_ERROR;
  }

  return(count);
}

void GenProcComm::mpiStretchSendBuffer(int bufsize)
{
  // This is a helper function for the pack methods.  It makes the
  // size of the send buffer bufsize.

  // Create new buffer
  char* newbuffer = new char[bufsize];
  
  // Copy old buffer into new
  for(int i = 0; i < sendPosition; i++)
    newbuffer[i] = sendBuffer[i];

  // Delete old buffer and replace with new
  delete[] sendBuffer;
  sendBuffer = newbuffer;
}

#endif     //-- HAVE_MPI


// ----------------------------------------------------------------------
// Helper functions for PVM
// ----------------------------------------------------------------------

#if defined(HAVE_PVM)

void GenProcComm::pvmPackCheck(int info)	// PRIVATE
{
  // Check return code from pvm_pack

  if (info == PvmNoMem) 
  {
    cerr << "ERROR: GenProcComm::pack - out of memory" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmNoBuf) 
  {
    cerr << "ERROR: GenProcComm::pack - no active send buffer" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::pack - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
}

void GenProcComm::pvmUnpackCheck(int info) // PRIVATE
{
  // Check return code form pvm_unpack

  if (info == PvmNoData) 
  {
    cerr << "ERROR: GenProcComm::unpack - reading past end of buffer" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmBadMsg) 
  {
    cerr << "ERROR: GenProcComm::unpack - incompatiable encoding" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info == PvmNoBuf) 
  {
    cerr << "ERROR: GenProcComm::unpack - no active receive buffer" << endl;
    throw MPI_PVM_ERROR;
  }
  if (info < 0) 
  {
    cerr << "ERROR: GenProcComm::unpack - unrecognized PVM error" << endl;
    throw MPI_PVM_ERROR;
  }
}

#endif     //-- HAVE_PVM

#endif     //-- HAVE_MPI || HAVE_PVM
