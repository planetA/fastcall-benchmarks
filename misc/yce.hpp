#include "ycall.hpp"
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

namespace yce {

static const unsigned FORK_YCALL_COUNT = 100;

/*
 * Base exception for the yce namespace.
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
const char IOCTLError[]{"ioctl failed"};
const char DeregError[]{"yce deregistration failed"};

/*
 * Manage a file descriptor with the RAII principle.
 */
class FileDescriptor {
public:
  FileDescriptor() : fd{open(yce::DEVICE_FILE, O_RDONLY)} {
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
  void io(unsigned type, void *args) {
    if (ioctl(fd, type, args) < 0) {
      throw ErrnoError<IOCTLError>{errno};
    }
  }

  void deregister(struct reg_args *args) {
    if (ioctl(fd, YCE_IOCTL_DEREGISTRATION, args) < 0) {
      throw ErrnoError<DeregError>{errno};
    }
  }

private:
  int fd;
};

const char MunmapMsg[]{"yce munmap failed"};

/*
 * Registers a lot of fastcalls until deconstruction.
 */
class ManyFastcalls {
public:
  ManyFastcalls() {
    for (auto &args : args_array)
      fd.io(YCE_IOCTL_REGISTRATION, &args);
  }
  ~ManyFastcalls() {
    try {
      for (auto &args : args_array)
        fd.deregister(&args);
    } catch (Error &e) {
      std::cerr << e.what() << '\n';
    }
  }

private:
  yce::FileDescriptor fd{};
  std::array<yce::reg_args, FORK_YCALL_COUNT> args_array{};
};

} // namespace yce

