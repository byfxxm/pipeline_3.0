// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "../pipeline/pipeline.h"
#include "../pipeline/worker.h"
#include "../pipeline/code.h"

#pragma comment(lib, "../x64/Debug/pipeline.lib")

struct TestCode : byfxxm::Code {
	TestCode(size_t n) : _n(n) {}
	size_t _n;
};

class FirstWorker : public byfxxm::Worker {
public:
	~FirstWorker() override = default;

	void Do(byfxxm::Code* code, byfxxm::WriteFunc write) override {
		for (size_t i = 0; i < 100; ++i) {
			write(new TestCode(i));
		}
	}
};

class TestWorker : public byfxxm::Worker {
public:
	~TestWorker() override = default;

	void Do(byfxxm::Code* code, byfxxm::WriteFunc write) override {
		write(code);
	}
};

class LastWorker : public byfxxm::Worker {
public:
	~LastWorker() override = default;
	void Do(byfxxm::Code* code, byfxxm::WriteFunc write) override {
		std::cout << static_cast<TestCode*>(code)->_n << std::endl;
	}
};

int main()
{
	auto pipeline = pipeline_new();

	FirstWorker first;
	TestWorker workers[2];
	pipeline_add_worker(pipeline, &first);
	for (auto& w : workers) {
		pipeline_add_worker(pipeline, &w);
	}
	LastWorker last;
	pipeline_add_worker(pipeline, &last);
	pipeline_start(pipeline);
	pipeline_delete(pipeline);
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
