#include "gcm.h"

namespace gcm
{

  /*
    Summary:
      Extracts the Apploader data from the disc

    Parameters:
      disc: Path to the disc to read from
      out_directory: Directory where files will be extracted to
  */
  void extract_app(std::string disc, std::string out_directory)
  {
    uint32_t appoffset = 0x2440;  //  Always after header and bi2

    uint32_t appsize = 0;

    appsize += util::read_big<uint32_t>(disc, 0x2454);  //  Apploader size
    appsize += util::read_big<uint32_t>(disc, 0x2458);  //  Trailer size
    appsize += util::pad(appsize, 0x100);               //  Pad it to a 0x100 byte boundary

    std::vector<uint8_t> appbin = util::read_file(disc, appsize, appoffset);

    util::write_file(out_directory + "apploader.bin", appbin);
  }

  /*
    Summary:
      Extracts the FST data from the disc

    Parameters:
      disc: Path to the disc to read from
      out_directory: Directory where files will be extracted to
  */
  void extract_fst(std::string disc, std::string out_directory)
  {
    uint32_t fstsize = util::read_big<uint32_t>(disc, Header::Offset::FSTSize);
    uint32_t fstoffset = util::read_big<uint32_t>(disc, Header::Offset::FSTOffset);

    std::vector<uint8_t> fstbin = util::read_file(disc, fstsize, fstoffset);

    util::write_file(out_directory + "fst.bin", fstbin);
  }

  /*
    Summary:
      Extracts the DOL binary from the disc

    Parameters:
      disc: Path to the disc to read from
      out_directory: Directory where files will be extracted to
  */
  void extract_dol(std::string disc, std::string out_directory)
  {
    uint32_t doloffset = util::read_big<uint32_t>(disc, Header::Offset::DOLOffset);

    uint32_t dolsize = 0x100;
    std::vector<uint8_t> sizes = util::read_file(disc, 0x48, doloffset + 0x90);

    for (int i = 0; i < 0x48; i += 4)
    {
      dolsize += util::read_big<uint32_t>(sizes, i);
    }

    std::vector<uint8_t> dolbin = util::read_file(disc, dolsize, doloffset);

    util::write_file(out_directory + "main.dol", dolbin);
  }

  /*
    Summary:
      Extracts files from a disc to a given directory

    Parameters:
      disc: Path to the disc to read from
      out_directory: Directory where files will be extracted to
  */
  void extract_files(std::string disc, std::string out_directory)
  {
    //  Get the raw FST data from the disc
    uint32_t fstsize = util::read_big<uint32_t>(disc, Header::Offset::FSTSize);
    uint32_t fstoffset = util::read_big<uint32_t>(disc, Header::Offset::FSTOffset);
    std::vector<uint8_t> fstbin = util::read_file(disc, fstsize, fstoffset);
    std::vector<std::thread> threads;

    //  Create FST object
    fst::FST fst(fstbin);

    for (auto& node : fst.entries())
    {
      std::string path = node.first.substr(2); // Skip the ./ part
      fst::Node entry = node.second;

      if (entry.is_dir())
      {
        std::cout << "Creating directory: " << out_directory << path << std::endl;
        boost::filesystem::create_directories(out_directory + path);
      }
      else
      {
        std::cout << "Writing file: " << out_directory << path << std::endl;

        //threads.push_back(std::thread(std::bind(util::write_file, out_directory + path, util::read_file(disc, entry.data_size(), entry.data_offset()))));
        util::write_file(out_directory + path, util::read_file(disc, entry.data_size(), entry.data_offset()));
      }
    }

    for (auto& thread : threads)
    {
      thread.join();
    }
  }

  /*
    Summary:
      Extracts all important binaries and files from a disc to a given directory.

    Parameters:
      disc: Path to the disc to read from
      outpath: Directory to extract files to 
  */
  void extract(std::string disc, std::string outpath)
  {
    //  Store the directories where files will be extracted
    std::string syspath = outpath + "/sys/";
    std::string filepath = outpath + "/files/";

    //  Create the directories required
    boost::filesystem::create_directories(outpath);
    boost::filesystem::create_directories(syspath);
    boost::filesystem::create_directories(filepath);

    //  Read the raw header and bi2 data
    std::vector<uint8_t> headerbin = util::read_file(disc, 0x440);
    std::vector<uint8_t> bi2bin = util::read_file(disc, 0x2000, 0x440);

    //  Write out the header and bi2 data
    util::write_file(syspath + "header.bin", headerbin);
    util::write_file(syspath + "bi2.bin", bi2bin);

    //  Extract each section of non-file data out
    extract_app(disc, syspath);
    extract_fst(disc, syspath);
    extract_dol(disc, syspath);

    //  Extract the files
    extract_files(disc, filepath);
  }

  /*
    Summary:
      Prints each file entry to the console. Does not print plain or empty directories.

    Parameters:
      disc: Path to the disc to read from
  */
  void files(std::string disc)
  {
    //  Get the raw FST data from the disc
    uint32_t fstsize = util::read_big<uint32_t>(disc, Header::Offset::FSTSize);
    uint32_t fstoffset = util::read_big<uint32_t>(disc, Header::Offset::FSTOffset);
    std::vector<uint8_t> fstbin = util::read_file(disc, fstsize, fstoffset);

    //  Create an FST object
    fst::FST table(fstbin);

    //  Print out each file listing
    for (auto& file : table.files())
    {
      std::cout << file.path() << std::endl;
    }
  }
}