#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include "mango.hpp"
#include "Solver.hpp"
#include "Package_hopspack.hpp"

#ifdef MANGO_HOPSPACK_AVAILABLE
#include "HOPSPACK_GenProcComm.hpp"
#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_Vector.hpp"
#include "HOPSPACK_ExecutorSerial.hpp"
#include "HOPSPACK_Hopspack.hpp"
#include "HOPSPACK_MangoEvaluator.hpp"

int HOPSPACK_startProcComm(HOPSPACK::GenProcComm &, MPI_Comm);
int HOPSPACK_behaveAsMaster(HOPSPACK::GenProcComm &, HOPSPACK::ParameterList*, mango::Solver*);
int HOPSPACK_behaveAsWorker(const int, HOPSPACK::GenProcComm &, mango::Solver*);
#endif


void mango::Package_hopspack::optimize(Solver* solver) {
#ifdef MANGO_HOPSPACK_AVAILABLE

  // Shorthand:
  MPI_Comm mpi_comm_group_leaders = solver->mpi_partition->get_comm_group_leaders();
  int mpi_rank_group_leaders = solver->mpi_partition->get_rank_group_leaders();
  bool proc0 = (mpi_rank_group_leaders == 0);
  int N_parameters = solver->N_parameters;

  if (solver->verbose > 0) std::cout << "Entering optimize_hopspack" << std::endl;

  //  if (mpi_partition.get_N_procs_group_leaders() < 2) throw std::runtime_error("Hopspack requires at least 2 worker groups");

  // Initialize HOPSPACK's MPI machinery. This code is based on code in HOPSPACK_main_mpi.cpp.
  using HOPSPACK::GenProcComm;
  GenProcComm &  cGPC = GenProcComm::getTheInstance();
  int  nProcRank = HOPSPACK_startProcComm(cGPC, solver->mpi_partition->get_comm_group_leaders());
  if (nProcRank == -1) throw std::runtime_error("Error starting MPI in HOPSPACK");

  if (solver->verbose > 0) std::cout << "Done initializing HOPSPACK MPI variables." << std::endl;

  // Set HOPSPACK's parameters:
  // See the HOPSPACK manual for the paramter sublists and parameters.
  using HOPSPACK::ParameterList;
  ParameterList hopspack_parameters;
  if (proc0) {
    HOPSPACK::ParameterList*  cProblemParams = &(hopspack_parameters.getOrSetSublist ("Problem Definition"));
    cProblemParams->setParameter("Objective Type","Minimize");
    cProblemParams->setParameter("Number Unknowns", N_parameters);

    // If bound constraints are available, hopefully hopspack can automatically set the scaling. Otherwise just try a uniform scaling.
    if (!solver->bound_constraints_set) {
      HOPSPACK::Vector scaling(N_parameters, 1.0);
      cProblemParams->setParameter("Scaling",scaling);
    }

    // Set the initial condition:
    HOPSPACK::Vector x0(N_parameters, 0.0);
    for (int j=0; j<N_parameters; j++) x0[j] = solver->state_vector[j];
    cProblemParams->setParameter("Initial X",x0);

    // Set bound constraints, if they are available:
    if (solver->bound_constraints_set) {
      HOPSPACK::Vector hopspack_lower_bounds(N_parameters, 0.0);
      HOPSPACK::Vector hopspack_upper_bounds(N_parameters, 0.0);
      // Eventually I may want to replace large values with HOPSPACK::dne() ?
      for (int j=0; j<N_parameters; j++) hopspack_lower_bounds[j] = solver->lower_bounds[j];
      for (int j=0; j<N_parameters; j++) hopspack_upper_bounds[j] = solver->upper_bounds[j];
      cProblemParams->setParameter("Lower Bounds",hopspack_lower_bounds);
      cProblemParams->setParameter("Upper Bounds",hopspack_upper_bounds);
    }
        
    HOPSPACK::ParameterList*  cMedParams = &(hopspack_parameters.getOrSetSublist ("Mediator"));
    //cout << "MJL Number Processors:" << cMedParams->getParameter("Number Processors",-17) << endl;
    cMedParams->setParameter("Number Processors",solver->mpi_partition->get_N_procs_group_leaders());
    cMedParams->setParameter("Citizen Count",1);
    cMedParams->setParameter("Maximum Evaluations",solver->max_function_evaluations);
    
    HOPSPACK::ParameterList*  cCitizenParams = &(hopspack_parameters.getOrSetSublist ("Citizen 1"));
    cCitizenParams->setParameter("Type","GSS");
    cCitizenParams->setParameter("Step Tolerance",1e-5);

    if (solver->verbose > 0) {
      cProblemParams->setParameter("Display",2);
      cMedParams->setParameter("Display",2);
      cCitizenParams->setParameter("Display",1);
    } else {
      cProblemParams->setParameter("Display",0);
      cMedParams->setParameter("Display",0);
      cCitizenParams->setParameter("Display",0);
    }
    
    if (solver->verbose > 0) std::cout << "Done setting HOPSPACK parameters." << std::endl;
  }

  // Run HOPSPACK
  int nReturnValue;
  if (solver->mpi_partition->get_N_worker_groups()==1) {
    // Run serial version of hopspack.
    // The code for this is based on HOPSPACK_main_serial.cpp.
    if (solver->verbose > 0) std::cout << "Beginning serial version of HOPSPACK." << std::endl;
    //HOPSPACK::MangoEvaluator* pEvaluator = HOPSPACK::MangoEvaluator::newInstance (hopspack_parameters.sublist ("Evaluator"));
    HOPSPACK::MangoEvaluator* pEvaluator = new HOPSPACK::MangoEvaluator (hopspack_parameters, solver);
    if (pEvaluator == NULL) throw std::runtime_error("Error constructing MangoEvaluator for serial HOPSPACK!");
    HOPSPACK::ExecutorSerial* pExecutor = new HOPSPACK::ExecutorSerial (pEvaluator);
    HOPSPACK::Hopspack optimizer (pExecutor);
    if (optimizer.setInputParameters (hopspack_parameters, solver) == true) optimizer.solve();
    delete pEvaluator;
    delete pExecutor;
    nReturnValue = 0;

  } else {
    // Run MPI version of hopspack
    if (proc0) {
      if (solver->verbose > 0) std::cout << "Beginning MPI version of HOPSPACK." << std::endl;
      nReturnValue = HOPSPACK_behaveAsMaster(cGPC, &hopspack_parameters, solver);
    } else {
      nReturnValue = HOPSPACK_behaveAsWorker(nProcRank, cGPC, solver);
    }
  }

  if (nReturnValue != 0) throw std::runtime_error("Error! HOPSPACK returned an error code.");

  /* 20200131 I'll think about what to do with this next bit of code later.
  // If we ran the MPI version of hopspack, here we get the data from the optimum transferred to proc 0.
  int proc_index, best_proc_index;
  double candidate_best_objective_function, apparent_best_objective_function;
  bool was_I_best;
  if (solver->mpi_partition->get_N_worker_groups() > 1) {
    if (proc0) {
      // Poll all other procs for their best objective function
      for (proc_index = 1; proc_index < solver->mpi_partition->get_N_procs_group_leaders(); proc_index++) {
	MPI_Recv(&candidate_best_objective_function, 1, MPI_DOUBLE, proc_index, proc_index, mpi_comm_group_leaders, MPI_STATUS_IGNORE);
	//std::cout << "Best objective function on group leader " << proc_index << ": " << std::scientific << candidate_best_objective_function << std::endl;
	if (proc_index==1 || (candidate_best_objective_function < apparent_best_objective_function)) {
	  apparent_best_objective_function = candidate_best_objective_function;
	  best_proc_index = proc_index;
	}
      }
      //std::cout << "Best_proc_index: " << best_proc_index << std::endl;
      
      // Verify apparent_best_objective_function here coincides with the best_objective_function we found in write_hopspack_line_to_file:
      if (std::fabs(apparent_best_objective_function - best_objective_function) > 1e-30) {
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
  }
  // Done transferring data about the optimum to proc 0.
  */

#else
  // MANGO_HOPSPACK_AVAILABLE is not defined
  throw std::runtime_error("Error! The HOPSPACK algorithm was requested, but Mango was compiled without HOPSPACK support.");
#endif  
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*
void mango::problem::write_hopspack_line_to_file(std::string line, double objective_function) {
  // This subroutine only ever is called on proc0_world.
  // 'line' is almost all of the line of the output file, except that the global # of function evaluations (the first element of the line)
  // is missing. This is because the line was generated on a proc >0, but only proc 0 knows the global # of function evaluations.

  clock_t now = clock();

  // In the case N_worker_groups=1 (i.e. a serial hopspack run), function_evaluations has already been incremented on this proc (0) 
  // in objective_function_wrapper() or residual_function_wrapper().
  if (mpi_partition.get_N_worker_groups() > 1) function_evaluations += 1; // This line is how proc 0 keeps track of the total number of function evaluations.

  //std::cout << "write_hopspack_line_to_file: function_evaluations is now " << function_evaluations << ", now=" << now << ", f=" << std::scientific << objective_function;

  // This next line is how proc0_world keeps track of which function evaluation was the optimum.
  if (function_evaluations == 1 || objective_function < best_objective_function) {
    best_function_evaluation = function_evaluations;
    best_objective_function = objective_function;
    best_time = now;
  }
  // In a serial hopspack run (i.e. N_worker_groups=1), best_time was already recorded in objective_function_wrapper() or residual_function_wrapper(),
  // and the 'if' block above did not executre, since objective_function is no longer < best_objective_function.
  // For consistency, then, we should use that time, not the time measured above in this subroutine.
  if (best_function_evaluation == function_evaluations) now = best_time;

  // Now actually write the line of the file.
  write_function_evaluation_and_time(now);
  output_file << line << std::endl << std::flush;
}
*/

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void mango::Package_hopspack::optimize_least_squares(Least_squares_solver* solver) {
  throw std::runtime_error("Error! mango somehow got to Package_hopspack::optimize_least_squares. This should never happen.");
}
