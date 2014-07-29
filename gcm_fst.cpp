#include "gcm_fst.h"

namespace fs = boost::filesystem;

namespace fst
{
  Node::Node(std::vector<uint8_t>& data)
  {
    m_type_string_offset = util::read_big<uint32_t>(data);
    m_file_parent_offset = util::read_big<uint32_t>(data, 4);
    m_size_next_offset = util::read_big<uint32_t>(data, 8);
  }

  FST::FST(std::vector<uint8_t>& data)
  {
    m_root = Node(data);
    uint32_t string_start = m_root.total_entries() * NodeSize;

    //  Start the current path at root (".")
    std::vector<std::pair<std::string, uint32_t>> path = { std::make_pair(".", -1) };

    for (uint32_t i = 1; i < m_root.total_entries(); i++)
    {
      //  Remove all directory names that are past their range
      path.erase(std::remove_if(path.begin(), path.end(), [i](std::pair<std::string, uint32_t>& x){return i >= x.second; }), path.end());

      Node next = Node(util::subset(data, i * NodeSize, NodeSize));
      std::string name = util::read(data, string_start + next.string_offset());
      std::string fullpath = compact_path(path);

      if (next.is_dir())
      {
        //  Push this directory name onto the list
        path.push_back(std::make_pair(name, next.next_offset()));
      }
      else
      {
        //  Push file entry into vector
        m_files.push_back(FileData(fullpath + name, next.data_size(), next.data_offset()));
      }

      //  Put data into nodes indexed by the absolute path
      m_nodes[fullpath + name] = next;
    }
  }

  FST::FST(std::string root, uint32_t fst_offset)
  {
    fs::recursive_directory_iterator dir(root), end;

    //  Set the start offset where file data is stored and give it some even padding
    m_file_offset = fst_offset + calculate_size(root);
    m_padding = util::pad(m_file_offset, 0x100);  //  Pad the FST to an even 0x100 byte boundary
    m_file_offset += m_padding; //  Add the padding

    //m_file_offset += (m_file_offset % 16) + 16;
    m_strtable_size = 0;

    //  Store original root path
    m_root_path = root;

    //  current_parent.back() holds the latest parent index which is pruned each iteration
    std::vector<std::pair<uint32_t, uint32_t>> current_parent = { std::make_pair(0, 0) };

    //  Create root node
    util::push_int_big<uint32_t>(m_raw, 0x1000000); //  Set as directory
    util::push_int_big<uint32_t>(m_raw, 0);         //  Parent is 0
    util::push_int_big<uint32_t>(m_raw, count_entries(root) + 1); //  Next offset is end of all entries

    //  Start at one to skip root index (above)
    uint32_t current_index = 1;

    //  Loop through every file and directory entry recursively
    while (dir != end)
    {
      //  Get the name of the path relative to the root directory
      std::string temp = dir->path().string().substr(root.length());
      boost::replace_all(temp, "\\", "/");

      std::string path = "./" + temp;

      //  Remove any directory indexes that are past their range
      current_parent.erase(std::remove_if(current_parent.begin(), current_parent.end(),
        [current_index](std::pair<uint32_t, uint32_t>& x)
        {
          //  Remove if the current index is greater than the directory range and isn't root (0)
          return (current_index >= x.second) && (x.first != 0); 
        }), current_parent.end());

      //  If this entry is a file
      if (fs::is_regular_file(dir->path()))
      {
        uint32_t filesize = static_cast<uint32_t>(fs::file_size(dir->path()));

        util::push_int_big<uint32_t>(m_raw, m_strtable_size & 0x00FFFFFF);  //  String table offset is 3 bytes. Upper byte is always 0 for files.
        util::push_int_big<uint32_t>(m_raw, m_file_offset);                 //  File data offset into disc
        util::push_int_big<uint32_t>(m_raw, filesize);                      //  File data length / File size

        //  Push the path as well as the file size and file offset into a vector for easy use later.
        m_files.push_back(FileData(dir->path().string(), filesize, m_file_offset));

        //  Increase the total file offset by the file size
        m_file_offset += filesize;

        //  Pad the next file entry to the next 16 byte boundary
        m_file_offset += util::pad(m_file_offset, 0x10);
      }
      else
      {
        uint32_t entries = count_entries(dir->path().string());
        uint32_t next_index = current_index + entries + 1;

        //  Upper byte is 1 to signal it's a directory. Last 3 bytes are the string table offset.
        util::push_int_big<uint32_t>(m_raw, 0x01000000 | (m_strtable_size & 0x00FFFFFF));

        //  Set the parent offset to the current parent's index
        util::push_int_big<uint32_t>(m_raw, current_parent.back().first);

        //  Set the next index to the end of this directories entries
        util::push_int_big<uint32_t>(m_raw, next_index);

        //  Set this directory as the current parent
        current_parent.push_back(std::make_pair(current_index, next_index));
      }

      //  Push back the name of the file or directory and increment the total size
      m_strtable.push_back(dir->path().filename().string());
      m_strtable_size += dir->path().filename().string().length() + 1;

      //  Go to next directory and increase the current index
      ++dir;
      ++current_index;
    }

    //  Compile all the strings into a valid string table
    std::vector<uint8_t> rawstr;

    for (auto& name : m_strtable)
    {
      for (auto& c : name)
      {
        rawstr.push_back(c);
      }
      rawstr.push_back(0);  //  Add a null padding to the end of each string
    }

    //  Add the padding at the end of the FST so data lines up nicely.
    for (uint32_t i = 0; i < m_padding; i++)
    {
      rawstr.push_back(0);
    }

    //  Add the string table to the end of the FST entries
    m_raw.insert(m_raw.end(), rawstr.begin(), rawstr.end());
  }

  /*
    Summary:
      Compacts a vector of strings into a full path. Currently a highly specialized for this class function.

    Parameters:
      std::vector<std::pair<std::string, uint32_t>>& path: Vector to use

    Returns:
      A string containing the strings combined like a file path
  */
  std::string FST::compact_path(std::vector<std::pair<std::string, uint32_t>>& path)
  {
    std::string ret;

    for (auto& pair : path)
    {
      ret += pair.first + "/";
    }
    return ret;
  }

  /*
    Summary: 
      Calculates the total end size of the FST without padding given a root directory

    Parameters:
      std::string root: The directory that is used as the root of the FST (usually the ./files directory)

    Returns:
      uint32_t with the total calculated size
  */
  uint32_t FST::calculate_size(std::string root)
  {
    fs::recursive_directory_iterator dir(root), end;

    //  Start the size at the total size for all nodes which is (File + Directory count + 1) * NodeSize
    uint32_t total_size = (count_entries(root) + 1) * NodeSize;

    while (dir != end)
    {
      //  Add the string length + 1 for the null padding
      total_size += dir->path().filename().string().length() + 1;
      ++dir;
    }

    return total_size;
  }
}