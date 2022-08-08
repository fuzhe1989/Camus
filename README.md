# Hafnium (hf)

[Hafnium](https://en.wikipedia.org/wiki/Hafnium) is a chemical element. It's chinese name is "铪" ^_^.

Hf is an experimental project.

## Motivation

1. Build applications by leveraging modern C++ (coroutine, ranges, execution, etc.) techniques.
2. Build a set of high-performance data-oriented systems from scratch.

## The modern C++ foundation

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

## The toy applications

Currently only one application is planning.

### Build a key-value storage engine with strong schema support

The first version will be:
1. built on RocksDB.
1. single node.
