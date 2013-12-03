if(NOT MSVC AND NOT BINGO)
	set(VISIBILITY_HIDDEN YES)
endif()
if (NOT CMAKE_BUILD_TYPE)
	if(NOT BUILD_DEBUG)
		 MESSAGE(STATUS "Set CMAKE_BUILD_TYPE to Release")
		 set(CMAKE_BUILD_TYPE Release)
	else()
		 MESSAGE(STATUS "Set CMAKE_BUILD_TYPE to Debug")
		 set(CMAKE_BUILD_TYPE Debug)
	endif()
endif()
MESSAGE(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
if(MSVC)
	add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		
		add_definitions(/Z7)
		message(STATUS DEBUG)
	endif()
endif()
if(UNIX AND NOT APPLE)
	if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(amd64|i.86|powerpc|ppc|sparc|x86_64)")
		if (SUBSYSTEM_NAME MATCHES "x86")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		elseif(SUBSYSTEM_NAME MATCHES "x64")
			if(CMAKE_SYSTEM_NAME MATCHES "Linux")
				set(CMAKE_C_FLAGS "-include ${CMAKE_CURRENT_LIST_DIR}/../hacks/gcc_preinclude.h ${CMAKE_C_FLAGS}")
				set(CMAKE_CXX_FLAGS "-include ${CMAKE_CURRENT_LIST_DIR}/../hacks/gcc_preinclude.h ${CMAKE_CXX_FLAGS}")
			endif()
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
		endif()
	endif ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	if (DEFINED ENV{UNIVERSAL})
		set(CMAKE_C_CREATE_SHARED_LIBRARY   "python ${CMAKE_CURRENT_LIST_DIR}/linkhack.py | gcc | <LINK_FLAGS> | Linux/${SUBSYSTEM_NAME} | <OBJECTS> | <LINK_LIBRARIES> | <TARGET>")
    	set(CMAKE_CXX_CREATE_SHARED_LIBRARY "python ${CMAKE_CURRENT_LIST_DIR}/linkhack.py | g++ | <LINK_FLAGS> | Linux/${SUBSYSTEM_NAME} | <OBJECTS> | <LINK_LIBRARIES> | <TARGET>")
	endif()
endif()
if(APPLE)
	include(MacFrameworks)
    if (NOT ${BingoOracle_SOURCE_DIR} MATCHES "")
	    set(CMAKE_OSX_ARCHITECTURES "x86_64")
	endif()
	if(DEFINED ENV{INDIGO_CMAKE_OSX_ARCHITECTURES})
		set(CMAKE_OSX_ARCHITECTURES $ENV{INDIGO_CMAKE_OSX_ARCHITECTURES})
		# All parameters: <CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET><TARGET_VERSION><TARGET_VERSION_SUFFIX> <OBJECTS> <LINK_LIBRARIES>; <CMAKE_SYMLINK> <TARGET><TARGET_VERSION_SUFFIX> <TARGET><TARGET_SOVERSION_SUFFIX>; <CMAKE_SYMLINK> <TARGET><TARGET_SOVERSION_SUFFIX> <TARGET>
    	set(CMAKE_C_CREATE_STATIC_LIBRARY "libtool -static <LINK_FLAGS> -arch_only $ENV{INDIGO_CMAKE_OSX_ARCHITECTURES} -o <TARGET> <OBJECTS>")
    	set(CMAKE_CXX_CREATE_STATIC_LIBRARY "libtool -static <LINK_FLAGS> -arch_only $ENV{INDIGO_CMAKE_OSX_ARCHITECTURES} -o <TARGET> <OBJECTS>")
    	set(CMAKE_C_CREATE_SHARED_LIBRARY "python ${CMAKE_CURRENT_LIST_DIR}/linkhack.py | gcc | <LINK_FLAGS> | $ENV{INDIGO_CMAKE_OSX_ARCHITECTURES} | <OBJECTS> | <LINK_LIBRARIES> | <TARGET>")
    	set(CMAKE_CXX_CREATE_SHARED_LIBRARY "python ${CMAKE_CURRENT_LIST_DIR}/linkhack.py | g++ | <LINK_FLAGS> | $ENV{INDIGO_CMAKE_OSX_ARCHITECTURES} | <OBJECTS> | <LINK_LIBRARIES> | <TARGET>")
	else()
    	set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
   		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
    if (${XCODE_VERSION} GREATER 4.2)
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SUBSYSTEM_NAME}.sdk)
    else()
        set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${SUBSYSTEM_NAME}.sdk)
    endif()
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${SUBSYSTEM_NAME})
    message(STATUS "SDK: ${CMAKE_OSX_SYSROOT}")
    message(STATUS "Deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
if(APPLE)
	SET(COMPILE_FLAGS "${COMPILE_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(LINK_FLAGS "${LINK_FLAGS}")
endif()
#Set RPATH
if(UNIX OR APPLE)
        SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
        if(APPLE)
                SET(CMAKE_INSTALL_RPATH "\@loader_path")
        else()
                SET(CMAKE_INSTALL_RPATH "\$ORIGIN")
        endif()
        SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
