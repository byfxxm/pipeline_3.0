#include "pch.h"
#include "pipeline_adapter.h"
#include "../pipeline/pipeline.h"
#include "../pipeline/code.h"

OutputFunc g_func = [](const char* s) {
	std::printf(s);
	};

struct TestCode : byfxxm::Code {
	TestCode(size_t n) : _n(n) {}
	size_t _n;
};

class FirstWorker : public byfxxm::Worker {
public:
	~FirstWorker() override = default;

	bool Do(std::unique_ptr<byfxxm::Code> code, const byfxxm::WriteFunc& write) noexcept override {
		for (size_t i = 0; i < 10; ++i) {
			write(std::make_unique<TestCode>(i));
		}

		return true;
	}
};

class TestWorker : public byfxxm::Worker {
public:
	~TestWorker() override = default;

	bool Do(std::unique_ptr<byfxxm::Code> code, const byfxxm::WriteFunc& write) noexcept override {
		if (static_cast<TestCode*>(code.get())->_n == 1000)
			return false;

		write(std::move(code));
		return true;
	}
};

class LastWorker : public byfxxm::Worker {
public:
	~LastWorker() override = default;

	bool Do(std::unique_ptr<byfxxm::Code> code, const byfxxm::WriteFunc& write) noexcept override {
		if (g_func)
			g_func(std::format("{}\n", static_cast<TestCode*>(code.get())->_n).c_str());

		return true;
	}
};

void* CreateAuto() {
	auto pipeline = byfxxm::MakePipeline();
	pipeline->AddWorker(std::make_unique<FirstWorker>());
	for (int i = 0; i < 2; ++i) {
		pipeline->AddWorker(std::make_unique<TestWorker>());
	}

	pipeline->AddWorker(std::make_unique<LastWorker>());
	return static_cast<void*>(pipeline.release());
}

void DeleteAuto(void* pipeline) {
	delete static_cast<byfxxm::Pipeline*>(pipeline);
}

void Start(void* pipeline) {
	return static_cast<byfxxm::Pipeline*>(pipeline)->Start();
}

void Stop(void* pipeline) {
	return static_cast<byfxxm::Pipeline*>(pipeline)->Stop();
}

void Wait(void* pipeline) {
	return static_cast<byfxxm::Pipeline*>(pipeline)->Wait();
}

void SetGlobalOutput(OutputFunc func) {
	g_func = func;
}
