#include <filesystem>
#include <iostream>
#include <random>
#include <vector>

#include "mask_renderer.hpp"
#include "svpng.inc"
#include "tictoc.hpp"

void random_bbox(double (&bbox)[4]) {
  double size = 10000;
  static std::uniform_real_distribution<double> xd(4373288.71429446,
                                                   4555048.71429446 - size);
  static std::uniform_real_distribution<double> yd(2698008.535541312,
                                                   2890828.535541312 - size);
  static std::default_random_engine re;

  double x = xd(re);
  double y = yd(re);

  bbox[0] = x;
  bbox[1] = y;
  bbox[2] = x + size;
  bbox[3] = y + size;
}

int main(int argc, char *argv[]) {
  int width = 256;
  int height = 256;
  double bbox[4];
  std::vector<std::string> collections = {argv[1]};
  std::string crs = "EPSG:3035";

  std::filesystem::create_directory("out");

  std::cout << "Running 1000 frames" << std::endl;
  {
    tictoc t("1000 frames");
    for (size_t i = 0; i < 1000; i++) {
      random_bbox(bbox);

      std::vector<uint8_t> image =
          get_mask(width, height, bbox, crs, collections, crs);

      char fname[1024];
      snprintf(fname, 1024, "out/%04ld.png", i);
      FILE *f = fopen(fname, "wb");
      svpng(f, width, height, image.data(), 1);
      fclose(f);
    }
  } // tictoc

  return 0;
}
