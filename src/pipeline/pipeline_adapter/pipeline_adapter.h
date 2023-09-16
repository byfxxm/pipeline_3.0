#pragma once

#ifdef PIPELINEADAPTER_EXPORTS
#define PIPELINEADAPTER_API __declspec(dllexport)
#else
#define PIPELINEADAPTER_API __declspec(dllimport)
#endif

typedef void(*OutputFunc)(const char*);

extern "C" {
	PIPELINEADAPTER_API void* CreateAuto(OutputFunc);
	PIPELINEADAPTER_API void DeleteAuto(void*);
	PIPELINEADAPTER_API void Start(void*);
	PIPELINEADAPTER_API void Stop(void*);
	PIPELINEADAPTER_API void Wait(void*);
}