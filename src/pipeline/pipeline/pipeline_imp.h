#pragma once
#include <vector>
#include <memory>
#include "coro.h"
#include "worker.h"
#include "ring_buffer.h"

namespace byfxxm {
	using Fifo = RingBuffer<Code*, 4>;
	struct Station {
		Worker* worker = nullptr;
		std::unique_ptr<Fifo> next = std::make_unique<Fifo>();
		Fifo* prev = nullptr;
		bool done = false;
	};

	class PipelineImp {
	public:
		void Start() {
			if (_station_list.empty())
				return;

			_co.SetMain([this](CoMainHelper* helper, void*) {
				while (!_stop && _cur_station < _station_list.size()) {
					helper->SwitchToSub(_cur_station);
				}
				});

			for (auto& sta : _station_list) {
				_co.AddSub([&, this](CoSubHelper* helper, void*) {
					auto read = [&, this]()->Code* {
						Code* code = nullptr;
						while (!sta->prev->Read(code)) {
							if (_cur_station > 0 && _station_list[_cur_station - 1]->done) {
								_station_list[_cur_station]->done = true;
								++_cur_station;
							}
							else {
								--_cur_station;
							}
							helper->SwitchToMain();
						}

						return code;
					};

					auto write = [&, this](Code* code) {
						while (!sta->next->Write(code)) {
							++_cur_station;
							helper->SwitchToMain();
						}
					};

					if (sta->prev == nullptr) {
						sta->worker->Do(nullptr, write);
						write(nullptr);
					}
					else {
						while (1) {
							Code* code = read();
							if (!code)
								break;

							sta->worker->Do(code, write);
						}
					}

					sta->done = true;
					++_cur_station;
					helper->SwitchToMain();
					});
			}

			_stop = false;
			_co.Run();
		}

		void Stop() {
			_stop = true;
		}

		void AddWorker(void* worker) {
			auto station = std::make_unique<Station>();
			station->worker = static_cast<Worker*>(worker);
			if (!_station_list.empty())
				station->prev = _station_list.back()->next.get();

			_station_list.emplace_back(std::move(station));
		}

	private:
		std::vector<std::unique_ptr<Station>> _station_list;
		size_t _cur_station = 0;
		Coro _co;
		bool _stop = false;
	};
}