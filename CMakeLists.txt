cmake_minimum_required (VERSION 2.8.4)

project (protoc-gen-luabind)
 
ADD_SUBDIRECTORY(src)

SET(CMAKE_INSTALL_PREFIX /usr/local)
 
INSTALL(PROGRAMS DESTINATION bin)