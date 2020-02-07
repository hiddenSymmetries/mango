// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#ifndef MANGO_PACKAGE_HOPSPACK_H
#define MANGO_PACKAGE_HOPSPACK_H

#ifdef MANGO_HOPSPACK_AVAILABLE
#endif

namespace mango {

  class Solver;
  class Least_squares_solver;

  class Package_hopspack : public Package {
  private:
    Solver* solver;

#ifdef MANGO_HOPSPACK_AVAILABLE
#endif

  public:
    void optimize(Solver*);
    void optimize_least_squares(Least_squares_solver*);
  };
}

#endif
