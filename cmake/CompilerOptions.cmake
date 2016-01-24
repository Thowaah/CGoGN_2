include(cmake/utilities.cmake)

# Profiler compilation flags
if(CGOGN_USE_GPROF)
	message(STATUS "Building for code profiling")
	add_flags(CMAKE_CXX_FLAGS -pg -DPROFILER)
	add_flags(CMAKE_C_FLAGS -pg -DPROFILER)
endif(CGOGN_USE_GPROF)

# Code coverage compilation flags
if(CGOGN_USE_GCOV)
	message(STATUS "Building for coverage analysis")
	add_flags(CMAKE_CXX_FLAGS --coverage)
	add_flags(CMAKE_C_FLAGS --coverage)
endif(CGOGN_USE_GCOV)

# Compilation flags for Google's AddressSanitizer
# These flags can only be specified for dynamic builds
if(CGOGN_USE_ASAN)
	message(STATUS "Building with AddressSanitizer (debug only)")
	add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer -O1)
	add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer -O1)
endif(CGOGN_USE_ASAN)
#TODO Use native GCC stack smash Protection and buffer overflow detection in debug when no asan ??

# Compilation flags for Google's ThreadSanitizer
# Does not work for the moment: cannot figure out how to link with library libtsan
if(CGOGN_USE_TSAN)
	message(STATUS "Building with ThreadSanitizer (debug only)")
	add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=thread)
	add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=thread)
endif(CGOGN_USE_TSAN)

if (NOT MSVC)
	# Warning flags
	set(NORMAL_WARNINGS -Wall -Wextra)

	if(NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang"))
		set(FULL_WARNINGS
			${NORMAL_WARNINGS}
			-pedantic
			-Wno-long-long
			-Wconversion
			-Winline
			-Wsign-conversion
			-Wdouble-promotion)

	else()
		set(FULL_WARNINGS
			-Weverything
			-Wno-unused-macros
			-Wno-disabled-macro-expansion
			-Wno-covered-switch-default
			-Wno-padded
			-Wno-float-equal
			# Ignore warnings about global variables ctors and dtors
			-Wno-global-constructors
			# Ignore warnings about global destructor
			-Wno-exit-time-destructors
			# Turn this on to detect documentation errors (very useful)
			-Wno-documentation
			# Ignore unknown documentation command (There are nrecognized but valid doxygen commands !)
			-Wno-documentation-unknown-command
			# Ignore warnings about C++98 compatibility
			-Wno-c++98-compat
			# Ignore warnings about c++98 compat pedantic mode
			-Wno-c++98-compat-pedantic
			# Ignore warnings about C++11 extensions (cgogn is promoting c++11 )
			-Wno-c++11-extensions

			# # Too many of sign conversion problems (Eigen!!!). Ignore them for the moment.
			# -Wno-sign-conversion
			# #
			# -Wno-deprecated
			# -Wno-old-style-cast
			)
	endif()

	add_flags(CMAKE_CXX_FLAGS "-Wnon-virtual-dtor")

	if (${CGOGN_USE_PARALLEL_GLIBCXX})
		add_flags(CMAKE_CXX_FLAGS_DEBUG "-D_GLIBCXX_PARALLEL")
	else()
		# _GLIBCXX_ASSERTIONS and _GLIBCXX_PARALLEL are not compatible on some systems.
		add_flags(CMAKE_CXX_FLAGS_DEBUG "-D_GLIBCXX_ASSERTIONS")
	endif()
	remove_definitions("-D_GLIBCXX_USE_DEPRECATED")

	# Enable SSE3 instruction set
	add_flags(CMAKE_CXX_FLAGS "-msse3")
	add_flags(CMAKE_C_FLAGS "-msse3")

	# Always generate position independant code
	# (to allow linking with DLLs)
	add_flags(CMAKE_CXX_FLAGS "-fPIC")
	add_flags(CMAKE_C_FLAGS "-fPIC")
	add_flags(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2")
	add_flags(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2")

else() # MSVC
	#Specifies the level of warning to be generated by the compiler. Valid warning levels for n range from 0 to 4.
	string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

	# C4127 - conditional expression is constant
	# C4714 - marked as __forceinline but not inlined
	# C4505 - unreferenced local function has been removed (impossible to deactive selectively)
	# C4910 - __declspec(dllexport)' and 'extern' are incompatible on an explicit instantiation
	# C4251 - 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	add_flags(CMAKE_CXX_FLAGS "/EHsc /wd4127 /wd4505 /wd4714 /wd4910 /wd4251")

endif()
