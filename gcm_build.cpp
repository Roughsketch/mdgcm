#include "gcm.h"

using namespace fst;

namespace fs = boost::filesystem;

namespace gcm
{
  /*
    Summary:
      Builds a GCM from the contents of a directory which has files previously extracted

    Parameter:
      root: Directory where the ./files and ./sys directories are
      outfile: Output path for the GCM file
  */
  void build(std::string root, std::string outfile)
  {
    std::string syspath = root + "/sys/";
    std::string filepath = root + "/files/";

    //  Create the header from the extracted file
    Header header(syspath + "header.bin");

    //  DOL offset is 0x2440 + apploader size
    uint32_t doloffset = static_cast<uint32_t>(0x2440 + boost::filesystem::file_size(syspath + "apploader.bin"));
    //  DOL padding to an even 4 byte boundary
    uint32_t dolpad = util::pad(doloffset, 4);

    //  FST is after the DOL program
    uint32_t fstoffset = static_cast<uint32_t>(doloffset + dolpad + boost::filesystem::file_size(syspath + "main.dol"));
    //  FST padding to an even 4 byte boundary
    uint32_t fstpad = util::pad(fstoffset, 4);

    //  Create a new FST from the files under the ./files directory
    FST fst(filepath, fstoffset + fstpad);

    //  Set the correct new data in the header
    header.set_fst_size(fst.rawsize());         //  FST size
    header.set_fst_offset(fstoffset + fstpad);  //  FST offset
    header.set_dol_offset(doloffset + dolpad);  //  DOL offset

    //  Write out each binary portion of the disc
    util::write_file(outfile, header.raw());
    util::append_file(outfile, util::read_file(syspath + "bi2.bin"));
    util::append_file(outfile, util::read_file(syspath + "apploader.bin"));
    util::append_file(outfile, util::read_file(syspath + "main.dol"), 0, doloffset + dolpad);
    util::append_file(outfile, fst.raw(), 0, fstoffset + fstpad);

    //  Begin writing each file to the disc
    for (auto& file : fst.files())
    {
      //  If file has actual content append it to the disc
      if (file.size() > 0)
      {
        std::cout << "Writing " << file.path() << std::endl;
        util::append_file(outfile, util::read_file(file.path()), 0, file.offset());
      }
    }
  }

  /*
    Summary:
      Determines whether a given root directory has the required structure and files to be used to create a GCM

    Parameter:
      root: Directory to check

    Returns:
      True if the directory can be used to create a GCM, or false otherwise.
  */
  bool valid_directory(std::string root)
  {
    bool ret = true;
    //  If the directory does not have /files and /sys then it wasn't extracted by this extractor
    if (fs::is_directory(root) == false)
    {
      std::cout << root << " is not a valid directory." << std::endl;
      ret = false;
    }

    if (fs::is_directory(root + "/files") == false)
    {
      std::cout << "Missing directory /files under " << root << std::endl;
      ret = false;
    }

    if (fs::is_directory(root + "/sys") == false)
    {
      std::cout << "Missing directory /sys under " << root << std::endl;
      ret = false;
    }

    if (fs::is_regular_file(root + "/sys/apploader.bin") == false)
    {
      std::cout << "Missing file " << root << "/sys/apploader.bin" << std::endl;
      ret = false;
    }

    if (fs::is_regular_file(root + "/sys/bi2.bin") == false)
    {
      std::cout << "Missing file " << root << "/sys/bi2.bin" << std::endl;
      ret = false;
    }

    if (fs::is_regular_file(root + "/sys/fst.bin") == false)
    {
      std::cout << "Missing file " << root << "/sys/fst.bin" << std::endl;
      ret = false;
    }

    if (fs::is_regular_file(root + "/sys/header.bin") == false)
    {
      std::cout << "Missing file " << root << "/sys/header.bin" << std::endl;
      ret = false;
    }

    if (fs::is_regular_file(root + "/sys/main.dol") == false)
    {
      std::cout << "Missing file " << root << "/sys/main.dol" << std::endl;
      ret = false;
    }

    return ret;
  }
}