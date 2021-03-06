# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-error=strict-overflow")
  if(CC_WNOERROR_MISLEADING_INDENTATION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=misleading-indentation")
  endif()
endif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
# with gcc
if(CMAKE_COMPILER_IS_GNUCXX)
  if(NOT ${CMAKE_SYSTEM_VERSION} MATCHES el5 AND NOT ${ROOT_VERSION_MAJOR} LESS 6)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-error=strict-overflow")
    if(CC_WNOERROR_MISLEADING_INDENTATION)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=misleading-indentation")
    endif()
  endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(ADbase)
add_subdirectory(ADrec)
add_subdirectory(ADsim)

# Installing the macros
install(DIRECTORY macros DESTINATION AD)

message(STATUS "AD enabled")

if(DA)
   add_subdirectory(DA)
endif(DA)
