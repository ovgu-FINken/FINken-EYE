######################################################################################################################
#
# JeVois Smart Embedded Machine Vision Toolkit - Copyright (C) 2016 by Laurent Itti, the University of Southern
# California (USC), and iLab at USC. See http://iLab.usc.edu and http://jevois.org for information about this project.
#
# This file is part of the JeVois Smart Embedded Machine Vision Toolkit.  This program is free software; you can
# redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
# Foundation, version 2.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
# License for more details.  You should have received a copy of the GNU General Public License along with this program;
# if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# Contact information: Laurent Itti - 3641 Watt Way, HNB-07A - Los Angeles, BA 90089-2520 - USA.
# Tel: +1 213 740 3527 - itti@pollux.usc.edu - http://iLab.usc.edu - http://jevois.org
######################################################################################################################

## CMake build rules for JeVois Base library and executables

## You may provide the installed JeVois config root as:
#   cmake -DJEVOIS_CONFIG=/jevois/config ..

cmake_minimum_required(VERSION 3.1)

## Set vendor names, our modules will be placed in a directory by that name under /jevois/modules:
set(JEVOIS_VENDOR "FINken_Nano")
set(JEVOIS_CONFIG "/jevois/config" CACHE STRING "Path to JeVois config to use")

## Include helper functions, config, etc from the JeVois install:
set(CMAKE_MODULE_PATH ${JEVOIS_CONFIG})
include(jevois_config)
include(JeVois)

## Set project name, detects compiler (which has been set by our helper module). Then set some complation flags:
project(svo_no_ros)
jevois_project_set_flags()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

## If your module will use components provided by jevoisbase, uncomment the lines below:
if (JEVOIS_PLATFORM)
  link_directories("/var/lib/jevois-microsd/lib/JeVois") # find libjevoisbase.so for platform
endif (JEVOIS_PLATFORM)
include_directories("/var/lib/jevois-build/usr/include") # find jevoisbase includes

## If you also want to use some of the jevoisbase contribs, just add the appropriate include paths here.  You will also
## need to set JEVOIS_SRC_ROOT environment variable in your shell to the parent of the jevoisbase directory, e.g., type:
#   export JEVOIS_SRC_ROOT=/home/myname
## in your shell.
#include_directories("$ENV{JEVOIS_SRC_ROOT}/jevoisbase/Contrib/darknet-nnpack/include")
#include_directories("$ENV{JEVOIS_SRC_ROOT}/jevoisbase/Contrib/NNPACK/include")
#include_directories("$ENV{JEVOIS_SRC_ROOT}/jevoisbase/Contrib/pthreadpool/include")

## Setup our modules that are in src/Modules. First arg: source directory for modules; 2nd arg: target build
## dependencies (i.e., cmake targets which must be built here before we build the modules), usually empty for a single
## module. See the CMakeLists.txt in jevoisbase for an example where we first build a shared library for all shared
## components, and then each module gets this library as a target dependency:
jevois_setup_modules(src/Modules "")

## Include Eigen3
#find_package( Eigen3 REQUIRED )
#INCLUDE_DIRECTORIES( ${EIGEN3_INCLUDE_DIR} )

## Include Eigen3 headers
#include_directories(eigen)

## Include and build subprojects
add_subdirectory(Sophus)
add_subdirectory(fast)
add_subdirectory(vikit_common)
add_subdirectory(svo)

## Include necessary libraries
include_directories("fast/include")
include_directories("Sophus")
include_directories("vikit_common/include")
include_directories("svo/include")
include_directories(eigen)

## Add any link libraries for each module. Add 'jevoisbase' here if you want to link against it:
if (JEVOIS_PLATFORM)
target_link_libraries(
	svo_no_ros
	${JEVOIS_PLATFORM_OPENCV_LIBS}
	-lopencv_imgproc
	-lopencv_core
	-L/home/jevois/svo_no_ros/svo/lib -lsvo
	-L/home/jevois/svo_no_ros/pbuild/Sophus -lSophus
  -L/home/jevois/svo_no_ros/vikit_common/lib -lvikit_common
	)
else()
target_link_libraries(
	svo_no_ros
	${JEVOIS_HOST_OPENCV_LIBS}
	opencv_imgproc
	opencv_core
	-L/home/jevois/svo_no_ros/svo/lib -lsvo
	-L/home/jevois/svo_no_ros/hbuild/Sophus -lSophus
  -L/home/jevois/svo_no_ros/vikit_common/lib -lvikit_common
	)
endif()

## Install any shared resources (cascade classifiers, neural network weights, etc) in the share/ sub-directory:
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/share"
  DESTINATION "${JEVOIS_INSTALL_ROOT}" COMPONENT bin)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_PREFIX}/lib:./lib:.")
set(CMAKE_INSTALL_RPATH "/modules/FINken_Nano/svo_no_ros")

## Wrapping up:
jevois_project_finalize()  # call this last in your CMakeLists.txt, will un-mount the live microSD if applicable



