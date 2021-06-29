# Gaemi-01

Gaemi-NN is a cross platform C++ realtime engine, set for indie game dev and personal
experiment. It is meant to be simple to use and straightforward. Version of the engine
evolves with projects and progressively add features.

# Compiler

Clang

## install clang

### windows
download and execute https://releases.llvm.org/download.html

## install ninja

### windows

Download from https://github.com/ninja-build/ninja/releases

or build from source:

```
git clone git://github.com/martine/ninja.git
cd ninja/
cmake .
```

Then open ninja.sln, set project to release, build solution. Add to your path the `ninja/Release` folder. Don't forget to restart your terminal / IDE to update its terminal path.