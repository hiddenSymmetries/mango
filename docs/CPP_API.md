# C++ API {#CPP_API}

To use MANGO in your C++ application, you only need to know the classes and other data structures
that are declared in mango.hpp. These are

* mango::package_type -- The available optimization packages.
* mango::algorithm_properties -- Properties of the available optimization algorithms.
* mango::algorithm_type -- The available optimization algorithms.
* mango::MPI_Partition -- A class for dividing up the available MPI processes into worker groups.
* mango::Problem -- A class for a standard optimization problem.
* mango::Least_squares_problem -- A class for a least-squares optimization problem.

There are other classes that are used internally by MANGO, which are described elsewhere in this documentation. 