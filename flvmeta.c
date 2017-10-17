#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define AV_RB8(x) (((const uint8_t *)(x))[0])

// clang-format off
#define AV_RB24(x)                         \
    ((((const uint8_t*)(x))[0] << 16) |    \
     (((const uint8_t*)(x))[1] <<  8) |    \
      ((const uint8_t*)(x))[2])

#define AV_RB32(x)                                   \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])
// clang-format on

#define FLV_HEADER "FLV\x1\x5\0\0\0\x9"
#define FLV_AUDIO_SOUND_FORMAT_AAC 10
#define FLV_VIDEO_CODECID_AVC 7

enum FlvTagType {
  FLV_TAG_TYPE_AUDIO = 0x08,
  FLV_TAG_TYPE_VIDEO = 0x09,
  FLV_TAG_TYPE_META  = 0x12,
};

// extract bits from 1 byte, big endian
// @byte: 1 byte to extract
// @start: start from the low bit side
// @n: number of bits
static inline uint8_t extract_bits(uint8_t byte, uint8_t start, uint8_t n) {
  uint8_t mask = 0;

  mask = ((1 << n) - 1) << start;
  return (byte & mask) >> start;
}

static int flv_parse_tag(const uint8_t *buf, const size_t buflen,
                         uint32_t offset, uint32_t *data_size) {

  ++offset; // skip tag type

  if (offset + 3 > buflen) {
    return 0;
  }

  *data_size = AV_RB24(buf + offset);
  offset += 10 + *data_size;

  // complete tag
  return offset <= buflen;
}

static int flv_parse_audio_tag(const uint8_t *buf, const size_t buflen,
                               uint32_t *offset) {
  uint32_t data_size;
  uint32_t data_offset;
  uint8_t sound_format;

  if (!flv_parse_tag(buf, buflen, *offset, &data_size) || data_size == 0) {
    return 0;
  }

  data_offset  = *offset + 11;
  sound_format = extract_bits(AV_RB8(buf + data_offset), 4, 4);
  if (sound_format != FLV_AUDIO_SOUND_FORMAT_AAC) {
    return 0;
  }

  if (data_size < 2) {
    return 0;
  }

  // AAC sequence header
  if (AV_RB8(buf + data_offset + 1) == 0) {
    *offset = data_offset + data_size;
    return 1;
  }

  return 0;
}

static int flv_parse_video_tag(const uint8_t *buf, const size_t buflen,
                               uint32_t *offset) {
  uint32_t data_size;
  uint32_t data_offset;
  uint8_t codec_id;

  if (!flv_parse_tag(buf, buflen, *offset, &data_size) || data_size == 0) {
    return 0;
  }

  data_offset = *offset + 11;
  codec_id    = extract_bits(AV_RB8(buf + data_offset), 0, 4);

  if (codec_id != FLV_VIDEO_CODECID_AVC) {
    return 0;
  }

  // AVC sequence header
  if (AV_RB8(buf + data_offset + 1) == 0) {
    *offset = data_offset + data_size;
    return 1;
  }

  return 0;
}

static int flv_parse_meta_tag(const uint8_t *buf, const size_t buflen,
                              uint32_t *offset) {
  uint32_t data_size;
  if (flv_parse_tag(buf, buflen, *offset, &data_size)) {
    *offset += 11 + data_size;
    return 1;
  }

  return 0;
}

// metadata tag and sequence header tag
static int flv_parse_important_tag(const uint8_t *buf, const size_t buflen,
                                   uint32_t *offset) {
  uint8_t type;

  if (*offset + 1 > buflen) {
    return 0;
  }

  type = AV_RB8(buf + *offset);

  switch (type) {
  case FLV_TAG_TYPE_AUDIO:
    return flv_parse_audio_tag(buf, buflen, offset);
  case FLV_TAG_TYPE_VIDEO:
    return flv_parse_video_tag(buf, buflen, offset);
  case FLV_TAG_TYPE_META:
    return flv_parse_meta_tag(buf, buflen, offset);
  }

  return 0;
}

int64_t flv_metadata_offset(const uint8_t *buf, const size_t buflen) {
  uint32_t offset;

  if (buflen < sizeof(FLV_HEADER)) {
    return -__LINE__;
  }

  offset = AV_RB32(buf + 5);

  if (!(buf[0] == 'F' && buf[1] == 'L' && buf[2] == 'V' && offset > 8 &&
        offset + 4 < buflen)) {
    return -__LINE__;
  }

  // PreviousTagSize0, Always 0
  if (AV_RB32(buf + offset) != 0) {
    return -__LINE__;
  }

  offset += 4;

  for (; flv_parse_important_tag(buf, buflen, &offset);) {
    if (offset + 4 > buflen) {
      return -__LINE__;
    }
    // PreviousTagSize
    offset += 4;
  }

  return offset;
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
