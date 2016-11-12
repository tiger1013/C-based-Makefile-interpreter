1.) Purpose: C-based Makefile interpreter- emulates UNIX's makefile functionality, also implementing concurrency.

2.) Usage: This will build the first target found in makefile ./make4061:

This will build only the specific target
    ./make4061 specifictarget


Available Options:
-f filename: filename will be the name of the makefile, otherwise the default name ’makefile’ is assumed.
-n: Only displays the commands that would be run, doesn’t actually execute them.
-B: Do not check timestamps for target and input (i.e. always recompile).
-m log.txt: The log from make4061 will be stored on file log.txt

Examples:
./make4061 -f yourownmakefile
./make4061 -n
./make4061 -m logfilename
3.) Other Information: This makefile assumes there can be only 10 targets in a makefile. So, the program is implemented on a fixed-size array of structs (See "util.h") The main work happens in "main.c", where the the makefile is parsed and is executed recurively using Depth-first search and exploiting concurrency.
