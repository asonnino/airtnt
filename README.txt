# airtnt

----------------------------
Purpose of RemoteAttestation
----------------------------
The project demonstrates:
- How an application enclave can attest to a remote party
- How an application enclave and the remote party can establish a secure session

------------------------------------
How to Build/Execute the Sample Code
------------------------------------
1. Install Intel(R) Software Guard Extensions (Intel(R) SGX) SDK for Linux* OS
2. Make sure your environment is set:
    $ source ${sgx-sdk-install-path}/environment
3. Build the project with the prepared Makefile:
    a. Hardware Mode, Debug build:
        $ make
    b. Hardware Mode, Pre-release build:
        $ make SGX_PRERELEASE=1 SGX_DEBUG=0
    c. Hardware Mode, Release build:
        $ make SGX_DEBUG=0
    d. Simulation Mode, Debug build:
        $ make SGX_MODE=SIM
    e. Simulation Mode, Pre-release build:
        $ make SGX_MODE=SIM SGX_PRERELEASE=1 SGX_DEBUG=0
    f. Simulation Mode, Release build:
        $ make SGX_MODE=SIM SGX_DEBUG=0
4. Execute the binary directly:
    $ ./app
5. Remember to "make clean" before switching build mode
6. Add libcrypto to the path before runnig: `export LD_LIBRARY_PATH=`pwd`/sample_libcrypto`

** HTTPS Communication
The implementation requires cpp-netlib v0.12-final http://cpp-netlib.org/

In this version, there's a bug with HTTP version number. To fix it, apply https://github.com/cpp-netlib/cpp-netlib/pull/579/commits/2babeb4b114be2e4e2ee47303b8ae51f30b32e4e
