#include "fastcall.h"
#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace fce {

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
 * A file descriptor exception.
 */
class FDError : public Error {
public:
  FDError(int nr) : Error(get_msg(nr)) {}

private:
  /*
   * Create a error message from the error number nr.
   */
  static std::string get_msg(int nr) {
    std::stringstream st{};
    st << "failed to open device driver: " << std::strerror(nr);
    return st.str();
  }
};

/*
 * Manage a file descriptor with the RAII principle.
 */
class FileDescriptor {
public:
  FileDescriptor() : fd{open(fce::DEVICE_FILE, O_RDONLY)} {
    if (fd < 0) {
      throw FDError(errno);
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

} // namespace fce