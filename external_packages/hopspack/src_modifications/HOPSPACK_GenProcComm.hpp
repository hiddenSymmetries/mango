// $Id: HOPSPACK_GenProcComm.hpp 149 2009-11-12 02:40:41Z tplante $ 
// $URL: svn+ssh://software.sandia.gov/svn/private/hopspack/trunk/src/src-shared/HOPSPACK_GenProcComm.hpp $ 

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
  \file HOPSPACK_GenProcComm.hpp
  \brief Class declaration of HOPSPACK::GenProcComm.
*/
#ifndef HOPSPACK_GENPROCCOMM_H
#define HOPSPACK_GENPROCCOMM_H

#include "HOPSPACK_common.hpp"
#include "HOPSPACK_Vector.hpp"

//---- THIS CLASS IF DEFINED ONLY FOR MPI OR PVM.
#if defined(HAVE_MPI) || defined(HAVE_PVM)


#if defined(HAVE_PVM)
  #include "pvm3.h"
#endif

#if defined(HAVE_MPI)
  #include "mpi.h"
#endif


namespace HOPSPACK {

/*!
  \brief  Methods for general inter-process communication using MPI or PVM.

  Although this class supports PVM, it is no longer officially supported
  by or tested with HOPSPACK. If you would like to use PVM, please contact
  the developers.

  \author MPI interface contributed by H. Alton Patrick, Summer 2000.
*/
class GenProcComm {

public:

    //! Message IDs to coordinate sender and receiver.
    enum MsgTag
    {
        //! First message received by an evaluation pool processor.
        INITWORKER_EVAL = 0,

        //! First message received by a citizen worker pool processor.
        INITWORKER_CTZN,

        //! Master tells evaluation processor to make an evaluation.
        EVALUATE,

        //! Master tells workers to halt.
        TERMINATE
    };


    //! Applications call this instead of a constructor.
    static GenProcComm &  getTheInstance (void);

    //! Applications should not call this directly (managed internally).
    ~GenProcComm (void);

    //@{ \name Initialization and exit

    //! For MPI or PVM, returns process rank. Otherwise, throws an error.
    /*! This routine puts all processes in a communicator called
        HOPS_COMM. Users who wish to divide the processes into more than
        one communicator, in order to do multi-level parallelism for
        instance, should add code to pare down worldgroup to just the set
        of processes HOPSPACK should use. */
  int init(int &argc, char ** &argv, MPI_Comm);

  #if defined(HAVE_PVM)
    //! For PVM, returns PVM taskid. Otherwise, throws an error.
    int init();
  
    //! For PVM, set this process to catch all the output. Otherwise, prints a warning.
    /*! Set standard output to pipe through this process ONLY IF this
        process HAS NOT been launched from the PVM console. If flag
        (default true) is false, then disable output redirection.*/
    void catchOutput(bool flag = true);

    //! For PVM, spawn a task. Otherwise, throws an error.
    /*! Spawn a SINGLE task with the given name on a specified host. If
        the host name is unspecified (""), then the task is spawned on
        any available machine. Argv is the array of arguments to the
        spawned job. Default argv is NULL. Return 0 if the spawn
        failed. Otherwise, return the PVM taskid. */
    int spawn(const string name, const string host, char* argv[] = NULL);

    //! For PVM, kills the specified taskid. Otherwise, prints a warning.
    void kill(int tid);
  #endif     //-- HAVE_PVM

    //! For MPI or PVM, exit the communications interface.
    void exit();

    //@}

    //@{ \name Status and notify

    //! Return MPI rank or PVM taskid.
    int getMyTid();

  #if defined(HAVE_MPI)
    //! For MPI, return the number of processes.
    int getNumProcs();
  #endif

  #if defined(HAVE_PVM)
    //! For PVM, returns true if the process was spawned from the console.
    /*! Returns true if the process was spawned from the command line or the
        PVM console. Returns false if it was spawned from another PVM process. */
    bool isOrphan();

    //! For PVM, returns the taskid of the parent. */
    int parent();

    //! For PVM, returns the hostid of the given task. */
    int tidToHost(int tid);

    //! For PVM, sets notify to send a message.
    /*! Sets notify to send a message with the given msgtag with the task
        specified by taskid exits. The body of the message will contain
        the taskid. */
    void notify(int msgtag, int taskid);

    //! For PVM, sets notify to send a message.
    /*! Sets notify to send a message with the given msgtag whenever any host
        dies. The body of the message will contain the hostid. */
    void notify(int msgtag);
  #endif     //-- HAVE_PVM

    //@}

    //@{ \name Send, Recv, and Related Commands

    //! For MPI or PVM, initialize send (must be called before pack and send).
    void initSend();

    //! For MPI or PVM, send a specific message to a specific task.
    /*!
     *  @param msgtag - message ID agreed upon by sender and receiver
     *  @param taskid - destination processor number (nonnegative integer)
     */
    void send(int msgtag, int taskid);

    //! For MPI or PVM, blocking receive.
    /*!
     *  @param[in] msgtag  Message ID agreed upon by sender and receiver
     *  @param[in] taskid  Sender's processor number (nonnegative integer)
     *  @return  If successful, then 1 (MPI) or buffer ID of the message (PVM);
     *           if unsuccessful, then return 0.
     */
    bool recv(int msgtag = -1, int taskid = -1);

