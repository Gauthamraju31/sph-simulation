SPH Simulation
============

Experimenting with performance evaluation and improvements to Brandon Pelfrey's [smoothed-particle hydrodynamics (SPH)](https://en.wikipedia.org/wiki/Smoothed-particle_hydrodynamics) fluid simulation tutorial:

* [Real-time Physics 101: Let’s Talk Particles](https://web.archive.org/web/20090530024753/http://blog.brandonpelfrey.com/?p=58)
* [Real-time Physics 102: Springboard into Constraints](https://web.archive.org/web/20090531100829/http://blog.brandonpelfrey.com/?p=242)
* [Real-Time Physics 103: Fluid Simulation in Games](https://web.archive.org/web/20090722233436/http://blog.brandonpelfrey.com/?p=303)

##### Requirements

* CMake >= 3.10.0
* C++11 compiler with OpenMP support
* OpenGL runtime support

##### Controls

* Q/Escape: Exit
* Space: Add more particles
* Mouse button: Attract nearby particles
* +/-: Increase/decrease the number of simulation steps per frame

You can use the [`OMP_NUM_THREADS` environment variable](https://gcc.gnu.org/onlinedocs/libgomp/OMP_005fNUM_005fTHREADS.html#OMP_005fNUM_005fTHREADS) to limit the number of threads used by OpenMP.

##### Dependencies

Appropriate versions of [GLM](https://glm.g-truc.net/) is included as submodules so there's no need to install it separately. However GLFW is needed to be installed seperately.
GLFW has some platform-specific build dependencies:

* Linux

      # Debian & derivatives:
      sudo apt install \
      libx11-dev \
      libxrandr-dev \
      libxinerama-dev \
      libxcursor-dev \
      libxi-dev \

##### Building

    git clone --recurse-submodules https://github.com/Gauthamraju31/sph-simulation.git
    cd sph-simulation
    mkdir build
    cd build
    cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build .
