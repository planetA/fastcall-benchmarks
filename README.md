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

## Libraries

fastcall-benchmarks uses following libraries:

- [_benchmark_](https://github.com/google/benchmark) by _Google Inc_ under the [_Apache-2.0 License_](https://github.com/google/benchmark/blob/master/LICENSE)
- [_parse_vdso.c_](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/testing/selftests/vDSO/parse_vdso.c?id=v5.11) by _Andrew Lutomirski_ under the [_Creative Commons Zero License, version 1.0_](http://creativecommons.org/publicdomain/zero/1.0/legalcode)

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
