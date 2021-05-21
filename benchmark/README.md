# fastcall-benchmark

Benchmarks for the comparison of the [fastcall mechanism](https://github.com/vilaureu/linux/tree/fastcall) with system calls, ioctl etc.
These benchmarks use the [_benchmark_](https://github.com/google/benchmark) library.

## Dependencies

- [_benchmark_](https://github.com/google/benchmark)

## Usage

To benchmark the fastcall mechanism _(requires the fastcall-enabled kernel)_:

`$ ./build/fastcall-benchmarks --benchmark_filter=fastcall`

To benchmark system calls _(requires the kernel with fccmp)_:

`$ ./build/fastcall-benchmarks --benchmark_filter=syscall`

To benchmark `ioctl` calls _(requires the kernel with fccmp)_:

`$ ./build/fastcall-benchmarks --benchmark_filter=ioctl`

To benchmark _vDSO_ calls _(requires the kernel with fccmp)_:

`$ ./build/fastcall-benchmarks --benchmark_filter=vdso`
