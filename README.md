# InTCPtor

The InTCPtor (pronounced: interceptor) is a library that intercepts TCP-related BSD socket function calls and introduces delays, message fragmentation and more.

Please note, that this software works only on GNU/Linux as it uses a platform-dependent way to override standard library calls. There is a chance, that in future, we will enhance the software to support macOS, but it is highly unlikely MS Windows will be supported.

There are two main components:
- `libintcptor-overrides` - dynamic library (shared object) that implements the overrides
- `intcptor-run` - runner that sets the `LD_PRELOAD` environment variable and starts the executable

## Build

To build the library and runner, you can execute the following commands:

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

If you intend to use the runner to start your network application, you can do it without any additional hassle. The only requirement is to have the runner and overrides library in the same directory:

```
./intcptor-run my-server 127.0.0.1 10000
```

The `intcptor-run` executable sets the environment variable `LD_PRELOAD` and starts the desired executable.

Alternatively, you can invoke the executable with the overrides library without the runner by setting the `LD_PRELOAD` manually:

```
LD_PRELOAD=./libintcptor-overrides.so ./my-server 127.0.0.1 10000
```

## What does it do?

It hooks the following functions: `socket`, `close`, `accept`, `recv`, `read`, `send`, `write`.

For now, the `read()` call upon a managed socket is translated as a call to `recv()` with `flags` parameter set to zero. The same applies to `write()` and `send()`.

## Planned features

## License

The InTCPtor software is distributed under MIT license. See attached LICENSE file for full licencing information.

|![University of West Bohemia](https://www.zcu.cz/en/assets/logo.svg)|![Department of Computer Science and Engineering](https://www.kiv.zcu.cz/site/documents/verejne/katedra/dokumenty/dcse-logo-barevne.png)|
|--|--|