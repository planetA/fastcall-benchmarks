# fastcall-benchmarks

Benchmarks for the comparison of the [fastcall mechanism](https://github.com/vilaureu/linux/tree/fastcall) with system calls, ioctl etc.

## Dependencies

### Building

- _GCC_ or _Clang_
- _CMake_
- [_benchmark_](https://github.com/google/benchmark)

### Benchmark Kernels

- [fastcall-enabled kernel](https://github.com/vilaureu/linux/tree/fastcall) with the fastcall-examples module loaded

## Build

```
$ cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release`
$ cmake --build build/
```

## Usage

To benchmark system calls:

`$ ./build/fastcall-examples --benchmark_filter=syscall`

To benchmark the fastcall mechanism _(requires the fastcall-enabled kernel)_:

`$ ./build/fastcall-examples --benchmark_filter=fastcall`

## Licence

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
