# Marching Cubes Server

A WebSocket API server that applies the marching cubes algorithm on a Cartesian grid that is sent by a client.
This application is meant to be used together with the CindyPrint plugin of the CindyJS framework (https://github.com/chrismile/CindyJS).

Application port: 17279


## Building and running the programm

On Ubuntu 20.04 for example, you can install all necessary packages with this command:

```
sudo apt-get install git cmake libboost-dev ocl-icd-opencl-dev opencl-headers clinfo libwebsocketpp-dev
```

The application was tested using NVIDIA's OpenCL implementation, Intel NEO and POCL. However, it should also run with different implementations.
Depending on the OpenCL implementation you want to use, install one of the following packages.

```
sudo apt-get install nvidia-opencl-dev intel-opencl-icd beignet-opencl-icd pocl-opencl-icd
```

intel-opencl-icd is the new OpenCL implementation of Intel that was added in Ubuntu 19.04 to the distribution repositories.
For previous distributions, please use Beignet. POCL is an OpenCL implementation using the user's CPU.

Finally, after installing all necessary prerequisites, the program can be compiled by calling the following commands.

```
mkdir build
cd build
cmake ..
make -j 4
ln -s ../cl .
```

Please note that creating a symbolic link in the application directory to the directory containing the OpenCL code files is necessary for the application to run.