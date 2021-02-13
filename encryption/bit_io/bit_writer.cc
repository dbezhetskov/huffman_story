#include "encryption/bit_io/bit_writer.h"

#include <climits>
#include <cstdint>

#include "bits_manipulation/bits_manipulation.h"

namespace bit_io {

void BitWriter::WriteByte(std::byte byte) {
  for (uint32_t bit_pos = 0; bit_pos < CHAR_BIT; ++bit_pos) {
    WriteBit(bits_manipulation::IsBitEnabled(byte, bit_pos));
  }
}

}  // namespace bit_io