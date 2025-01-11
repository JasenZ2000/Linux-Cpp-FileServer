#include "Buffer.h"

void Buffer::set_buf(const char *buf) {
  std::string new_buf(buf);
  buf_.swap(new_buf);
}

void Buffer::Append(const char *str, int size) {
  for (int i = 0; i < size; ++i) {
    if (str[i] == '\0') {
      break;
    }
    buf_.push_back(str[i]);
  }
}
