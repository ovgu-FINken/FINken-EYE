################################################################################
# user build settings

SET(TRACE TRUE)
SET(HAVE_G2O FALSE) 
SET(USE_ROS FALSE) # Set FALSE if you want to build this package without Catkin
SET(DEBUG_OUTPUT FALSE) # Only relevant if build without ROS

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################################################################

SET(PROJECT_NAME svo)
PROJECT(${PROJECT_NAME})
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.3)
SET(CMAKE_BUILD_TYPE Release) # Release, RelWithDebInfo
SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules/")

## Include helper functions, config, etc from the JeVois install:
set(JEVOIS_CONFIG "/jevois/config" CACHE STRING "Path to JeVois config to use")
set(CMAKE_MODULE_PATH ${JEVOIS_CONFIG})
include(jevois_config)
include(JeVois)

# Set definitions
IF(TRACE)
  ADD_DEFINITIONS(-DSVO_TRACE)
ENDIF()
IF(HAVE_G2O)
  ADD_DEFINITIONS(-DUSE_BUNDLE_ADJUSTMENT)
ENDIF()
IF(USE_ROS)
  ADD_DEFINITIONS(-DSVO_USE_ROS)
ENDIF()
IF(DEBUG_OUTPUT)
  ADD_DEFINITIONS(-DSVO_DEBUG_OUTPUT)
ENDIF()

# Set build flags, set ARM_ARCHITECTURE environment variable on Odroid
# Set build flags. Set IS_ARM on odroid board as environment variable
SET(CMAKE_CXX_FLAGS "-Wall -D_LINUX -D_REENTRANT -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unknown-pragmas")
IF(DEFINED ENV{ARM_ARCHITECTURE})
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=hard -ftree-vectorize -Ofast -funsafe-math-optimizations -mfp16-format=ieee")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmmx -msse -msse -msse2 -msse3 -mssse3")
ENDIF()
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -fsee -fomit-frame-pointer -fno-signed-zeros -fno-math-errno -funroll-loops")

# Add plain cmake packages 
#FIND_PACKAGE(OpenCV REQUIRED)
#FIND_PACKAGE(Eigen REQUIRED)
#FIND_PACKAGE(Sophus REQUIRED)
#FIND_PACKAGE(fast REQUIRED)
#FIND_PACKAGE(Boost REQUIRED COMPONENTS thread system)

#FIND_PACKAGE(vikit_common REQUIRED)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Include dirs
INCLUDE_DIRECTORIES(
  include
#  /home/jevois/workspace/opencv-3.4.5/include
#  /home/jevois/workspace/opencv-3.4.5/modules/core/include
#  /home/jevois/workspace/opencv-3.4.5/modules/imgproc/include
#  /home/jevois/workspace/opencv-3.4.5/modules/highgui/include
#  /home/jevois/workspace/opencv-3.4.5/modules/calib3d/include
#  /home/jevois/workspace/opencv-3.4.5/modules/features2d/include
#  /home/jevois/workspace/opencv-3.4.5/modules/dnn/include
#  /home/jevois/workspace/opencv-3.4.5/modules/flann/include
#  /home/jevois/workspace/opencv-3.4.5/modules/imgcodecs/include
#  /home/jevois/workspace/opencv-3.4.5/modules/photo/include
#  /home/jevois/workspace/opencv-3.4.5/modules/shape/include
#  /home/jevois/workspace/opencv-3.4.5/modules/stitching/include
#  /home/jevois/workspace/opencv-3.4.5/modules/superres/include
#  /home/jevois/workspace/opencv-3.4.5/modules/ts/include
#  /home/jevois/workspace/opencv-3.4.5/modules/video/include
#  /home/jevois/workspace/opencv-3.4.5/modules/videoio/include
#  /home/jevois/workspace/opencv-3.4.5/modules/videostab/include
#  /home/jevois/workspace/opencv-3.4.5/modules/viz/include
#  /home/jevois/workspace/opencv-3.4.5/modules/world/include
#  /home/jevois/workspace/opencv-3.4.5/modules/ml/include
#  /home/jevois/workspace/opencv-3.4.5/modules/objdetect/include
#  /home/jevois/workspace/opencv-3.4.5/build/
  /usr/share/jevois-opencv-4.0.1/include/opencv4
  /home/jevois/svo_no_ros/eigen
  /home/jevois/workspace/Sophus
  /home/jevois/workspace/fast/include
  /home/jevois/svo_no_ros/vikit_common/include
)

# Set link libraries
IF(DEFINED ENV{ARM_ARCHITECTURE})
  LIST(APPEND LINK_LIBS 
  -lopencv_core -lopencv_imgproc -lopencv_features2d -lopencv_flann -lopencv_ml -lopencv_objdetect -lopencv_imgcodecs -lopencv_video -lopencv_videoio -lopencv_highgui -lopencv_calib3d
  -L/home/jevois/svo_no_ros/pbuild/Sophus -lSophus
  -L/home/jevois/svo_no_ros/pbuild/fast -lfast
  boost_thread
  boost_system
  -L/home/jevois/svo_no_ros/lib -lvikit_common
  )
ELSE()
  LIST(APPEND LINK_LIBS 
  -L/usr/share/jevois-opencv-4.0.1/lib -lopencv_core -lopencv_imgproc -lopencv_features2d -lopencv_flann -lopencv_ml -lopencv_objdetect -lopencv_imgcodecs -lopencv_video -lopencv_videoio -lopencv_highgui -lopencv_calib3d
  -L/home/jevois/workspace/Sophus/build -lSophus
  -L/home/jevois/svo_no_ros/hbuild/fast -lfast
  -L/home/jevois/svo_no_ros/lib -lvikit_common
  boost_thread
  boost_system
  
)
ENDIF()

# Set sourcefiles
LIST(APPEND SOURCEFILES
  src/frame_handler_mono.cpp
  src/frame_handler_base.cpp
  src/frame.cpp
  src/point.cpp
  src/map.cpp
  src/pose_optimizer.cpp
  src/initialization.cpp
  src/matcher.cpp
  src/reprojector.cpp
  src/feature_alignment.cpp
  src/feature_detection.cpp
  src/depth_filter.cpp
  src/config.cpp
  src/sparse_img_align.cpp)

# Create svo library
ADD_LIBRARY(svo SHARED ${SOURCEFILES})
TARGET_LINK_LIBRARIES(svo ${LINK_LIBS})

################################################################################
# TESTS

ADD_EXECUTABLE(test_feature_align test/test_feature_alignment.cpp)
TARGET_LINK_LIBRARIES(test_feature_align svo)

ADD_EXECUTABLE(test_pipeline test/test_pipeline.cpp)
TARGET_LINK_LIBRARIES(test_pipeline svo)

ADD_EXECUTABLE(test_matcher test/test_matcher.cpp)
TARGET_LINK_LIBRARIES(test_matcher svo)

ADD_EXECUTABLE(test_feature_detection test/test_feature_detection.cpp)
TARGET_LINK_LIBRARIES(test_feature_detection svo)

ADD_EXECUTABLE(test_depth_filter test/test_depth_filter.cpp)
TARGET_LINK_LIBRARIES(test_depth_filter svo)

ADD_EXECUTABLE(test_sparse_img_align test/test_sparse_img_align.cpp)
TARGET_LINK_LIBRARIES(test_sparse_img_align svo)

ADD_EXECUTABLE(test_pose_optimizer test/test_pose_optimizer.cpp)
TARGET_LINK_LIBRARIES(test_pose_optimizer svo) 
