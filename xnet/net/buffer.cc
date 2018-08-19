#include "xnet/net/buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <memory.h>

namespace xnet {
namespace net {


Buffer::Buffer() : buffer_(kCheapPrepend + kInitialSize),
                   reader_index_(kCheapPrepend),
                   writer_index_(kCheapPrepend) {
  assert(ReadableBytes() == 0);
  assert(WriteableBytes() == kInitialSize);
  assert(PrependableBytes() == kCheapPrepend);
}

Buffer::~Buffer() {}

ssize_t Buffer::ReadFd(int fd, int* saved_errno) {
  char extra_buffer[65535] = {0};
  struct iovec vec[2];
  const size_t writerable = WriteableBytes();
  vec[0].iov_base = Begin() + writer_index_;
  vec[0].iov_len = writerable;
  vec[1].iov_base = extra_buffer;
  vec[1].iov_len = sizeof extra_buffer;

  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writerable) {
    writer_index_ += n;
  } else {
    writer_index_ = buffer_.size();
    Append(absl::string_view(extra_buffer, n - writerable));
  }
  return n;
}

void Buffer::MakeSpace(size_t len) {
  if (WriteableBytes() + PrependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writer_index_ + len);
  } else {
    assert(kCheapPrepend < reader_index_);
    size_t readable = ReadableBytes();
    std::copy(Begin() + reader_index_,
              Begin() + writer_index_,
              Begin() + kCheapPrepend);
    reader_index_ = kCheapPrepend;
    writer_index_ = reader_index_ + readable;
    assert(readable == ReadableBytes());
  }
}


}  // namespace net
}  // namespace xnet