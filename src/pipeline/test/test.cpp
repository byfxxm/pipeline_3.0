// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "../pipeline/code.hpp"
#include "../pipeline/gparser/gparser.hpp"
#include "../pipeline/gworker.hpp"
#include "../pipeline/pipeline.hpp"
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

using OutputFunc = void (*)(const char *);
OutputFunc g_outputfunc = [](const char *str) { printf(str); };

std::mutex g_mutex;

template <class T> void PrintLine(T &&t) {
  std::unique_lock lock(g_mutex);
  if (g_outputfunc)
    g_outputfunc(std::format("{}\n", t).c_str());
}

template <class T> void Print(T &&t) {
  std::unique_lock lock(g_mutex);
  if (g_outputfunc)
    g_outputfunc(std::format("{}", t).c_str());
}

struct TestCode : byfxxm::Code {
  TestCode(size_t n) : _n(n) {}
  size_t _n;
};

class Gimpl : public byfxxm::Ginterface {
public:
  virtual bool None(const byfxxm::Gparams &params,
                    const byfxxm::Address *addr) override {
    return true;
  }
  virtual bool G0(const byfxxm::Gparams &params,
                  const byfxxm::Address *addr) override {
    return true;
  }
  virtual bool G1(const byfxxm::Gparams &params,
                  const byfxxm::Address *addr) override {
    return true;
  }
  virtual bool G2(const byfxxm::Gparams &params,
                  const byfxxm::Address *addr) override {
    return true;
  }
  virtual bool G3(const byfxxm::Gparams &params,
                  const byfxxm::Address *addr) override {
    return true;
  }
  virtual bool G4(const byfxxm::Gparams &params,
                  const byfxxm::Address *addr) override {
    return true;
  }
};

void TestParser() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  double x = 5;
  addr.Insert(10000, &x);
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[1]) == -10.);
  assert(byfxxm::Get(addr[2]) == 20);
  assert(byfxxm::Get(addr[30]) == 5);
  assert(byfxxm::Get(addr[10]) == 87);
  assert(byfxxm::Get(addr[-174]) == -2);
  assert(byfxxm::Get(addr[3]) == -0.4);
  assert(byfxxm::Get(addr[10000]) == 5);
}

void TestParser1() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test1.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::IsNaN(byfxxm::Get(addr[4])));
  assert(byfxxm::Get(addr[3]) == 2);
  assert(byfxxm::Get(addr[2]) == 1);
  assert(byfxxm::IsNaN(byfxxm::Get(addr[20])));
}

void TestParser2() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test2.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[1]) == 1);
  assert(byfxxm::Get(addr[6]) == 6);
  assert(byfxxm::Get(addr[3]) == 3);
  assert(byfxxm::Get(addr[2]) == 2);
  assert(byfxxm::Get(addr[5]) == 5);
}

void TestParser3() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test3.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[1]) == 20);
  assert(byfxxm::Get(addr[3]) == 22);
  assert(byfxxm::Get(addr[2]) == 20);
  assert(byfxxm::Get(addr[5]) == 5);
  assert(byfxxm::Get(addr[10]) == 234.5);
  assert(byfxxm::Get(addr[20]) == 25);
  assert(byfxxm::Get(addr[25]) == 100);
  assert(byfxxm::Get(addr[100]) == 100);
}

void TestParser4() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test4.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[1]) == 234.5);
  assert(byfxxm::Get(addr[3]) == 2);
  assert(byfxxm::Get(addr[2]) == 234.5);
}

void TestParser5() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test5.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[10]) == 21);
}

void TestParser6() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test6.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;
  if (auto res = parser.Run(&addr, &gimpl)) {
    PrintLine(res.value());
    return;
  }

  assert(byfxxm::Get(addr[1]) == 1);
  assert(byfxxm::Get(addr[2]) == 2);
}

void TestParser7() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test7.nc"));
  auto gimpl = Gimpl();
  byfxxm::Address addr;

  constexpr size_t lines[] = {2, 3, 4, 5, 7, 8, 12, 14};
  auto iter = std::begin(lines);
  if (auto res = parser.Run(&addr, &gimpl, [&iter](size_t line) {
        PrintLine(line);
        assert(line == (*iter++));
      })) {
    PrintLine(res.value());
    return;
  }
}

void TestParser8() {
  auto parser = byfxxm::Gparser(std::ifstream(
      std::filesystem::current_path().string() + "/ncfiles/test8.nc"));
  auto gimpl = Gimpl();
  double x = 5;
  double y = 6;
  byfxxm::Address addr = {
      {1, &y},
      {2, byfxxm::GetSetSharp{[&]() { return x; }, [&](double v) { x = v; }}},
  };
  auto res = parser.Run(&addr, &gimpl);
  assert(res);
  puts(res.value().c_str());
  assert(x == 2);
  assert(y == 1);
}

