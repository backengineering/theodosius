# This file is automatically generated from cmake.toml - DO NOT EDIT
# See https://github.com/build-cpp/cmkr for more information

# Create a configure-time dependency on cmake.toml to improve IDE support
if(CMKR_ROOT_PROJECT)
	configure_file(cmake.toml cmake.toml COPYONLY)
endif()

# spdlog
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/spdlog")
else()
	set(CMAKE_FOLDER spdlog)
endif()
add_subdirectory(spdlog)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

# Target linux-pe
set(CMKR_TARGET linux-pe)
set(linux-pe_SOURCES "")

set(CMKR_SOURCES ${linux-pe_SOURCES})
add_library(linux-pe INTERFACE)

if(linux-pe_SOURCES)
	target_sources(linux-pe INTERFACE ${linux-pe_SOURCES})
endif()

target_include_directories(linux-pe INTERFACE
	"linux-pe/includes/"
)

unset(CMKR_TARGET)
unset(CMKR_SOURCES)

# Target xed
set(CMKR_TARGET xed)
set(xed_SOURCES "")

set(CMKR_SOURCES ${xed_SOURCES})
add_library(xed INTERFACE)

if(xed_SOURCES)
	target_sources(xed INTERFACE ${xed_SOURCES})
endif()

target_include_directories(xed INTERFACE
	"xed/obj/wkit/include/xed"
)

target_link_libraries(xed INTERFACE xed.lib)
target_link_libraries(xed INTERFACE xed-ild.lib)
target_link_directories(xed INTERFACE xed/obj/wkit/lib/)

unset(CMKR_TARGET)
unset(CMKR_SOURCES)

