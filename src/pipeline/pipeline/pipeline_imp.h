#pragma once
#include <vector>
#include <memory>
#include "common.h"
#include "coroutine.h"
#include "worker.h"
#include "ring_buffer.h"

NAMESPACE_BEGIN
class PipelineImp {
public:
	using Fifo = RingBuffer<Code*, 4>;

	class Station {
	public:
		Station(PipelineImp& p, Worker& w) : _pipeline(p), _worker(w) {}

		void Write(Code* code) {
			while (!_next->Write(code)) {
				if (_pipeline._cur_station == _pipeline._station_list.size() - 1)
					break;

				++_pipeline._cur_station;
				_handle();
			}
		}

		Code* Read() {
			Code* ret = nullptr;
			while (!_prev->Read(ret)) {
				if (_pipeline._cur_station == 0)
					break;

				--_pipeline._cur_station;
				_handle();
			}

			return ret;
		}

		void Do() {
			auto write = [this](Code* p) {
				Write(p);
			};

			if (IsFirst()) {
				_worker.Process(nullptr, write);
				return;
			}

			while (1) {
				Code* code = Read();
				if (code == nullptr)
					break;

				_worker.Process(nullptr, write);
			}
		}

		void Link(Station& station) {
			station._prev = _next.get();
		}

		bool IsFirst() const {
			return _prev == nullptr;
		}

		void SetCoHandle(std::coroutine_handle<CoTask::promise_type> h) {
			_handle = h;
		}

		auto GetCoHandle() const {
			return _handle;
		}

		CoTask Coroutine() {
			Do();
			co_return;
		}

	private:
		PipelineImp& _pipeline;
		Worker& _worker;
		std::unique_ptr<Fifo> _next = std::make_unique<Fifo>();
		Fifo* _prev = nullptr;
		std::coroutine_handle<CoTask::promise_type> _handle;
	};

	void Start() {
		if (_station_list.empty())
			return;

		_Schedule();
	}

	void Stop() {

	}

	void AddWorker(void* worker) {
		auto station = std::make_unique<Station>(*this, *static_cast<Worker*>(worker));
		if (!_station_list.empty())
			_station_list.back()->Link(*station);

		_station_list.emplace_back(std::move(station));
	}

private:
	void _Schedule() {
		_cur_station = 0;
		for (auto& w : _station_list) {
			w->Coroutine();
		}

		while (_cur_station < _station_list.size()) {
			_station_list[_cur_station]->GetCoHandle().resume();
		}
	}

private:
	std::vector<std::unique_ptr<Station>> _station_list;
	size_t _cur_station = 0;
};

NAMESPACE_END