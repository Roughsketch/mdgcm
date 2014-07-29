#ifndef _GAMECUBE_HEADER_H
#define _GAMECUBE_HEADER_H

#include <cstdint>
#include <fstream>
#include <vector>
#include <memory>
#include <thread>

#include <boost/filesystem.hpp>

#include "util.h"

#include "gcm_header.h"
#include "gcm_fst.h"

namespace gcm
{
  bool valid_directory(std::string root);

  void extract(std::string disc, std::string outfile);
  void extract_app(std::string disc, std::string out_directory);
  void extract_fst(std::string disc, std::string out_directory);
  void extract_dol(std::string disc, std::string out_directory);
  void extract_files(std::string disc, std::string out_directory);

  void build(std::string root, std::string outfile);

  void files(std::string disc);
}

#endif
