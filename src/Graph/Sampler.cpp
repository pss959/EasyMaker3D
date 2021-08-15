#include "Graph/Sampler.h"

namespace Graph {

Sampler::Sampler() : i_sampler_(new ion::gfx::Sampler) {
    std::cerr << "XXXX Creating sampler = " << this << "\n";
}

}  // namespace Graph
