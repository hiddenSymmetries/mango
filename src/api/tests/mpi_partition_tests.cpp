#include "catch.hpp"
#include "mango.hpp"

// Should I be using 'Generators' instead of putting CHECK's inside loops?
// https://www.reddit.com/r/cpp/comments/a6bdee/just_found_catch2_c_unit_test_framework_supports/

/*
TEST_CASE("MPI_Partition.set_custom(): Verify that an exception is thrown if input communicators supplied do not make sense.","[mpi_partition]") {
  mango::MPI_Partition mp;
  CHECK_THROWS(mp.set_custom(MPI_COMM_WORLD, MPI_COMM_WORLD, MPI_COMM_WORLD));
}
*/


TEST_CASE("MPI_Partition: Verify that calling getters before initialization causes exceptions","[mpi_partition]") {
  mango::MPI_Partition mp;
  CHECK_THROWS(mp.get_comm_world());
  CHECK_THROWS(mp.get_comm_worker_groups());
  CHECK_THROWS(mp.get_comm_group_leaders());

  CHECK_THROWS(mp.get_proc0_world());
  CHECK_THROWS(mp.get_proc0_worker_groups());

  CHECK_THROWS(mp.get_rank_world());
  CHECK_THROWS(mp.get_rank_worker_groups());
  CHECK_THROWS(mp.get_rank_group_leaders());

  CHECK_THROWS(mp.get_N_procs_world());
  CHECK_THROWS(mp.get_N_procs_worker_groups());
  CHECK_THROWS(mp.get_N_procs_group_leaders());
}


TEST_CASE("MPI_Partition.init(): Verify that all the properties make sense when N_worker_groups=1.","[mpi_partition]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  mango::MPI_Partition mp;
  mp.set_N_worker_groups(1); // There is only 1 worker group, so only 1 group leader
  mp.init(MPI_COMM_WORLD);
  
  CHECK(mp.get_N_worker_groups() == 1);
  
  CHECK(mp.get_rank_world() == rank_world);
  CHECK(mp.get_rank_worker_groups() == rank_world);
  CHECK(mp.get_rank_group_leaders() == (rank_world==0 ? 0 : -1));
  
  CHECK(mp.get_N_procs_world() == N_procs_world);
  CHECK(mp.get_N_procs_worker_groups() == N_procs_world);
  CHECK(mp.get_N_procs_group_leaders() == (rank_world==0 ? 1 : -1));
  
  CHECK(mp.get_proc0_world() == (rank_world==0));
  CHECK(mp.get_proc0_worker_groups() == (rank_world==0));
}


TEST_CASE("MPI_Partition.init(): Verify that all the properties make sense when N_worker_groups=N_procs_world or more, or when N_worker_groups <= 0.","[mpi_partition]") {
  // When N_worker_groups is <= 0, MPI_Partition.init() should set N_worker_groups equal to the number of available processors.

  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  //for (int mode = 0; mode < 2; mode++) {
  //for (int shift = 0; shift<5; shift++) {
  auto mode = GENERATE(range(0,1));
  auto shift = GENERATE(range(0,4));
      int N_worker_groups_requested;
      if (mode==0) {
	N_worker_groups_requested = N_procs_world + shift;
      } else {
	N_worker_groups_requested = -shift; // so N_worker_groups is <= 0.
      }
      mango::MPI_Partition mp;
      mp.set_N_worker_groups(N_worker_groups_requested);
      mp.init(MPI_COMM_WORLD);
      
      CAPTURE(rank_world, N_worker_groups_requested);

      CHECK(mp.get_N_worker_groups() == N_procs_world);
      
      CHECK(mp.get_rank_world() == rank_world);
      CHECK(mp.get_rank_worker_groups() == 0);
      CHECK(mp.get_rank_group_leaders() == rank_world);
      
      CHECK(mp.get_N_procs_world() == N_procs_world);
      CHECK(mp.get_N_procs_worker_groups() == 1);
      CHECK(mp.get_N_procs_group_leaders() == N_procs_world);
      
      CHECK(mp.get_proc0_world() == (rank_world==0));
      CHECK(mp.get_proc0_worker_groups());
      //}
      //}
}


