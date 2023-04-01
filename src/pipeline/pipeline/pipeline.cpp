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

void* garser_new() {
	return new Gworker();
}

void garser_delete(void* gparser) {
	delete static_cast<Gworker*>(gparser);
}