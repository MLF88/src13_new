
PROJECT := caffe

CXX ?= /usr/bin/g++

CONFIG_FILE := Makefile.config
include $(CONFIG_FILE)

DYNAMIC_VERSION_MAJOR 		:= 1
DYNAMIC_VERSION_MINOR 		:= 0
DYNAMIC_VERSION_REVISION 	:= 0-rc3
COMMON_FLAGS += -DCAFFE_VERSION=$(DYNAMIC_VERSION_MAJOR).$(DYNAMIC_VERSION_MINOR).$(DYNAMIC_VERSION_REVISION)

COMMON_FLAGS += -DNDEBUG -O2


COMMON_FLAGS += -DUSE_CUDNN
COMMON_FLAGS += -DUSE_OPENCV
COMMON_FLAGS += -DUSE_LEVELDB
COMMON_FLAGS += -DUSE_LMDB
COMMON_FLAGS += -DWITH_PYTHON_LAYER

ifeq ($(SAVE_FRAME_IN), 1)
	COMMON_FLAGS += -DSAVE_FRAME_IN
endif

ifeq ($(SAVE_FRAME_OUT), 1)
	COMMON_FLAGS += -DSAVE_FRAME_OUT
endif

CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
#CUDA_INCLUDE_DIR2 := /usr/local/cuda-8.0/targets/aarch64-linux/include
INCLUDE_DIRS += $(MYCAFFE_SRC)
INCLUDE_DIRS += $(MYCAFFE_SRC_BUILD)
INCLUDE_DIRS += $(MYCAFFE_INCLUDE)
INCLUDE_DIRS += $(CUDA_INCLUDE_DIR)
#INCLUDE_DIRS += $(CUDA_INCLUDE_DIR2)

COMMON_FLAGS += $(foreach includedir,$(INCLUDE_DIRS),-isystem $(includedir))

WARNINGS := -Wall -Wno-sign-compare

LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)

LIBRARY_NAME := $(PROJECT)

LIBRARIES := cudart cublas curand
LIBRARIES += glog gflags protobuf boost_system boost_filesystem boost_regex m hdf5_hl hdf5
LIBRARIES += openblas
LIBRARIES += leveldb snappy
LIBRARIES += lmdb
LIBRARIES += opencv_core opencv_highgui opencv_imgproc
LIBRARIES += mycamera
#LIBRARIES += zhineng
#LIBRARIES += init_pid
LIBRARIES += boost_thread stdc++
LIBRARIES += cudnn
LIBRARIES += caffe

PYTHON_LIBRARIES ?= boost_python python2.7
LIBRARIES += $(PYTHON_LIBRARIES)



LIB_BUILD_DIR := $(MYBUILD_DIR)/lib
CUDA_LIB_DIR += $(CUDA_DIR)/lib64

LIBRARY_DIRS += $(CUDA_LIB_DIR)
LIBRARY_DIRS += $(LIB_BUILD_DIR)

LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) \
		$(foreach library,$(LIBRARIES),-l$(library))

#ssd_detect_video: ssd_detect_video.cpp
#	$(CXX) ssd_detect_video.cpp -o ssd_detect_video $(LINKFLAGS) -lmycaffe $(LDFLAGS) \
#			-Wl,-rpath
#clean:
#	rm -rf ssd_detect_video

#ssd_detect_pic: ssd_detect_pic.cpp
#	$(CXX) ssd_detect_pic.cpp -o ssd_detect_pic $(LINKFLAGS) -lmycaffe $(LDFLAGS) \
			-Wl,-rpath
#clean:
#	rm -rf ssd_detect_pic

all: main.cpp dp_learning.cpp zhineng.cpp motorx.cpp motory.cpp
	$(Q)$(CXX) $^ -o $@ $(LINKFLAGS) -lmycaffe $(LDFLAGS) \
			-Wl,-rpath
	cp all /home/ubuntu/mu/
clean:
	rm -rf all /home/ubuntu/mu/all










