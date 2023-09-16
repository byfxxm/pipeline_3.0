#pragma once
#include <functional>

#ifdef PIPELINE_EXPORTS
#define PIPELINE_API __declspec(dllexport)
#else
#define PIPELINE_API __declspec(dllimport)
#endif

namespace byfxxm {
	struct Code;
	using WriteFunc = std::function<void(std::unique_ptr<Code>)>;

	class Worker {
	public:
		virtual ~Worker() = default;
		virtual bool Do(std::unique_ptr<Code>, const WriteFunc&) noexcept = 0;
	};

	enum class gworker_t {
		FILE = 0,
		MEMORY,
	};

	class Pipeline {
	public:
		virtual ~Pipeline() = default;
		virtual void AddWorker(Worker*) = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void Wait() = 0;
	};

	PIPELINE_API std::unique_ptr<Pipeline> MakePipeline();
	PIPELINE_API std::unique_ptr<Worker> MakeGworker(gworker_t type, const char* content);
	PIPELINE_API std::unique_ptr<Worker> MakeIssuer();
}
