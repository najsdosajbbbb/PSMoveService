cmake_minimum_required(VERSION 3.0)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set(OPENVR_PLUGIN_INCL_DIRS)
set(OPENVR_PLUGIN_REQ_LIBS)

# platform independent include paths
list(APPEND OPENVR_PLUGIN_INCL_DIRS
    ${ROOT_DIR}/src/psmoveclient/
    ${PROTOBUF_INCLUDE_DIRS})

#OpenVR
FIND_PACKAGE(OpenVR REQUIRED)
list(APPEND OPENVR_PLUGIN_INCL_DIRS ${OPENVR_INCLUDE_DIR})
list(APPEND OPENVR_PLUGIN_REQ_LIBS ${OPENVR_LIBRARIES})    
    
# platform independent libraries
list(APPEND OPENVR_PLUGIN_REQ_LIBS 
    PSMoveClient)

# Source files that are needed for the shared library
file(GLOB OPENVR_PLUGIN_LIBRARY_SRC
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

# Shared library
add_library(driver_psmoveservice SHARED ${OPENVR_PLUGIN_LIBRARY_SRC})
target_include_directories(driver_psmoveservice PUBLIC ${OPENVR_PLUGIN_INCL_DIRS})
target_link_libraries(driver_psmoveservice ${OPENVR_PLUGIN_REQ_LIBS})

# Install    
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
install(TARGETS driver_psmoveservice
	RUNTIME DESTINATION ${ROOT_DIR}/win32/bin
	LIBRARY DESTINATION ${ROOT_DIR}/win32/lib
	ARCHIVE DESTINATION ${ROOT_DIR}/win32/lib)
ELSE() #Linux/Darwin
ENDIF()