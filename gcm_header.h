#ifndef _GCM_HEADER_H
#define _GCM_HEADER_H

#include "gcm.h"

namespace gcm
{
  struct Header
  {
    Header() {};
    Header(std::string file);

    std::vector<uint8_t> raw();

    inline void set_fst_offset(uint32_t value)
    {
      m_fst_offset = value;
    }

    inline void set_fst_size(uint32_t value)
    {
      m_fst_size = value;
      m_fst_max_size = value;
    }

    inline void set_dol_offset(uint32_t value)
    {
      m_dol_offset = value;
    }

    enum Offset
    {
      ConsoleID = 0x000,
      Gamecode = 0x001,
      CountryCode = 0x003,
      MakerCode = 0x004,
      DiskID = 0x006,
      Version = 0x007,
      AudoStreaming = 0x008,
      StreamBufferSize = 0x009,
      Zero1 = 0x00A,
      MagicWord = 0x01C,
      Name = 0x020,
      DebugOffset = 0x400,
      DebugAddress = 0x404,
      Zero2 = 0x408,
      DOLOffset = 0x420,
      FSTOffset = 0x424,
      FSTSize = 0x428,
      FSTMaxSize = 0x42C,
      UserPosition = 0x430,
      UserLength = 0x434,
      Unknown = 0x438,
      Zero3 = 0x43C
    };
  private:
    std::string m_identifier;
    uint8_t m_disk_id;
    uint8_t m_version;
    uint8_t m_audio_streaming;
    uint8_t m_stream_buffer_size;

    uint32_t m_magic;
    std::string m_name;

    uint32_t m_debug_offset;
    uint32_t m_debug_load_addr;

    uint32_t m_dol_offset;
    uint32_t m_fst_offset;
    uint32_t m_fst_size;
    uint32_t m_fst_max_size;

    uint32_t m_user_position;
    uint32_t m_user_length;

    uint32_t m_unknown;
    uint32_t m_zero3;

  };
}
#endif