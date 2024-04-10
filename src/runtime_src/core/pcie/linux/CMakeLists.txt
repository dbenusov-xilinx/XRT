# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
add_subdirectory(plugin/xdp)

add_library(core_pcielinux_objects OBJECT
  pcidev.cpp
  pcidrv.cpp
  system_linux.cpp
  )

target_compile_definitions(core_pcielinux_objects
  PRIVATE
  XCLHAL_MAJOR_VER=2
  XCLHAL_MINOR_VER=1
  )

target_include_directories(core_pcielinux_objects
  PRIVATE
  ${XRT_SOURCE_DIR}/runtime_src
  ${XRT_BINARY_DIR}/gen
  )


add_library(xrt_core SHARED
  $<TARGET_OBJECTS:core_pcielinux_objects>
  $<TARGET_OBJECTS:core_pciecommon_objects>
  )

add_library(xrt_core_static STATIC
  $<TARGET_OBJECTS:core_pcielinux_objects>
  $<TARGET_OBJECTS:core_pciecommon_objects>
  )

set_target_properties(xrt_core PROPERTIES
  VERSION ${XRT_VERSION_STRING}
  SOVERSION ${XRT_SOVERSION})

# Private dependencies for fully resolved dynamic xrt_core
target_link_libraries(xrt_core
  PRIVATE
  xrt_coreutil
  ${Boost_SYSTEM_LIBRARY}
  pthread
  rt
  dl
  uuid
  )

# Targets linking with xrt_core_static must also link with additional
# libraries to satisfy xrt_core_static dependencies. These type of
# link dependencies are known as INTERFACE dependencies.  Here the
# boost libraries are specified by their system name so that static of
# target can pick static link libraries of boost
target_link_libraries(xrt_core_static
  INTERFACE
  xrt_coreutil_static
  boost_system
  uuid
  dl
  rt
  pthread
  )

install(TARGETS xrt_core
  EXPORT xrt-targets
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR} ${XRT_NAMELINK_SKIP}
  RUNTIME DESTINATION ${XRT_INSTALL_BIN_DIR}
)

install(TARGETS xrt_core xrt_core_static
  EXPORT xrt-dev-targets
  ARCHIVE DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_DEV_COMPONENT}
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_DEV_COMPONENT} ${XRT_NAMELINK_ONLY}
)

# Create alveo only package
set(ALVEO_TARGET xrt_driver_alveo)

add_library(${ALVEO_TARGET} SHARED
  $<TARGET_OBJECTS:core_pcielinux_plugin_xdp_objects>
  $<TARGET_OBJECTS:core_common_objects>
  debug.cpp
  device_linux.cpp
  pcidev_alveo.cpp
  pcidrv_xocl.cpp
  pcidrv_xclmgmt.cpp
  shim.cpp
  )

set_target_properties(${ALVEO_TARGET} PROPERTIES
  VERSION ${XRT_VERSION_STRING}
  SOVERSION ${XRT_SOVERSION}
  )

target_compile_definitions(${ALVEO_TARGET} PRIVATE
  # below macros is required so that i/f defined in ishim.h is
  # consistent with native xrt implementation
  XRT_ENABLE_AIE
  XRT_AIE_BUILD
  XCLHAL_MAJOR_VER=2
  XCLHAL_MINOR_VER=1
  )

target_include_directories(${ALVEO_TARGET}
  PRIVATE
  ${XRT_SOURCE_DIR}/runtime_src
  ${XRT_BINARY_DIR}/gen
  )

target_link_libraries(${ALVEO_TARGET}
  PRIVATE
  xrt_core
  xrt_coreutil
  )

install(TARGETS ${ALVEO_TARGET} DESTINATION xrt/lib COMPONENT alveo)

