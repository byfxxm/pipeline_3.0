#include "pch.h"
#include "pipeline.h"
#include "pipeline_imp.h"
#include "gworker.h"
#include "issuer.h"

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

void pipeline_wait(void* pipeline) {
	static_cast<PipelineImp*>(pipeline)->Wait();
}

void* gworker_new(gworker_t type, const char* content) {
	try {
		switch (type) {
		case gworker_t::FILE:
			return new Gworker(std::fstream(content));

		case gworker_t::MEMORY:
			return new Gworker(std::stringstream(content));

		default:
			assert(0);
		}
	}
	catch (const std::exception& exc) {
		puts(exc.what());
	}

	return nullptr;
}

void gworker_delete(void* gworker) {
	delete static_cast<Gworker*>(gworker);
}

void* issuer_new() {
	return new Issuer();
}

void issuer_delete(void* issuer) {
	delete static_cast<Issuer*>(issuer);
}