# This file is automatically generated from cmake.toml - DO NOT EDIT
# See https://github.com/build-cpp/cmkr for more information

# Create a configure-time dependency on cmake.toml to improve IDE support
if(CMKR_ROOT_PROJECT)
	configure_file(cmake.toml cmake.toml COPYONLY)
endif()

# demo
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/demo")
else()
	set(CMAKE_FOLDER demo)
endif()
add_subdirectory(demo)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

# demolib
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/demolib")
else()
	set(CMAKE_FOLDER demolib)
endif()
add_subdirectory(demolib)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

# demolib2
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/demolib2")
else()
	set(CMAKE_FOLDER demolib2)
endif()
add_subdirectory(demolib2)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

