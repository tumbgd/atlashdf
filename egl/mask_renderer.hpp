#ifndef MASK_RENDERER_H
#define MASK_RENDERER_H

#include <string>
#include <vector>

std::vector<uint8_t> get_mask(int width, int height, double bbox[4],
                              std::string bbox_crs,
                              std::vector<std::string> collections,
                              std::string crs);
#endif
