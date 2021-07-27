# Install script for directory: E:/projects/NPLRuntime/Client

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/CLIENT")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/projects/NPLRuntime/Client/Win32/zlib-1.2.11/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/curl-7.47.1/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/lua-5.1.4/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/LuaJIT/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/luabind-0.9/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/sqlite-3.6.23.1/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/jsoncpp-0.5.0/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/tinyxpath_1_3_1/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/common/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/features/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/Pinocchio/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/luasql/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/assimp-4.0.0/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/ParaEngineClient/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/bullet3/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/PhysicsBT/cmake_install.cmake")
  include("E:/projects/NPLRuntime/Client/Win32/NPLMono/NPLMono2/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "E:/projects/NPLRuntime/Client/Win32/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
