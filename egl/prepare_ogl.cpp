#include <highfive/H5Easy.hpp>
#include <iostream>
#include <proj.h>
#include <vector>

typedef std::vector<std::vector<double>> coord_type;

coord_type coords;

PJ_CONTEXT *C;
PJ *P;
PJ *norm;
PJ_COORD a, b;

#define EPS 0.000001

bool near(double x, double y, double u, double v) {
  return fabs((x - u) * (x - u) + (y - v) * (y - v)) < EPS;
}

size_t find_or_add(double x, double y, std::vector<double> &vertex_array) {
  for (size_t i = 0; i < vertex_array.size(); i++) {
    if (near(x, y, vertex_array[i], vertex_array[i + 1])) {
      return i / 2;
    }
    i++; // skip over y
  }
  // not found, add
  vertex_array.push_back(x);
  vertex_array.push_back(y);
  return (vertex_array.size() / 2) - 1;
}

void transform(std::vector<std::vector<double>> &coords,
               std::string from_crs = "EPSG:4326",
               std::string to_crs = "EPSG:3857") {
  C = proj_context_create();
  P = proj_create_crs_to_crs(C, from_crs.c_str(), to_crs.c_str(), NULL);

  if (0 == P) {
    std::cout << "Failed to create transformation object." << std::endl;
    return;
  }

  /* This will ensure that the order of coordinates for the input CRS */
  /* will be longitude, latitude, whereas EPSG:4326 mandates latitude, */
  /* longitude */
  norm = proj_normalize_for_visualization(C, P);
  if (0 == norm) {
    std::cout << "Failed to normalize transformation object." << std::endl;
    return;
  }
  proj_destroy(P);
  P = norm;

  for (auto &coord : coords) {
    a = proj_coord(coord[0], coord[1], 0, 0);
    b = proj_trans(P, PJ_FWD, a);
    coord[0] = b.enu.e;
    coord[1] = b.enu.n;
  }

  proj_destroy(P);
  proj_context_destroy(C); /* may be omitted in the single threaded case */
}

// This function prepares vertex array and index array for the tile renderer
void load_opengl(std::vector<std::string> &collections,
                 std::vector<double> &vertex_array,
                 std::vector<uint32_t> &index_array, std::string &to_crs) {
  for (auto collection : collections) {
    auto position = collection.find(".h5/") + 3;

    std::string filename = collection.substr(0, position);
    std::string coords_ds = collection.substr(position);

    H5Easy::File file(filename, H5Easy::File::ReadOnly);
    try {
      auto new_coords = H5Easy::load<coord_type>(file, coords_ds);
      coords.insert(coords.end(), new_coords.begin(), new_coords.end());
    } catch (...) {
      std::cout << "Loading failed. Expected " << coords_ds << " in "
                << filename << std::endl;
      return;
    }
  }

  std::cout << "Found " << coords.size() / 3 << " triangles" << std::endl;

  // Transformation
  std::cout << "Reprojecting ..." << std::endl;

  transform(coords, "EPSG:4326", to_crs);

  // Optimizing (slow)
  std::cout << "Optimizing ..." << std::endl;

  vertex_array.clear();
  index_array.clear();

  for (auto coord : coords) {
    index_array.push_back(
        (uint32_t)find_or_add(coord[0], coord[1], vertex_array));
  }
  std::cout << "Compressed Vertices " << vertex_array.size() << std::endl;
  std::cout << "Compressed Indices " << index_array.size() << std::endl;
}
