# About
SONAR (Simple Offline Network AnalyseR) is a trace analysis tool to extract metric from large scale MPI application traces.

# Requirements
To build SONAR, you'll need the following tools and libraries:

* cmake >= 2.8
* GCC compiler >= 4.8.2
* zlib1g zlib1g-dev
* gnuplot
* OpenMPI >= 1.6.5 (optional, for trace generation)
* VampirTrace >= 5.14.4 (optional, for trace generation)

For Debian/Ubuntu based systems:
```
apt-get install cmake gnuplot zlib1g zlib1g-dev
```

The [Open Trace Format (OTF)](https://tu-dresden.de/zih/forschung/projekte/otf) library will be downloaded and built automatically.
Please make sure that zlib has been installed before the library will be built, as it is essential for reading compressed traces.

# Installation
```
git clone
cd sonar
mkdir build
cd build
cmake ..
make [-j4]
```

# Usage
After a successful build, you can test sonar with one of the example traces in the samples directory:

```
./sonar ../sampletraces/lulesh_8p.otf
```

This will run the analysis of the example trace. SONAR will create a new directory in which the results will be stored.
The directory will be named according to the trace. Furthermore, the current time will be appended which prevents the overwriting of results due to multiple runs.

# Custom trace generation
You can create traces of your own application using [VampirTrace](https://tu-dresden.de/zih/forschung/projekte/vampirtrace).
Quick start:
```
vmpirun [MPI-OPTIONS] ./mpi-application [APP-OPTIONS]
```
For more information, please refer to the VampirTrace documentation.

# License
Copyright (c) 2016, Computer Engineering Group at Ruprecht-Karls University of Heidelberg, Germany. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of Ruprecht-Karls University of Heidelberg, Georgia Institute of Technology nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