class MyFileStream {
public:
  explicit MyFileStream(const std::filesystem::path &pa) {
    fopen_s(&_file, pa.string().c_str(), "r");
    assert(_file);
  }

  ~MyFileStream() {
    if (_file)
      fclose(_file);
  }

  MyFileStream(MyFileStream &&rhs) noexcept
      : _file(std::exchange(rhs._file, nullptr)),
        _cache(std::move(rhs._cache)) {}

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

  bool eof() { return feof(_file) != 0; }

private:
  FILE *_file{nullptr};
  std::optional<int> _cache;
};

inline auto perform = [](const std::filesystem::path &pa, int times) {
  auto t0 = std::chrono::high_resolution_clock::now();
  for (auto i = 0; i < times; ++i) {
    auto parser = byfxxm::Gparser(MyFileStream(pa));
    byfxxm::Address addr;
    parser.Run(&addr, nullptr);
  }
  auto t1 = std::chrono::high_resolution_clock::now();

  double cost =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1e6;
  double length =
      static_cast<double>(std::filesystem::file_size(pa)) / 1e6 * times;

  Print(length);
  PrintLine(" MB");
  Print(cost);
  PrintLine(" s");
  Print(length / cost);
  PrintLine(" MB/s");
};

void TestPerformance() {
  perform(std::filesystem::path(std::filesystem::current_path().string() +
                                R"(\ncfiles\LTJX.nc)"),
          1);
}

void TestPerformance1() {
  perform(std::filesystem::path(std::filesystem::current_path().string() +
                                R"(\ncfiles\macro1.nc)"),
          100);
}

std::string _Format(const byfxxm::AxesArray &axes) {
  std::string ret;
  std::ranges::for_each(axes, [&](auto &&item) {
    ret += " ";
    ret += std::to_string(item);
  });

  return ret;
}

class Issuer : public byfxxm::Worker {
private:
  virtual bool Do(std::unique_ptr<byfxxm::Code> code,
                  const byfxxm::WriteFunc &writefn) noexcept override {
    switch (code->tag) {
    case byfxxm::codetag::MOVE:
      Print("G0");
      PrintLine(_Format(static_cast<byfxxm::Move *>(code.get())->end));
      break;
    case byfxxm::codetag::LINE:
      Print("G1");
      PrintLine(_Format(static_cast<byfxxm::Line *>(code.get())->end));
      break;
    case byfxxm::codetag::ARC:
      break;
    default:
      break;
    }

    return true;
  }
};

class FirstWorker : public byfxxm::Worker {
public:
  ~FirstWorker() override = default;

  bool Do(std::unique_ptr<byfxxm::Code> code,
          const byfxxm::WriteFunc &write) noexcept override {
    for (size_t i = 0; i < 10000; ++i) {
      write(std::make_unique<TestCode>(i));
    }

    return true;
  }
};

class TestWorker : public byfxxm::Worker {
public:
  ~TestWorker() override = default;

  bool Do(std::unique_ptr<byfxxm::Code> code,
          const byfxxm::WriteFunc &write) noexcept override {
    if (static_cast<TestCode *>(code.get())->_n == 1000)
      return false;

    write(std::move(code));
    return true;
  }
};

class LastWorker : public byfxxm::Worker {
public:
  ~LastWorker() override = default;

  bool Do(std::unique_ptr<byfxxm::Code> code,
          const byfxxm::WriteFunc &write) noexcept override {
    PrintLine(static_cast<TestCode *>(code.get())->_n);
    return true;
  }
};

void TestPipeline() {
  auto pipeline = byfxxm::Pipeline();
  pipeline.AddWorker(std::make_unique<FirstWorker>());
  for (int i = 0; i < 20; ++i) {
    pipeline.AddWorker(std::make_unique<TestWorker>());
  }

  pipeline.AddWorker(std::make_unique<LastWorker>());
  pipeline.Start();
  auto t = std::thread([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pipeline.Stop();
  });
  pipeline.Wait();
  t.join();
}

void TestPipeline1() {
  auto pipeline = byfxxm::Pipeline();
  pipeline.AddWorker(
      std::make_unique<byfxxm::Gworker>(std::stringstream(R"(G0 X0Y0Z0
G1X100
Y100
)")));
  pipeline.AddWorker(std::make_unique<Issuer>());
  pipeline.Start();
  pipeline.Wait();
}

int main() {
  std::thread thr[1];
  for (auto &t : thr) {
    t = std::thread([]() {
      TestParser();
      TestParser1();
      TestParser2();
      TestParser3();
      TestParser4();
      TestParser5();
      TestParser6();
      TestParser7();
      TestParser8();
      // TestPipeline();
      // TestPipeline1();
      TestPerformance();
      TestPerformance1();
    });
  }

  for (auto &t : thr) {
    t.join();
  }

  return 0;
}

void SetOutput(OutputFunc func) { g_outputfunc = func; }

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5.
//   转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
