#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "mpi.h"


// main() taken from https://stackoverflow.com/questions/58289895/is-it-possible-to-use-catch2-for-testing-an-mpi-code 
int main( int argc, char* argv[] ) {
  MPI_Init(&argc, &argv);
  int result = Catch::Session().run( argc, argv );
  MPI_Finalize();
  return result;
}