TEST_CASE("MPI_Partition.init(): Verify that for any choice of N_worker_groups, parameters are within the expected ranges.","[mpi_partition]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  // Try values of N_worker_groups somewhat outside the range [1, N_procs_world] too, just to make sure these cases are handled correctly:
  for (int N_worker_groups_requested = -1; N_worker_groups_requested <= N_procs_world + 2; N_worker_groups_requested++) {
    mango::MPI_Partition mp;
    mp.set_N_worker_groups(N_worker_groups_requested);
    mp.init(MPI_COMM_WORLD);

    CAPTURE(rank_world, N_worker_groups_requested);

    CHECK(mp.get_N_worker_groups() >= 1);
    CHECK(mp.get_N_worker_groups() <= N_procs_world);
    
    CHECK(mp.get_rank_world() == rank_world);
    CHECK(mp.get_rank_worker_groups() >= 0);
    CHECK(mp.get_rank_worker_groups() < N_procs_world);
    
    CHECK(mp.get_N_procs_world() == N_procs_world);
    CHECK(mp.get_N_procs_worker_groups() >= 1);
    CHECK(mp.get_N_procs_worker_groups() <= N_procs_world);
    
    CHECK(mp.get_proc0_world() == (rank_world==0));
    
    bool proc0_worker_groups = mp.get_proc0_worker_groups();
    if (proc0_worker_groups) {
      CHECK(mp.get_rank_group_leaders() >= 0);
      CHECK(mp.get_rank_group_leaders() < N_procs_world);
      CHECK(mp.get_N_procs_group_leaders() >= 1);
      CHECK(mp.get_N_procs_group_leaders() <= N_procs_world);
    } else {
      CHECK(mp.get_rank_group_leaders() == -1);
      CHECK(mp.get_N_procs_group_leaders() == -1);
    }

    // The sizes of the worker groups should be relatively even, with a difference of no more than 1 between the largest and the smallest.
    int N_procs_worker_groups_max = mp.get_N_procs_worker_groups();
    int N_procs_worker_groups_min = mp.get_N_procs_worker_groups();
    if (rank_world==0) {
      MPI_Reduce(MPI_IN_PLACE, &N_procs_worker_groups_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(MPI_IN_PLACE, &N_procs_worker_groups_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
      CHECK(N_procs_worker_groups_max - N_procs_worker_groups_min <= 1);
      CHECK(N_procs_worker_groups_max - N_procs_worker_groups_min >= 0);
    } else {
      MPI_Reduce(&N_procs_worker_groups_max, NULL, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(&N_procs_worker_groups_min, NULL, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    }
  }
}


TEST_CASE("MPI_Partition.set_custom(): Verify that parameters are correct for the case in which every proc is a group leader.","[mpi_partition]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  mango::MPI_Partition mp;
  mp.set_custom(MPI_COMM_WORLD, MPI_COMM_WORLD, MPI_COMM_SELF);

  CAPTURE(rank_world);
  CHECK(mp.get_N_worker_groups() == N_procs_world);
      
  CHECK(mp.get_rank_world() == rank_world);
  CHECK(mp.get_rank_worker_groups() == 0);
  CHECK(mp.get_rank_group_leaders() == rank_world);
      
  CHECK(mp.get_N_procs_world() == N_procs_world);
  CHECK(mp.get_N_procs_worker_groups() == 1);
  CHECK(mp.get_N_procs_group_leaders() == N_procs_world);
      
  CHECK(mp.get_proc0_world() == (rank_world==0));
  CHECK(mp.get_proc0_worker_groups());
}


TEST_CASE("MPI_Partition.set_custom(): Verify that parameters are correct for the case in which there is a single worker group.","[mpi_partition]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  int color;
  if (rank_world==0) {
    color = 0;
  } else {
    color = MPI_UNDEFINED;
  }
  MPI_Comm mpi_comm_proc0;
  MPI_Comm_split(MPI_COMM_WORLD, color, rank_world, &mpi_comm_proc0);  // key = rank_world. The key doesn't really matter here.

  mango::MPI_Partition mp;
  mp.set_custom(MPI_COMM_WORLD, mpi_comm_proc0, MPI_COMM_WORLD);

  CAPTURE(rank_world);
  CHECK(mp.get_N_worker_groups() == 1);

  CHECK(mp.get_rank_world() == rank_world);
  CHECK(mp.get_rank_worker_groups() == rank_world);
  CHECK(mp.get_rank_group_leaders() == (rank_world==0 ? 0 : -1));
  
  CHECK(mp.get_N_procs_world() == N_procs_world);
  CHECK(mp.get_N_procs_worker_groups() == N_procs_world);
  CHECK(mp.get_N_procs_group_leaders() == (rank_world==0 ? 1 : -1));
  
  CHECK(mp.get_proc0_world() == (rank_world==0));
  CHECK(mp.get_proc0_worker_groups() == (rank_world==0));
}


TEST_CASE("MPI_Partition.set_custom(): Verify that for any choice of N_worker_groups, if we generate communicators using MPI_Partition.init() and supply them as inputs to set_custom(), the results of set_custom() are identical to init().","[mpi_partition]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);
  //CAPTURE(N_procs_world,rank_world);

  for (int N_worker_groups_requested = 1; N_worker_groups_requested <= N_procs_world; N_worker_groups_requested++) {
  //auto N_worker_groups_requested = GENERATE( range(1, 5) ); // Really we need to only go up to N_procs_world rather than 5, but if I try this with GENERATE_COPY in catch2, I get an abort signal.
    mango::MPI_Partition mp_init, mp_custom;
    mp_init.set_N_worker_groups(N_worker_groups_requested);
    mp_init.init(MPI_COMM_WORLD);
    mp_custom.set_custom(MPI_COMM_WORLD, mp_init.get_comm_group_leaders(), mp_init.get_comm_worker_groups());

    CAPTURE(rank_world);
    CHECK(mp_init.get_proc0_world() == mp_custom.get_proc0_world());
    CHECK(mp_init.get_proc0_worker_groups() == mp_custom.get_proc0_worker_groups());

    CHECK(mp_init.get_rank_world() == mp_custom.get_rank_world());
    CHECK(mp_init.get_rank_worker_groups() == mp_custom.get_rank_worker_groups());
    CHECK(mp_init.get_rank_group_leaders() == mp_custom.get_rank_group_leaders());

    CHECK(mp_init.get_N_procs_world() == mp_custom.get_N_procs_world());
    CHECK(mp_init.get_N_procs_worker_groups() == mp_custom.get_N_procs_worker_groups());
    CHECK(mp_init.get_N_procs_group_leaders() == mp_custom.get_N_procs_group_leaders());

    CHECK(mp_init.get_worker_group() == mp_custom.get_worker_group());
    CHECK(mp_init.get_N_worker_groups() == mp_custom.get_N_worker_groups());
    CHECK(mp_init.get_N_worker_groups() == N_worker_groups_requested);
    }
}


/*
TEST_CASE("minimal example") {
  int N;
  MPI_Comm_size(MPI_COMM_WORLD, &N);
  //  int N=4;
  auto j = GENERATE_REF(range(1,N));
  CHECK(j > 0);
}
*/

/*
TEST_CASE("minimal example 2") {
  int N;
  MPI_Comm_size(MPI_COMM_WORLD, &N);
  for (int j = 1; j <= N; j++) {
    CHECK(j > 0);
  }
}
*/
