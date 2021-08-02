# ScopeChecker
Clang tool to check if global variables within lambda's are locally scoped to in the function the lambda is called from. If a globally scope variable is used a YAKL parallel_for is able to compile, but when ran on a GPU there are issues with variables that aren't scoped locally.

##Build and Run ScopeChecker

``
mkdir build 
cd build && cmake ..
make
cd src
./ScopChecker testFile.cpp
``
