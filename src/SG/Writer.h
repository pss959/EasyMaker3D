#pragma once

#include <ostream>

#include "SG/Typedefs.h"

namespace SG {

//! The Writer class writes an SG::Scene to a stream.
class Writer {
  public:
    Writer();
    ~Writer();

    //! Writes a Scene to a stream.
    void WriteScene(const Scene &scene, std::ostream &out);
};

}  // namespace SG
