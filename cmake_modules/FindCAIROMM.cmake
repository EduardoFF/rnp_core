
# - Try to find ImageMagick++
# Once done, this will define
#
#  Cairomm_FOUND - system has Cairomm-1.0
#  Cairomm_INCLUDE_DIRS - the Cairom includes
#  Cairomm_LIBRARIES - link these to use Cairomm

include(LibFindMacros)

# Dependencies
#libfind_package(Cairomm Cairo)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Cairomm_PKGCONF cairomm-1.0)

# Include dir
find_path(Cairomm_INCLUDE_DIR
  NAMES cairomm/cairomm.h
  PATHS ${Cairomm_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Cairomm_LIBRARY
  NAMES cairomm-1.0
  PATHS ${Cairomm_PKGCONF_LIBRARY_DIRS}
)


# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Cairomm_PROCESS_INCLUDES Cairomm_INCLUDE_DIR Cairomm_INCLUDE_DIRS)
set(Cairomm_PROCESS_LIBS Cairomm_LIBRARY Cairomm_LIBRARIES)
libfind_process(cairomm)


MESSAGE( STATUS "${Cairomm_INCLUDE_DIR}  ${Cairomm_LIBRARY}")