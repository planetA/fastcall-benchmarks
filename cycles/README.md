# fastcall-cycles

A set of cycle-based benchmarks for the
[fastcall mechanism](https://github.com/vilaureu/linux/tree/fastcall).

## Dependencies

- [_Boost_](https://www.boost.org/) (_program_options_ library)

## Usage

The executable prints just a list of the measured cycle count.

To make a benchmark without any code in the timed section:

`$ ./build/cycles/fastcall-cycles noop`

To perform the noop fastcall benchmark:

`$ ./build/cycles/fastcall-cycles fastcall`

To get comparative values using _fccmp_:

`$ ./build/cycles/fastcall-cycles <vdso|syscall|ioctl>`
