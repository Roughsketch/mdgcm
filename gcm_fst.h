#ifndef _FST_H
#define _FST_H

#include <cstdint>
#include <algorithm>
#include <vector>
#include <map>
#include <iterator>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"

namespace fst
{
  const uint32_t NodeSize = 0x0C;
  struct Node
  {
    Node() : m_type_string_offset(0), m_file_parent_offset(0), m_size_next_offset(0) {};

    Node(bool file, uint32_t str_off, uint32_t fp_off, uint32_t size_nextoff)
    {
      m_type_string_offset = (file ? 0 : 1) << 24;
      m_type_string_offset |= str_off & 0x00FFFFFF;
      m_file_parent_offset = fp_off;
      m_size_next_offset = size_nextoff;
    }

    Node(std::vector<uint8_t>& data);

    inline uint32_t type()
    {
      return (m_type_string_offset & 0xFF000000) >> 24;
    }

    inline bool is_file()
    {
      return type() == 0;
    }

    inline bool is_dir()
    {
      return type() == 1;
    }

    inline uint32_t string_offset()
    {
      return m_type_string_offset & 0x00FFFFFF;
    }

    inline uint32_t data_offset()
    {
      return m_file_parent_offset;
    }

    inline uint32_t data_size()
    {
      return m_size_next_offset;
    }

    inline uint32_t next_offset()
    {
      return m_size_next_offset;
    }

    inline uint32_t total_entries()
    {
      return m_size_next_offset;
    }
  private:
    uint32_t m_type_string_offset;  //  & 0xFF000000 = file type
                                    //  & 0x00FFFFFF = string offset
    uint32_t m_file_parent_offset;  //  File offset or Parent offset
    uint32_t m_size_next_offset;    //  File size or Next offset (number of entries if root)
  };

  struct FileData
  {
    FileData(std::string path, uint32_t size, uint32_t offset)
    {
      m_path = path;
      m_filesize = size;
      m_fileoffset = offset;
    }

    inline std::string path()
    {
      return m_path;
    }

    inline uint32_t size()
    {
      return m_filesize;
    }

    inline uint32_t offset()
    {
      return m_fileoffset;
    }
  private:
    std::string m_path;
    uint32_t m_filesize;
    uint32_t m_fileoffset;
  };

  struct FST
  {
    FST() : m_root(), m_strtable_size(0), m_file_offset(0) {};
    FST(std::string root, uint32_t fst_offset);
    FST(std::vector<uint8_t>& data);

    inline std::map<std::string, Node> entries()
    {
      return m_nodes;
    }

    inline std::vector<uint8_t> raw()
    {
      return m_raw;
    }

    inline std::vector<FileData> files()
    {
      return m_files;
    }

    inline uint32_t size()
    {
      return calculate_size(m_root_path);
    }

    inline uint32_t rawsize()
    {
      return m_raw.size() - m_padding;
    }
  private:
    std::string m_root_path;              //  Store the root path provided if using the FST(std::string, uint32_t) constructor
    Node m_root;                          //  Store the root node separately 
    std::map<std::string, Node> m_nodes;  //  Store each node indexed by its path

    uint32_t m_strtable_size; //  Size of the entire string table
    uint32_t m_file_offset;   //  The current file offset used when adding a file to the FST
    uint32_t m_padding;       //  Total padding used to align the end of the FST to a 0x100 byte boundary

    std::vector<uint8_t> m_raw;           //  The raw FST data
    std::vector<std::string> m_strtable;  //  String table entries without their null padding

    //  Stores information like path, file size, and file data offset
    std::vector<FileData> m_files;

    std::string compact_path(std::vector<std::pair<std::string, uint32_t>>& path);
    uint32_t calculate_size(std::string root);


    inline uint32_t count_entries(std::string path)
    {
      return std::distance(boost::filesystem::recursive_directory_iterator(path),
                           boost::filesystem::recursive_directory_iterator());
    }
  };
}

#endif
