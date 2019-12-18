#include "catch.hpp"
#include "mango.hpp"


TEST_CASE("In MPI_Partition, verify get before initialized causes exceptions","[mpi_partition]") {
  mango::MPI_Partition mpi_partition;
  CHECK_THROWS(mpi_partition.get_comm_world());
  CHECK_THROWS(mpi_partition.get_comm_worker_groups());
}
