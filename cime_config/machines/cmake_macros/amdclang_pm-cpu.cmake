if (COMP_NAME STREQUAL gptl)
  string(APPEND CPPDEFS " -DHAVE_NANOTIME -DBIT64 -DHAVE_SLASHPROC -DHAVE_GETTIMEOFDAY")
endif()
string(APPEND SLIBS " -L$ENV{CRAY_HDF5_PARALLEL_PREFIX}/lib -lhdf5_hl -lhdf5 -L$ENV{CRAY_NETCDF_HDF5PARALLEL_PREFIX} -L$ENV{CRAY_PARALLEL_NETCDF_PREFIX}/lib -lpnetcdf -lnetcdf -lnetcdff")
string(APPEND SLIBS " -lblas -llapack -lamdlibm")
if (NOT MPILIB STREQUAL mpi-serial)
  string(APPEND SLIBS " -L$ENV{ADIOS2_DIR}/lib64 -ladios2_c_mpi -ladios2_c -ladios2_core_mpi -ladios2_core -ladios2_evpath -ladios2_ffs -ladios2_dill -ladios2_atl -ladios2_enet")
endif()
set(NETCDF_PATH "$ENV{CRAY_NETCDF_HDF5PARALLEL_PREFIX}")
set(NETCDF_C_PATH "$ENV{CRAY_NETCDF_HDF5PARALLEL_PREFIX}")
set(NETCDF_FORTRAN_PATH "$ENV{CRAY_NETCDF_HDF5PARALLEL_PREFIX}")
set(HDF5_PATH "$ENV{CRAY_HDF5_PARALLEL_PREFIX}")
set(PNETCDF_PATH "$ENV{CRAY_PARALLEL_NETCDF_PREFIX}")
if (NOT DEBUG)
  string(APPEND CFLAGS " -O2 -g")
  string(APPEND CXXFLAGS " -O2 -g")
  string(APPEND FFLAGS " -O2")
endif()
#string(APPEND FFLAGS " -march=znver3")
set(SCC "clang")
set(SCXX "clang++")
set(SFC "flang")

if (COMP_NAME STREQUAL cism)
  string(APPEND CMAKE_OPTS " -D CISM_GNU=ON")
endif()

string(APPEND FC_AUTO_R8 " -fdefault-real-8")
string(APPEND FFLAGS " -Mflushz ")
string(APPEND FIXEDFLAGS " -Mfixed")
string(APPEND FREEFLAGS " -Mfreeform")
if (compile_threaded)
  string(APPEND FFLAGS " -mp")
  string(APPEND LDFLAGS " -mp")
endif()