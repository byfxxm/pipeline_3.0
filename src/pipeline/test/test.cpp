// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "../pipeline/pipeline.h"
#include "../pipeline/worker.h"
#include "../pipeline/code.h"
#include "../pipeline/lexer.h"
#include "../pipeline/syntax.h"
#include "../pipeline/gparser.h"

#pragma comment(lib, "../x64/Debug/pipeline.lib")

struct TestCode : byfxxm::Code {
	TestCode(size_t n) : _n(n) {}
	size_t _n;
};

class FirstWorker : public byfxxm::Worker {
public:
	~FirstWorker() override = default;

	bool Do(byfxxm::Code* code, const byfxxm::WriteFunc& write) noexcept override {
		for (size_t i = 0; i < 10000; ++i) {
			write(new TestCode(i));
		}

		return true;
	}
};

class TestWorker : public byfxxm::Worker {
public:
	~TestWorker() override = default;

	bool Do(byfxxm::Code* code, const byfxxm::WriteFunc& write) noexcept override {
		if (static_cast<TestCode*>(code)->_n == 1000)
			return false;

		write(code);
		return true;
	}
};

class LastWorker : public byfxxm::Worker {
public:
	~LastWorker() override = default;

	bool Do(byfxxm::Code* code, const byfxxm::WriteFunc& write) noexcept override {
		std::cout << static_cast<TestCode*>(code)->_n << std::endl;
		delete code;
		return true;
	}
};

void TestPipeline() {
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
}

void TestLexer() {
	std::string s =
		R"(
		#1 = 10
		#2 = 20
		#3=[#1+ #2]*30
)";

	std::vector<byfxxm::Token> res;
	byfxxm::Token tok;
	byfxxm::Lexer lex(s);
	while ((tok = lex.Next()).kind != byfxxm::Kind::KEOF) {
		res.emplace_back(std::move(tok));
	}

	std::for_each(res.begin(), res.end(), [](const byfxxm::Token& tok) {
		std::cout << static_cast<int>(tok.kind) << std::endl;
		});
}

void TestSyntax() {
	std::string s =
		R"(
		#1 = -10
		#2 = 20
		#30 = 5
		#10 =#1+#30*#2
		#180 = 2
		#3=#[2*#[#1+ #2]]/5
)";

	class GImpl : public byfxxm::Ginterface {
	public:
		virtual bool G0(const byfxxm::Gfunc&) override {
			return true;
		}
	};

	auto syntax = byfxxm::Syntax(byfxxm::Lexer(s));
	while (1) {
		auto tree = syntax.Next();
		if (!tree.has_value())
			break;
		tree.value().Execute(GImpl());
	}

	return;
}

void TestParser() {
	std::string s =
		R"(
		#1 = -10
		#2 = 20
		#30 = 5
		#10 =#1+#30*#2
		#180 = 2
		#3=#[2*#[#1+ #2]]/5
)";

	class GImpl : public byfxxm::Ginterface {
	public:
		virtual bool G0(const byfxxm::Gfunc&) override {
			return true;
		}
	};

	auto parser = byfxxm::Gparser(byfxxm::Syntax(byfxxm::Lexer(s)));
	parser.Run(GImpl());
}

int main()
{
	//TestPipeline();
	TestLexer();
	TestSyntax();
	TestParser();
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
