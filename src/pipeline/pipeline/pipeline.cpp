#include "pch.h"
#include "pipeline.h"
#include "pipeline_imp.h"
#include "gworker.h"
using namespace byfxxm;

void* pipeline_new() {
	return new PipelineImp();
}

void pipeline_delete(void* pipeline) {
	delete static_cast<PipelineImp*>(pipeline);
}

void pipeline_add_worker(void* pipeline, void* worker) {
	static_cast<PipelineImp*>(pipeline)->AddWorker(worker);
}

void pipeline_start(void* pipeline) {
	static_cast<PipelineImp*>(pipeline)->Start();
}

void pipeline_stop(void* pipeline) {
	static_cast<PipelineImp*>(pipeline)->Stop();
}

void* gworker_new(gworker_t type, const char* name) {
	try {
		switch (type) {
		case gworker_t::FILE:
			return new Gworker(std::fstream(name));

		case gworker_t::MEMORY:
			return new Gworker(std::stringstream(name));

		default:
			assert(0);
		}
	}
	catch (const std::exception& exc) {
		puts(exc.what());
	}

	return nullptr;
}

void gworker_delete(void* gparser) {
	delete static_cast<Gworker*>(gparser);
}