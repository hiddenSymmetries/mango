subroutine mango_read_namelist(problem, filename)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem
  character(len=*), intent(in) :: filename

  integer :: fileUnit, didFileAccessWork
  character(len=100) :: algorithm

  namelist / mango_in / algorithm

  !---------------------------------------------

  print *,"Reading filename ",filename

  fileUnit=11
  open(unit=fileUnit, file=filename, action="read", status="old", iostat=didFileAccessWork)
  if (didFileAccessWork /= 0) then
     print *,"Error opening input file ", trim(filename)
     stop
  else
     read(fileUnit, nml=mango_in, iostat=didFileAccessWork)
     if (didFileAccessWork /= 0) then
        print *,"Error!  I was able to open the file ", trim(filename), &
             " but not read data from the mango_in namelist in it."
        if (didFileAccessWork==-1) then
           print *,"Make sure there is a carriage return after the / at the end of the namelist!"
        end if
        stop
     end if
     print *,"Successfully read parameters from mango_in namelist in ", trim(filename), "."
  end if
  close(unit = fileUnit)

  problem%algorithm = trim(algorithm)

end subroutine mango_read_namelist
