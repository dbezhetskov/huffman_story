#ifndef ENCRYPTION_CHAR_STREAMS_ADAPTERS_CHAR_OSTREAM_ADAPTER_H_
#define ENCRYPTION_CHAR_STREAMS_ADAPTERS_CHAR_OSTREAM_ADAPTER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>

#include "bit_io/bit_writer.h"

namespace {
constexpr uint32_t kBufferSizeInBytes = 1024u * 1024u;
}

namespace char_adapters {

class CharAlignedBitWriter final : public bit_io::BitWriter {
 public:
  explicit CharAlignedBitWriter(std::shared_ptr<std::ostream> ostream);
  ~CharAlignedBitWriter() override;

  bool WriteBit(bool enabled) override;

 private:
  void FlushBuffer();
  void WriteFooter();

  // TODO: can be replaced with fixed size bitset that can converts into char* array.
  std::array<std::byte, kBufferSizeInBytes> buffer_;
  uint32_t bit_idx_{0u};
  bool has_bits_written_{false};
  uint8_t num_of_filled_bits_in_last_byte_{0u};
  std::shared_ptr<std::ostream> underlying_writer_;
};

}  // namespace char_adapters

#endif  // ENCRYPTION_CHAR_STREAMS_ADAPTERS_CHAR_OSTREAM_ADAPTER_H_
