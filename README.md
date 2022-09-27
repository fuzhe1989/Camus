# Hafnium (hf)

[Hafnium](https://en.wikipedia.org/wiki/Hafnium) is a chemical element. It's chinese name is "铪".

Hf is an experimental project.

## Motivations

1. Build applications by leveraging modern C++ (coroutine, ranges, execution, etc.) techniques.
2. Build a set of high-performance data-oriented systems from scratch.

### The modern C++ foundation

1. Package manager.
1. Networking (RPC, TCP, DPDK).
1. Storage (iouring support).
1. Logging.
1. Runtime (coroutine and scheduler).
1. Memory allocator.
1. Monitoring.
1. Tracing.
1. Configuration.
1. ...

### Build a key-value storage engine with strong schema support

TODO

## Build

Install [conan](https://conan.io/).

```
pip install conan
```

Then:

```
mkdir build && cd build
conan install .. --build=missing
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make
```

