#include "fastcall.h"
#include <array>
#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace fce {

static const unsigned FORK_FASTCALL_COUNT = 100;

/*
 * Base exception for the fce namespace.
 */
class Error : public std::exception {
public:
  Error(std::string msg) : msg{msg} {}
  virtual const char *what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

/*
 * An error described by an errno.
 */
template <const char *Prefix> class ErrnoError : public Error {
public:
  ErrnoError(int nr) : Error(get_msg(nr)) {}

private:
  /*
   * Create a error message from the error number nr.
   */
  static std::string get_msg(int nr) {
    std::stringstream st{};
    st << Prefix << ": " << std::strerror(nr);
    return st.str();
  }
};

const char FDMsg[]{"failed to open device driver"};

/*
 * Manage a file descriptor with the RAII principle.
 */
class FileDescriptor {
public:
  FileDescriptor() : fd{open(fce::DEVICE_FILE, O_RDONLY)} {
    if (fd < 0) {
      throw ErrnoError<FDMsg>(errno);
    }
  }
  ~FileDescriptor() {
    if (close(fd) < 0)
      std::cerr << "failed to close device driver: " << std::strerror(errno)
                << '\n';
  }

  /*
   * ioctl function for the wrapped file descriptor.
   */
  int io(unsigned type, void *args) { return ioctl(fd, type, args); }

private:
  int fd;
};

const char MunmapMsg[]{"fce munmap failed"};

template <typename Args> static void deregister(Args &args) {
  if (munmap(reinterpret_cast<void *>(args.fn_addr), args.fn_len) < 0) {
    throw ErrnoError<MunmapMsg>{errno};
  }
}

const char IOCTLError[]{"ioctl failed"};

/*
 * Registers a lot of fastcalls until deconstruction.
 */
class ManyFastcalls {
public:
  ManyFastcalls() {
    for (auto &args : args_array) {
      int err = fd.io(fce::IOCTL_ARRAY, &args);
      if (err < 0) {
        throw ErrnoError<IOCTLError>{errno};
      }
    }
  }
  ~ManyFastcalls() {
    try {
      for (auto &args : args_array)
        fce::deregister(args);
    } catch (Error &e) {
      std::cerr << e.what() << '\n';
    }
  }

private:
  fce::FileDescriptor fd{};
  std::array<fce::array_args, FORK_FASTCALL_COUNT> args_array{};
};

} // namespace fce
