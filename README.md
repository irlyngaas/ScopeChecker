# ScopeChecker
Clang tool to check if global variables within YAKL_parallel_fors are locally scoped in the function it is called from. If a variable a globally defined variable is not defined within the local function scope the code will still compile but causes runtime errors on GPUs.

# Example: SAM++ on thatchroof
1. Needs a functional Clang C/C++ compiler. Currently no issues cross-compiling using a gnu fortran compiler
2. Add -DCMAKE_EXPORT_COMPILE_COMMANDS=1 to cmake command in cmakescript.sh in the build directory
4. source thatchroof_env_clang (export CC=clang; export CXX=clang++;) 
5. ./cmakescript.sh && make
6. Should now be compile_commands.json file in the build directory


## Build and Run ScopeChecker

```
export PROJ_DIR=$PROJ_DIR
git clone https://github.com/irlyngaas/ScopeChecker.git
cd ScopeChecker
mkdir build && cd build && cmake .. && make 
./check-all.py -p $PROJ_DIR/test/buildcompile_commands.json -l build/src/libScopeChecker.so -n ScopeChecker
```

# Example: MiniWeather on summit
