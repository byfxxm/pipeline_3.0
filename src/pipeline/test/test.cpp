// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <utility>
#include <format>
#include "../pipeline/pipeline.h"
#include "../pipeline/code.h"
#include "../pipeline/gparser/gparser.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#ifdef _WIN64
#pragma comment(lib, "../x64/Debug/pipeline.lib")
#else
#pragma comment(lib, "../Debug/pipeline.lib")
#endif
#else
#ifdef _WIN64
#pragma comment(lib, "../x64/Release/pipeline.lib")
#else
#pragma comment(lib, "../Release/pipeline.lib")
#endif
#endif
#endif

using OutputFunc = void(*)(const char*);
OutputFunc g_outputfunc = [](const char* str) {
	printf(str);
	};

std::mutex g_mutex;

template <class T>
void PrintLine(T&& t) {
	std::unique_lock lock(g_mutex);
	if (g_outputfunc)
		g_outputfunc(std::format("{}\n", t).c_str());
}

template <class T>
void Print(T&& t) {
	std::unique_lock lock(g_mutex);
	if (g_outputfunc)
		g_outputfunc(std::format("{}", t).c_str());
}

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
		PrintLine(static_cast<TestCode*>(code.get())->_n);
		return true;
	}
};

void TestPipeline() {
	auto pipeline = byfxxm::MakePipeline();
	FirstWorker first;
	pipeline->AddWorker(std::make_unique<FirstWorker>());
	for (int i = 0; i < 20; ++i) {
		pipeline->AddWorker(std::make_unique<TestWorker>());
	}

	pipeline->AddWorker(std::make_unique<LastWorker>());
	pipeline->Start();
	pipeline->Wait();
}

void TestLexer() {
	std::string s =
		R"(
		#1 = 10
		#2 = 20
		#3=[#1+ #2]*30
)";

	std::vector<byfxxm::token::Token> res;
	byfxxm::token::Token tok;
	auto lex = byfxxm::Lexer(std::stringstream(s));
	while ((tok = lex.Get()).kind != byfxxm::token::Kind::KEOF) {
		res.emplace_back(std::move(tok));
	}

	std::for_each(res.begin(), res.end(), [](const byfxxm::token::Token& tok) {
		PrintLine(static_cast<int>(tok.kind));
		});
}

class Gpimpl : public byfxxm::Ginterface {
public:
	virtual bool None(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
	virtual bool G0(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
	virtual bool G1(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
	virtual bool G2(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
	virtual bool G3(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
	virtual bool G4(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
		return true;
	}
};

void TestParser() {
	std::string s =
		R"(


		#1 = -10
		#2 = 20
		#30 = 5
		#10 =#1+#30*#2-3
		#-174 = -2 ; #3=+#-[2*#[#1+ #2]]/5
		#3 LT #2


)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[1] == -10);
	assert(*addr[2] == 20);
	assert(*addr[30] == 5);
	assert(*addr[10] == 87);
	assert(*addr[-174] == -2);
	assert(*addr[3] == -0.4);
}

void TestParser1() {
	std::string s =
		R"(
		#2 = 1
		#3=2
		#20=#4
		G#2X-#3Y#20Z10
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(byfxxm::IsNaN(*addr[4]));
	assert(*addr[3] == 2);
	assert(*addr[2] == 1);
	assert(byfxxm::IsNaN(*addr[20]));
}

void TestParser2() {
	std::string s =
		R"(
		#1 = 1
		#2 = 2
		IF #1 LT #2 THEN
			#3 = 3
			
			IF #2 LT #3 THEN
				#6 = 6
			ELSE
				#7=7
			ENDIF
			#5=5
		ELSE
			#4 = 4
		ENDIF
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[1] == 1);
	assert(*addr[6] == 6);
	assert(*addr[3] == 3);
	assert(*addr[2] == 2);
	assert(*addr[5] == 5);
}

void TestParser3() {
	std::string s =
		R"(
		#1 = 1
		#2 = 20
		IF NOT [#1 GE #2] THEN
			#3 = 3
			
			WHILE [#1 LT #2] DO
				#1 = #1 +1
				#3 = #3 + 1
				IF #1 LT #3 THEN
					#10 = 234.5
					#20 = MAX[#MIN[1,2,3], 3.5, 10]
					WHILE [#20 LT 25] DO
						#20 = 1 + #20
					END
					#100 = 100
					IF #20 EQ 25 THEN
						#[#20 ]= #100
					ENDIF
				ENDIF

				G1 X#3
			END
			#5=5
		ELSE
			#4 = 4
		ENDIF
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[1] == 20);
	assert(*addr[3] == 22);
	assert(*addr[2] == 20);
	assert(*addr[5] == 5);
	assert(*addr[10] == 234.5);
	assert(*addr[20] == 25);
	assert(*addr[25] == 100);
	assert(*addr[100] == 100);
}

void TestParser4() {
	std::string s =
		R"(
		#1 = 234.5
		#2 = MAX[#MIN[1,2,3], 3.5, 2]
		#3 = MIN[2]
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[1] == 234.5);
	assert(*addr[3] == 2);
	assert(*addr[2] == 234.5);
}

void TestParser5() {
	std::string s =
		R"(
		#10 =[ [1 + 2]] * [3 + 4]
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[10] == 21);
}

void TestParser6() {
	std::string s =
		R"(
		IF 1 LT 2 THEN
		IF 1 LT 2 THEN
		IF 1 LT 2 THEN
		IF 1 LT 2 THEN
		IF 1 LT 2 THEN
		#1=1
		#2=2
		ENDIF
		ENDIF
		ENDIF
		ENDIF
		ENDIF
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl);

	assert(*addr[1] == 1);
	assert(*addr[2] == 2);
}

void TestParser7() {
	std::string s =
		R"(
		#1 = 1
		#2 = 2
		IF #1 LT #2 THEN
			#3 = 3
			
			IF #2 LT #3 THEN
				#6 = 6
			ELSE
				#7=7
			ENDIF
			#5=5
		ELSE
			#4 = 4
		ENDIF
)";

