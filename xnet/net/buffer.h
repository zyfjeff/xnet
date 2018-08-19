#ifndef XNET_NET_BUFFER_H_
#define XNET_NET_BUFFER_H_

#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>

#include "absl/strings/string_view.h"
#include "xnet/base/macros.h"

namespace xnet {
namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode


class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  Buffer();
  ~Buffer();
  
  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(reader_index_, rhs.reader_index_);
    std::swap(writer_index_, rhs.writer_index_);
  }

  size_t ReadableBytes() const {
    return writer_index_ - reader_index_;
  }

  size_t WriteableBytes() const {
    return buffer_.size() - writer_index_; 
  }

  size_t PrependableBytes() const {
    return reader_index_;
  }

  const char* Peek() const {
    return Begin() + reader_index_;
  }

  void Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    reader_index_ += len;
  }

  void RetrieveUntil(const char* end) {
    assert(Peek() <= end);
    assert(end <= BeginWrite());
    Retrieve(end - Peek());
  }

  void RetrieveAll() {
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
  }

  std::string RetrieveAsString() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
  }

  void Append(absl::string_view str) {
    EnsureWritableBytes(str.size());
    std::copy(str.data(), str.data() + str.size(), BeginWrite());
    HasWritten(str.size());
  }

  void EnsureWritableBytes(size_t len) {
    if (WriteableBytes() < len) {
      MakeSpace(len);
    }

    assert(WriteableBytes() >= len);
  }

  char* BeginWrite() {
    return Begin() + writer_index_;
  }

  const char* BeginWrite() const {
    return Begin() + writer_index_;
  }

  void Prepend(absl::string_view str) {
    assert(str.size() <= PrependableBytes());
    reader_index_ -= str.size();
    const char* d = static_cast<const char*>(str.data());
    std::copy(d, d + str.size(), Begin() + reader_index_);
  }

  void Shrink(size_t reserve) {
    std::vector<char> buf(kCheapPrepend + ReadableBytes() + reserve);
    std::copy(Peek(), Peek() + ReadableBytes(), buf.begin() + kCheapPrepend);
    buf.swap(buffer_);
  }

  void HasWritten(size_t len) {
    writer_index_ += len;
  }

  ssize_t ReadFd(int fd, int* saved_errno);

 private:
  char* Begin() { return &*buffer_.begin(); }
  const char* Begin() const { return &*buffer_.begin(); }

  void MakeSpace(size_t len);


  std::vector<char> buffer_;
  size_t reader_index_;
  size_t writer_index_;
  
};

}  // namespace net
}  // namespace xnet

#endif  // XNET_NET_BUFFER_H_