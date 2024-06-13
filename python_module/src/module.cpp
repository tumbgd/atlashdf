#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
// The AtlasHDF module

#include <mask_renderer.hpp>
#include <osm_immediate.h>
#include <osm_resolvegeom.h>

class AtlasHDF {
protected:
  std::string hdf_container;
  int chunk_size = 1024;
  // std::string nodes_filter;
  std::map<std::string, std::string> filters{
      {"nodes", "."}, {"ways", "."}, {"relations", "."}};

public:
  AtlasHDF() : hdf_container("") {}
  AtlasHDF &set_container(std::string filename) {
    hdf_container = filename;
    if (false) {
      FILE *f = fopen(filename.c_str(), "w");
      if (f == NULL)
        throw(std::runtime_error("Cannot truncate " + filename));
      fclose(f);
    }
    return *this; // method chaining by returning a self-reference
  }

  AtlasHDF &clear_filters() {
    filters["nodes"] = filters["ways"] = filters["relations"] = ".";
    return *this;
  }
  AtlasHDF &set_filter(std::string scope, std::string filter) {
    filters[scope] = filter;
    return *this;
  }

  AtlasHDF &import(std::string filename) {
    if (hdf_container == "")
      throw(std::runtime_error("Run set_container before import"));
    import_osm_immediate(filename, hdf_container, chunk_size, filters["nodes"],
                         filters["ways"], filters["relations"]);
    return *this;
  }

  AtlasHDF &resolve(std::string triangulation) {
    if (hdf_container == "")
      throw(std::runtime_error("Run set_container before import"));
    resolve_osm_geometry(hdf_container, "notimplemented", triangulation,
                         filters["nodes"], filters["ways"],
                         filters["relations"]);
    return *this;
  }

  std::vector<uint8_t> mask(int width, int height, double bbox[4],
                            std::string bbox_crs,
                            std::vector<std::string> collections,
                            std::string crs) {
    return get_mask(width, height, bbox, bbox_crs, collections, crs);
  }
};

// ----------------
// Python interface
// ----------------

namespace py = pybind11;

PYBIND11_MODULE(atlashdf, m) {
  m.doc() = "atlashdf module";
  py::class_<AtlasHDF>(m, "AtlasHDF")
      .def(py::init<>())
      .def("set_container", &AtlasHDF::set_container)
      .def("set_filter", &AtlasHDF::set_filter)
      .def("clear_filters", &AtlasHDF::clear_filters)
      .def("import_immediate", &AtlasHDF::import)
      .def("resolve", &AtlasHDF::resolve)
      .def("get_mask", &AtlasHDF::mask);
}
