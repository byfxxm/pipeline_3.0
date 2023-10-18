#include "pipeline.hpp"
#include "gworker.hpp"
#include "pipeline_imp.hpp"

namespace byfxxm {
std::unique_ptr<Pipeline> MakePipeline() {
  return std::make_unique<PipelineImp>();
}

std::unique_ptr<Worker> MakeGworker(gworker_t type, const char *content) {
  try {
    switch (type) {
    case gworker_t::FILE:
      return std::make_unique<Gworker>(std::fstream(content));

    case gworker_t::MEMORY:
      return std::make_unique<Gworker>(std::stringstream(content));

    default:
      assert(0);
    }
  } catch (const std::exception &exc) {
    puts(exc.what());
  }

  return {};
}
} // namespace byfxxm
