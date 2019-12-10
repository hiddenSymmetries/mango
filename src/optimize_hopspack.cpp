#include<iostream>
#include<iomanip>
#include<stdexcept>
#include "mango.hpp"

#ifdef MANGO_HOPSPACK_AVAILABLE
#include "HOPSPACK_GenProcComm.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Vector.hpp"

int HOPSPACK_startProcComm(HOPSPACK::GenProcComm &, MPI_Comm);
int HOPSPACK_behaveAsMaster(HOPSPACK::GenProcComm &, HOPSPACK::ParameterList*, mango::problem*);
int HOPSPACK_behaveAsWorker(const int, HOPSPACK::GenProcComm &, mango::problem*);
#endif


void mango::problem::optimize_hopspack() {
#ifdef MANGO_HOPSPACK_AVAILABLE

  // Shorthand:
  MPI_Comm mpi_comm_group_leaders = mpi_partition.get_comm_group_leaders();
  int mpi_rank_group_leaders = mpi_partition.get_rank_group_leaders();
  bool proc0 = (mpi_rank_group_leaders == 0);

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
  if (proc0) {
    HOPSPACK::ParameterList*  cProblemParams = &(hopspack_parameters.getOrSetSublist ("Problem Definition"));
    cProblemParams->setParameter("Objective Type","Minimize");
    cProblemParams->setParameter("Number Unknowns", N_parameters);
    HOPSPACK::Vector scaling(N_parameters, 1.0);
    cProblemParams->setParameter("Scaling",scaling);
    // Set the initial condition:
    HOPSPACK::Vector x0(N_parameters, 0.0);
    for (int j=0; j<N_parameters; j++) x0[j] = state_vector[j];
    cProblemParams->setParameter("Initial X",x0);
        
    HOPSPACK::ParameterList*  cMedParams = &(hopspack_parameters.getOrSetSublist ("Mediator"));
    //cout << "MJL Number Processors:" << cMedParams->getParameter("Number Processors",-17) << endl;
    cMedParams->setParameter("Number Processors",mpi_partition.get_N_procs_group_leaders());
    cMedParams->setParameter("Citizen Count",1);
    cMedParams->setParameter("Maximum Evaluations",max_function_evaluations);
    
    HOPSPACK::ParameterList*  cCitizenParams = &(hopspack_parameters.getOrSetSublist ("Citizen 1"));
    cCitizenParams->setParameter("Type","GSS");
    cCitizenParams->setParameter("Step Tolerance",1e-5);

    if (verbose > 0) {
      cProblemParams->setParameter("Display",2);
      cMedParams->setParameter("Display",2);
      cCitizenParams->setParameter("Display",1);
    } else {
      cProblemParams->setParameter("Display",0);
      cMedParams->setParameter("Display",0);
      cCitizenParams->setParameter("Display",0);
    }
    
    if (verbose > 0) std::cout << "Done setting HOPSPACK parameters.\n";
  }

  // Run HOPSPACK
  int nReturnValue;
  if (proc0) {
    nReturnValue = HOPSPACK_behaveAsMaster(cGPC, &hopspack_parameters, this);
  } else {
    nReturnValue = HOPSPACK_behaveAsWorker(nProcRank, cGPC, this);
  }

  if (nReturnValue != 0) throw std::runtime_error("Error! HOPSPACK returned an error code.");

  // How do I get the global function evaluation index of the best function eval??
  int proc_index, best_proc_index;
  double candidate_best_objective_function, apparent_best_objective_function;
  bool was_I_best;
  if (proc0) {
    // Poll all other procs for their best objective function
    for (proc_index = 1; proc_index < mpi_partition.get_N_procs_group_leaders(); proc_index++) {
      MPI_Recv(&candidate_best_objective_function, 1, MPI_DOUBLE, proc_index, proc_index, mpi_comm_group_leaders, MPI_STATUS_IGNORE);
      //std::cout << "Best objective function on group leader " << proc_index << ": " << std::scientific << candidate_best_objective_function << std::endl;
      if (proc_index==1 || (candidate_best_objective_function < apparent_best_objective_function)) {
	apparent_best_objective_function = candidate_best_objective_function;
	best_proc_index = proc_index;
      }
    }
    //std::cout << "Best_proc_index: " << best_proc_index << std::endl;

    // Verify apparent_best_objective_function here coincides with the best_objective_function we found in write_hopspack_line_to_file:
    if (fabs(apparent_best_objective_function - best_objective_function) > 1e-30) {
      std::cerr << "WARNING!!! Significant difference between apparent_best_objective_function and best_objective_function." << std::endl;
      std::cerr << "apparent_best_objective_function: " << std::scientific << apparent_best_objective_function << "  best_objective_function: " << best_objective_function
		<< "  diff: " << apparent_best_objective_function - best_objective_function << std::endl;
    }

    // Tell each proc whether they were the one that acheieved the best objective function
    MPI_Bcast(&best_proc_index, 1, MPI_INT, 0, mpi_comm_group_leaders);

    // Receive details of the optimum from the appropriate process:
    MPI_Recv(best_state_vector, N_parameters, MPI_DOUBLE, best_proc_index, best_proc_index, mpi_comm_group_leaders, MPI_STATUS_IGNORE);
    if (least_squares) MPI_Recv(best_residual_function, N_terms, MPI_DOUBLE, best_proc_index, best_proc_index, mpi_comm_group_leaders, MPI_STATUS_IGNORE);

  } else {
    // Handle procs other than 0.

    // Tell proc0 the best objective function achieved on this proc.
    MPI_Send(&best_objective_function, 1, MPI_DOUBLE, 0, mpi_rank_group_leaders, mpi_comm_group_leaders);
    // Hear from proc0 which proc achieved the best objective function:
    MPI_Bcast(&best_proc_index, 1, MPI_INT, 0, mpi_comm_group_leaders);
    // If it was me, then send details of the optimum
    if (mpi_rank_group_leaders == best_proc_index) {
      MPI_Send(best_state_vector, N_parameters, MPI_DOUBLE, 0, best_proc_index, mpi_comm_group_leaders);
      if (least_squares) MPI_Send(best_residual_function, N_terms, MPI_DOUBLE, 0, best_proc_index, mpi_comm_group_leaders);
    }
  }


#else
  // MANGO_HOPSPACK_AVAILABLE is not defined
  throw std::runtime_error("Error! The HOPSPACK algorithm was requested, but Mango was compiled without HOPSPACK support.");
#endif  
}

