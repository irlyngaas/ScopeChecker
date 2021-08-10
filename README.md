# ScopeChecker
Clang tool to check if global variables within YAKL_parallel_fors are locally scoped in the function it is called from. If a variable a globally defined variable is not defined within the local function scope the code will still compile but causes runtime errors on GPUs.

# Example: SAM++ on thatchroof
1. Needs a functional Clang C/C++ compiler version >=10.0. Currently no issues cross-compiling using a gnu fortran compiler
2. Add -DCMAKE_EXPORT_COMPILE_COMMANDS=1 to cmake command in cmakescript.sh in the build directory
4. `source thatchroof_env_clang` (export CC=clang; export CXX=clang++;)
5. Replace YAKL_LAMBDA's because clang doesn't handle macros.  `sed -i s/YAKL_LAMBDA/\[=\]/g' *.cpp`
7. `./cmakescript.sh && make`
8. Should now be compile_commands.json file in the build directory


## Build and Run ScopeChecker

```
export PROJ_DIR=$PATH_to_SAM++_BUILD_DIR
git clone https://github.com/irlyngaas/ScopeChecker.git
cd ScopeChecker
mkdir build && cd build && cmake .. && make && cd ..
./check-all.py -p $PROJ_DIR/test/buildcompile_commands.json -l build/src/libScopeChecker.so -n ScopeChecker
```

Add YAKL_LAMBDA macros back to the code
```
cd ../../..
sed -i 's/YAKL_LAMBDA/\[=\]/g' *.cpp
```
