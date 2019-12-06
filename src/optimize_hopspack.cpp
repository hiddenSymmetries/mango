#include<iostream>
#include<stdexcept>
#include "mango.hpp"

#ifdef MANGO_HOPSPACK_AVAILABLE
#include "HOPSPACK_GenProcComm.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Vector.hpp"

int HOPSPACK_startProcComm(HOPSPACK::GenProcComm &, MPI_Comm);
int HOPSPACK_behaveAsMaster(HOPSPACK::GenProcComm &, HOPSPACK::ParameterList*);
int HOPSPACK_behaveAsWorker(const int, HOPSPACK::GenProcComm &);
#endif


void mango::problem::optimize_hopspack() {
#ifdef MANGO_HOPSPACK_AVAILABLE

  if (verbose > 0) std::cout << "Entering optimize_hopspack\n";

  if (mpi_partition.get_N_procs_group_leaders() < 2) throw std::runtime_error("Hopspack requires at least 2 worker groups");

  // Initialize HOPSPACK's MPI machinery. This code is based on code in HOPSPACK_main_mpi.cpp.
  using HOPSPACK::GenProcComm;
  GenProcComm &  cGPC = GenProcComm::getTheInstance();
  int  nProcRank = HOPSPACK_startProcComm(cGPC, mpi_partition.get_comm_group_leaders());
  if (nProcRank == -1) throw std::runtime_error("Error starting MPI in HOPSPACK");

  if (verbose > 0) std::cout << "Done initializing HOPSPACK MPI variables.\n";

  // Set HOPSPACK's parameters:
  // See the HOPSPACK manual for the paramter sublists and parameters.
  using HOPSPACK::ParameterList;
  ParameterList hopspack_parameters;
  if (nProcRank == 0) {
    HOPSPACK::ParameterList*  cProblemParams = &(hopspack_parameters.getOrSetSublist ("Problem Definition"));
    cProblemParams->setParameter("Objective Type","Minimize");
    cProblemParams->setParameter("Number Unknowns", N_parameters);
    HOPSPACK::Vector scaling(N_parameters, 1.0);
    cProblemParams->setParameter("Scaling",scaling);
    // Set the initial condition:
    HOPSPACK::Vector x0(N_parameters, 0.0);
    for (int j=0; j<N_parameters; j++) x0[j] = state_vector[j];
    cProblemParams->setParameter("Initial X",x0);
    
    cProblemParams->setParameter("Display",2);
    
    HOPSPACK::ParameterList*  cMedParams = &(hopspack_parameters.getOrSetSublist ("Mediator"));
    //cout << "MJL Number Processors:" << cMedParams->getParameter("Number Processors",-17) << endl;
    cMedParams->setParameter("Number Processors",mpi_partition.get_N_procs_group_leaders());
    cMedParams->setParameter("Citizen Count",1);
    cMedParams->setParameter("Display",2);
    
    HOPSPACK::ParameterList*  cCitizenParams = &(hopspack_parameters.getOrSetSublist ("Citizen 1"));
    cCitizenParams->setParameter("Type","GSS");
    cCitizenParams->setParameter("Display",1);
    cCitizenParams->setParameter("Step Tolerance",1e-5);
    
    if (verbose > 0) std::cout << "Done setting HOPSPACK parameters.\n";
  }

  // Run HOPSPACK
  int nReturnValue;
  if (nProcRank == 0) {
    nReturnValue = HOPSPACK_behaveAsMaster(cGPC, &hopspack_parameters);
  } else {
    nReturnValue = HOPSPACK_behaveAsWorker(nProcRank, cGPC);
  }

  if (nReturnValue != 0) throw std::runtime_error("Error! HOPSPACK returned an error code.");

#else
  // MANGO_HOPSPACK_AVAILABLE is not defined
  throw std::runtime_error("Error! The HOPSPACK algorithm was requested, but Mango was compiled without HOPSPACK support.");
#endif  
}
