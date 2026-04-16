![GitHub repo size](https://img.shields.io/github/repo-size/Stephen-Duffy-SunyPoly/particle-life-parallel)
![GitHub](https://img.shields.io/github/license/hunar4321/life_code)

# Particle Life Simulation
A simple program to simulate primitive Artificial Life using simple rules of attraction or repulsion among atom-like particles, producing complex self-organzing life-like patterns.

---
# Parallel implementation Details
The work in this repo is a part of the final project for my graduate level parallel programing class.  
The goal of this project was to increase the performance of the simulation through the use of multiple threads.  
Specific modifications:
- Switch to CMake build system
  - The original version of this project was created to use Visual Studio's propritaty build system witch does not work with other IDEs and is not cross-platform portable.
  - The graphical system this project uses is called Open Frameworks. The intended way to utilize this libray is to have the Visual Studio project modified with the absolute paths to the compiled version of open frameworks on your system. As this is not cross platform compatable and not compable with CMake, this project has included a stripped down version of the open framework source code to compile with the project
  - All required dependencies for Open Frameworks are automatically fetched and built.
- Fixed font loading which was broken from the original repo
- Refactored storage of partible data to allow for generic particle type access in the parallel code.
- Rewrite particle interaction code to be computable in parallel
  - Moved particle position update outside the computation loop to improve computation performance and prevent race conditions
  - Changed the update calculations to update local velocity counters instead of the global counters to improve performance and prevent race conditions
  - Changed computation function parameters to take in group identifiers instead of the raw group parameters
- Implemented multy tear parallel system
  - 2 classes of worker threads exist
    - Color Manager thread. These threads are responsible for coordinating the computations for a specific color of particle vs all other colors. This thread type is triggerd from the main rendering thead, then trigger all the threads it manages. After all the computations are done, it merges the output of its managed threads into a single usable output. One of theese threads exsists per color.
    - Compute thread. These threads handle the computation of 1 type of particle vs another type of particle. This thread type is triggerd by its management thread.

# Supported Platforms
- Window
- ~~Linux~~ (Support was attempted but further development was abandoned due to a suspected compiler bug. Check out the linux branch if you want to test it)


# Compiling 
Compiling is accomplished through CMake  
It is recommended that you use the MSVC compiler if on Windows and build in release mode.
```shell
cd particle_life
mkdir build
cd build

cmake ..

cmake --build . --config Release
```
After the first time you should only need to re-run the last command unless significant structural changes have been made to the project.  
Note: The first time through, it will take a while to download all the dependencies

### Original Creator explanation video
https://youtu.be/0Kx4Y9TVMGg