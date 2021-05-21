# fastcall-benchmarks

Benchmarks of the [fastcall mechanism](https://github.com/vilaureu/linux/tree/fastcall).

This repository contains multiple benchmarks executables: _benchmark_ and _misc_.
Have a look at the directories with the same names.

## Dependencies

### Building

- _GCC_ or _Clang_
- _CMake_

### Benchmark Kernels

- [fastcall-enabled kernel](https://github.com/vilaureu/linux/tree/fastcall) with the fastcall-examples module loaded
- [kernel with fccmp](https://github.com/vilaureu/linux/tree/fccmp) with the fccmp module loaded

## Build

```
$ cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
$ cmake --build build/
```

## Libraries

fastcall-benchmarks uses following libraries:

- [_benchmark_](https://github.com/google/benchmark) by _Google Inc_ under the [_Apache-2.0 License_](https://github.com/google/benchmark/blob/master/LICENSE)
- [_parse_vdso.c_](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/testing/selftests/vDSO/parse_vdso.c?id=v5.11) by _Andrew Lutomirski_ under the [_Creative Commons Zero License, version 1.0_](http://creativecommons.org/publicdomain/zero/1.0/legalcode)
- [_Boost_](https://www.boost.org/) under the [_Boost Software License, Version 1.0_](https://www.boost.org/LICENSE_1_0.txt)

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
