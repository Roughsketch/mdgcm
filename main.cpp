/*
  Originally created by Maiddog <maiddogsrl@gmail.com> 7/19/2014

  GCM v1.0 - 7/20/2014
      Support for extraction, re-building, and file listing
*/

#include <iostream>
#include <boost/filesystem.hpp>

#include "gcm.h"

void usage()
{
  std::cout << "Usage: gcm.exe <Command> <Root> <Output>";
  std::cout << R"DOC(
    <Command>: "build"|"b" or "extract"|"e" or "files"|"f"
    <Root>   : Build: Directory where a disc was previously extracted
               Extract: Path to the disc to extract from
               Files: Path to the disc
    <Output> : Build: Output file path and name
               Extract: Output directory where files will be extracted
    Examples:
      gcm.exe extract Example.gcm output_dir
      gcm.exe build output_dir RebuiltExample.gcm
  )DOC" << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    usage();
    exit(EXIT_FAILURE);
  }

  std::string cmd(argv[1]);   //  Command comes first

  if (argc == 4 && (cmd == "build" || cmd == "b"))
  {
    std::string root(argv[2]);  //  Root directory or file path
    std::string out(argv[3]);   //  Output directory or file path
    if (gcm::valid_directory(root))
    {
      gcm::build(root, out);
    }
    else
    {
      std::cout << "Invalid directory.";
      exit(EXIT_FAILURE);
    }
  }
  else if (argc == 4 && (cmd == "extract" || cmd == "e"))
  {
    std::string root(argv[2]);  //  Root directory or file path
    std::string out(argv[3]);   //  Output directory or file path
    gcm::extract(root, out);
  }
  else if (argc == 3 && (cmd == "files" || cmd == "f"))
  {
    std::string root(argv[2]);  //  Root directory or file path
    gcm::files(root);
  }
  else
  {
    std::cout << "Invalid command: " << cmd << std::endl;
    usage();
    exit(EXIT_FAILURE);
  }


  return EXIT_SUCCESS;
}