	auto parser = byfxxm::Gparser(s);
	auto pimpl = Gpimpl();
	byfxxm::Address addr;
	parser.Run(&addr, &pimpl, [](size_t line) {
		PrintLine(line);
		});
}

class MyFileStream {
public:
	MyFileStream(const std::filesystem::path& pa) {
		fopen_s(&_file, pa.string().c_str(), "r");
		assert(_file);
	}

	~MyFileStream() {
		if (_file)
			fclose(_file);
	}

	MyFileStream(MyFileStream&& rhs) noexcept
		: _file(std::exchange(rhs._file, nullptr))
		, _cache(std::move(rhs._cache)) {}

	int get() {
		auto ret = peek();
		_cache.reset();
		return ret;
	}

	int peek() {
		if (!_cache)
			_cache = fgetc(_file);

		return _cache.value();
	}

	bool eof() {
		return feof(_file) != 0;
	}

private:
	FILE* _file{ nullptr };
	std::optional<int> _cache;
};

inline auto perform = [](const std::filesystem::path& pa, int times) {
	auto t0 = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < times; ++i) {
		auto parser = byfxxm::Gparser(MyFileStream(pa));
		byfxxm::Address addr;
		parser.Run(&addr, nullptr);
	}
	auto t1 = std::chrono::high_resolution_clock::now();

	double cost = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;
	double length = static_cast<double>(std::filesystem::file_size(pa)) / 1e6 * times;

	Print(length);
	PrintLine(" MB");
	Print(cost);
	PrintLine(" s");
	Print(length / cost);
	PrintLine(" MB/s");
};

void TestPerformance() {
	perform(std::filesystem::path(std::filesystem::current_path().string() + R"(\LTJX.nc)"), 1);
}

void TestPerformance1() {
	perform(std::filesystem::path(std::filesystem::current_path().string() + R"(\macro1.nc)"), 100);
}

std::string _Format(const byfxxm::AxesArray& axes) {
	std::string ret;
	std::ranges::for_each(axes, [&](auto&& item) {
		ret += " ";
		ret += std::to_string(item);
		});

	return ret;
}

class Issuer : public byfxxm::Worker {
private:
	virtual bool Do(std::unique_ptr<byfxxm::Code> code, const byfxxm::WriteFunc& writefn) noexcept override {
		switch (code->tag) {
		case byfxxm::codetag::MOVE:
			Print("G0");
			PrintLine(_Format(static_cast<byfxxm::Move*>(code.get())->end));
			break;
		case byfxxm::codetag::LINE:
			Print("G1");
			PrintLine(_Format(static_cast<byfxxm::Line*>(code.get())->end));
			break;
		case byfxxm::codetag::ARC:
			break;
		default:
			break;
		}

		return true;
	}
};

void TestPipeline1() {
	auto pipeline = byfxxm::MakePipeline();
	pipeline->AddWorker(MakeGworker(byfxxm::gworker_t::MEMORY, R"(G0 X0Y0Z0
G1X100
Y100
)"));
	pipeline->AddWorker(std::make_unique<Issuer>());
	pipeline->Start();
	pipeline->Wait();
}

int main()
{
	TestParser();
	TestParser1();
	TestParser2();
	TestParser3();
	TestParser4();
	TestParser5();
	TestParser6();
	TestParser7();
	TestPipeline();
	TestPipeline1();
	TestPerformance();
	TestPerformance1();
	return 0;
}

void SetOutput(OutputFunc func) {
	g_outputfunc = func;
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
