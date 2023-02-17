#pragma once

#ifdef PIPELINE_EXPORTS
#define PIPELINE_API __declspec(dllexport)
#else
#define PIPELINE_API __declspec(dllimport)
#endif

extern "C" {
	PIPELINE_API void* pipeline_new();
	PIPELINE_API void pipeline_delete(void* pipeline);
	PIPELINE_API void pipeline_add_worker(void* pipeline, void* worker);
	PIPELINE_API void pipeline_start(void* pipeline);
	PIPELINE_API void pipeline_stop(void* pipeline);
}