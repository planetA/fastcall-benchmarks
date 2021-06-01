# fastcall-misc

Some miscellaneous benchmarks for the [fastcall mechanism](https://github.com/vilaureu/linux/tree/fastcall).

## Dependencies

- [_Boost_](https://www.boost.org/) (_program_options_ library)

## Usage

The executable prints just a list of the measured nanoseconds.

To make a benchmark without any code in the timed section:

`$ ./build/misc/fastcall-misc noop`

To perform some (de)registration benchmarks:

`$ ./build/misc/fastcall-misc <registration-minimal|registration-mappings|deregistration-minimal|deregistration-mappings>`

Finally, to get some `fork` and `vfork` timings (also without fastcall):

`$ ./build/misc/fastcall-misc <fork-simple|fork-fastcall|vfork-simple|vfork-fastcall>`
