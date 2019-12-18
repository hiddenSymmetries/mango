#include "catch.hpp"
#include "mango.hpp"


TEST_CASE("MPI_Partition: Verify get before initialized causes exceptions","[mpi_partition]") {
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


TEST_CASE("MPI_Partition: Verify that all the properties make sense when N_worker_groups=1.","[mpi_partition][mpi]") {
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


TEST_CASE("MPI_Partition: Verify that all the properties make sense when N_worker_groups=N_procs_world.","[mpi_partition][mpi]") {
  int rank_world, N_procs_world;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs_world);

  mango::MPI_Partition mp;
  mp.set_N_worker_groups(N_procs_world); // Every proc is a worker "group" of 1.
  mp.init(MPI_COMM_WORLD);

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