    //! For MPI or PVM, non-blocking probe.
    /*! Non-blocking probe for a message with the specified message tag
      and task id. Returns the buffer id of the message for PVM or 1
      for MPI, if any, and 0 otherwise. The inputs msgtag and msgtid
      default to -1 (wildcard).*/
    bool probe(int msgtag = -1, int taskid = -1);

    //! For MPI or PVM, find info for the most recent message.
    /*! Determines the msgtag and msgtid for the most recently probed
        or received message. */
    void bufinfo(int& msgtag, int& taskid);

  #if defined(HAVE_MPI)
    //! For MPI, blocking probe
    /*! Do a blocking probe for a message with tag msgtag and from
        process msgtid. The default values of msgtag and msgtid are both -1. */
    void bprobe(int msgtag = -1, int taskid = -1); 
  #endif

  #if defined(HAVE_PVM)
    //! For PVM, broadcast a message.
    /*! Broadcast the message stored in the buffer using tag msgtag.  Send it
        to the n tasks listed in the taskid array. */
    void broadcast (int msgtag, const vector<int>& taskid);

    //! For PVM, non-blocking receive.
    /*! Non-blocking receive for a message with the given tag and task
        id.  The inputs msgtag and msgtid default to -1 (wildcard).
        Returns the buffer id of the message, if any, and 0 otherwise.*/
    bool nrecv(int msgtag = -1, int taskid = -1);
  #endif

    //@}

    //@{ \name Pack and Unpack primitives.

    //! Pack an integer
    void pack(int i);

    //! Pack a char
    void pack(char i);

    //! Pack a double
    void pack(double d);

    //! Pack a float
    void pack(float f);

    //! Pack a bool
    void pack(bool b);

    //! Pack a string
    void pack(const string s);

    //! Pack integer vector
    void pack(const vector<int>& v);

    //! Pack double vector
    void pack(const Vector& v);

    //! Pack float vector
    void pack(const vector<float>& v);

    //! Pack a vector of vectors of doubles
    void pack(const vector< Vector >& v);

    //! Pack a vector of string
    void pack(const vector<string>& v);

    //! Pack a vector of bools
    void pack(const vector<bool>& v);

    //! Pack a char array
    void pack(int length, const char* array);

    //! Pack an integer array
    void pack(int length, const int* array);

    //! Pack a double array
    void pack(int length, const double* array);

    //! Pack a float array
    void pack(int length, const float* array);

    //! Unpack an integer
    void unpack(int& i);

    //! Unpack a char
    void unpack(char& c);

    //! Unpack a double
    void unpack(double& d);

    //! Unpack a float
    void unpack(float& f);

    //! Unpack a book
    void unpack(bool& b);

    //! Unpack a string
    void unpack(string& s);

    //! Unpack a vector of integers
    void unpack(vector<int>& v);

    //! Unpack a vector of doubles
    void unpack(Vector& v);

    //! Unpack a vector of a vector of doubles
    void unpack(vector< Vector >& v);

    //! Unpack a vector of strings
    void unpack(vector<string>& v);

    //! Unpack a vector of bools
    void unpack(vector<bool>& v);

    //! Unpack a char array
    void unpack(int& length, char*& array);

    //! Unpack an integer array
    void unpack(int& length, int*& array);

    //! Unpack a double array
    void unpack(int& length, double*& array);

    //! Unpack a float array
    void unpack(int& length, float*& array);

    //@}

    //@{ \name Host Information

  #if defined(HAVE_PVM)
    /*! \brief For PVM, reset host information by asking PVM to update
        hostp - the data structure that contains the host
        information. Return the number of hosts. */
    int resetHostInfo();

    //! For PVM. get the hostid of the i-th host
    int getHostTid(int i);

    //! For PVM,get the speed of the i-th host
    int getHostSpeed(int i);

    //! For PVM, get a pointer to name of the i-th host
    char* getHostName(int i);
  #endif     //-- HAVE_PVM

    //@}



private:

    //! Singleton implementation hides the constructor.
    GenProcComm (void);
    //! By design, there is no copy constructor.
    GenProcComm (const GenProcComm &);
    //! By design, there is no assignment operator.
    GenProcComm & operator= (const GenProcComm &);

    void pack(const char* s);
    void unpack(char*& s);

#if defined(HAVE_MPI)
    char* sendBuffer;
    char* recvBuffer;
    int sendPosition, recvPosition, msgSize;
    MPI_Comm HOPS_COMM;
    MPI_Status status;
    int mpiGetMsgSize(MPI_Status localStatus, MPI_Datatype dataType);
    void mpiStretchSendBuffer(int nBytes);
#endif

#if defined(HAVE_PVM)
    void pvmPackCheck(int info);
    void pvmUnpackCheck(int info);
    int lastBufferId;
    struct pvmhostinfo* pvmHostInfo;
    int nHosts;
    int nArch;
#endif

};

}

#endif     //-- HAVE_MPI || HAVE_PVM

#endif     //-- HOPSPACK_GENPROCCOMM_H
