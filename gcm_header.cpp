#include "gcm_header.h"

namespace gcm
{
  Header::Header(std::string file)
  {
    //  Read in each section of data from the file

    m_identifier = util::read(file, Header::Offset::ConsoleID, 6);

    m_disk_id = util::read_big<uint8_t>(file, Header::Offset::DiskID);
    m_version = util::read_big<uint8_t>(file, Header::Offset::Version);
    m_audio_streaming = util::read_big<uint8_t>(file, Header::Offset::AudoStreaming);
    m_stream_buffer_size = util::read_big<uint8_t>(file, Header::Offset::StreamBufferSize);

    m_magic = util::read_big<uint32_t>(file, Header::Offset::MagicWord);

    m_name = util::read(file, Header::Offset::Name, 0x3E0);

    m_debug_offset = util::read_big<uint32_t>(file, Header::Offset::DebugOffset);
    m_debug_load_addr = util::read_big<uint32_t>(file, Header::Offset::DebugAddress);

    m_dol_offset = util::read_big<uint32_t>(file, Header::Offset::DOLOffset);
    m_fst_offset = util::read_big<uint32_t>(file, Header::Offset::FSTOffset);
    m_fst_size = util::read_big<uint32_t>(file, Header::Offset::FSTSize);
    m_fst_max_size = util::read_big<uint32_t>(file, Header::Offset::FSTMaxSize);

    m_user_position = util::read_big<uint32_t>(file, Header::Offset::UserPosition);
    m_user_length = util::read_big<uint32_t>(file, Header::Offset::UserLength);

    m_unknown = util::read_big<uint32_t>(file, Header::Offset::Unknown);
    m_zero3 = util::read_big<uint32_t>(file, Header::Offset::Zero3);
  }

  /*
    Summary:
      Creates a completely formed header from previously read data

    Returns:
      The raw header data that was built from the current data in the class
  */
  std::vector<uint8_t> Header::raw()
  {
    std::vector<uint8_t> data;

    for (auto& c : m_identifier)
    {
      data.push_back(c);
    }

    data.push_back(m_disk_id);
    data.push_back(m_version);
    data.push_back(m_audio_streaming);
    data.push_back(m_stream_buffer_size);
    
    for (int i = 0; i < 0x12; i++)
    {
      data.push_back(0);
    }

    util::push_int_big<uint32_t>(data, m_magic);

    for (auto& c : m_name)
    {
      data.push_back(c);
    }

    util::push_int_big<uint32_t>(data, m_debug_offset);
    util::push_int_big<uint32_t>(data, m_debug_load_addr);

    for (int i = 0; i < 0x18; i++)
    {
      data.push_back(0);
    }

    util::push_int_big<uint32_t>(data, m_dol_offset);
    util::push_int_big<uint32_t>(data, m_fst_offset);
    util::push_int_big<uint32_t>(data, m_fst_size);
    util::push_int_big<uint32_t>(data, m_fst_max_size);

    util::push_int_big<uint32_t>(data, m_user_position);
    util::push_int_big<uint32_t>(data, m_user_length);

    util::push_int_big<uint32_t>(data, m_unknown);
    util::push_int_big<uint32_t>(data, m_zero3);

    return data;
  }
}