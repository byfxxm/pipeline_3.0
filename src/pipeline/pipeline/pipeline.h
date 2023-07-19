#pragma once

#ifdef PIPELINE_EXPORTS
#define PIPELINE_API __declspec(dllexport)
#else
#define PIPELINE_API __declspec(dllimport)
#endif

enum class gworker_t{
	FILE = 0,
	MEMORY,
};

extern "C" {
	PIPELINE_API void* pipeline_new();
	PIPELINE_API void pipeline_delete(void* pipeline);
	PIPELINE_API void pipeline_add_worker(void* pipeline, void* worker);
	PIPELINE_API void pipeline_start(void* pipeline);
	PIPELINE_API void pipeline_stop(void* pipeline);
	PIPELINE_API void pipeline_wait(void* pipeline);

	PIPELINE_API void* gworker_new(gworker_t, const char*);
	PIPELINE_API void gworker_delete(void* gworker);
	PIPELINE_API void* issuer_new();
	PIPELINE_API void issuer_delete(void* issuer);
}
