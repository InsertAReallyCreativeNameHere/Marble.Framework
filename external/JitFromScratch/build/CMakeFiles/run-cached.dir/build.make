# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch\build

# Utility rule file for run-cached.

# Include the progress variables for this target.
include CMakeFiles/run-cached.dir/progress.make

CMakeFiles/run-cached:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Running JitFromScratch (with object cache)"
	.\JitFromScratch.exe -cache-dir=cache/

run-cached: CMakeFiles/run-cached
run-cached: CMakeFiles/run-cached.dir/build.make

.PHONY : run-cached

# Rule to build all files generated by this target.
CMakeFiles/run-cached.dir/build: run-cached

.PHONY : CMakeFiles/run-cached.dir/build

CMakeFiles/run-cached.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\run-cached.dir\cmake_clean.cmake
.PHONY : CMakeFiles/run-cached.dir/clean

CMakeFiles/run-cached.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch\build C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch\build C:\Users\Administrator\Documents\GitHub\CykaBlyat-2-IdiNahui-Boogaloo\MarbleFramework\external\JitFromScratch\build\CMakeFiles\run-cached.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/run-cached.dir/depend

