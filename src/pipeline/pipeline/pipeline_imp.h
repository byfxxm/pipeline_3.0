#pragma once
#include <vector>
#include <memory>
#include <thread>
#include "ring_buffer.h"

namespace byfxxm
{
	using Fifo = RingBuffer<std::unique_ptr<Code>, 4>;
	struct Station
	{
		std::unique_ptr<Worker> worker;
		std::unique_ptr<Fifo> fifo = std::make_unique<Fifo>();
		Station *prev = nullptr;
		bool done = false;
	};

	struct PipelineException : public std::exception
	{
	};

	class PipelineImp : public Pipeline
	{
	public:
		void Start() override
		{
			if (_station_list.empty())
				return;

			_thread = std::thread([this]()
								  {
					_DoStation(0);
					_stop = false; });
		}

		void Stop() override
		{
			_stop = true;
		}

		void Wait() override
		{
			if (_thread.joinable())
				_thread.join();
		}

		void AddWorker(std::unique_ptr<Worker> worker) override
		{
			if (!worker)
				throw std::exception();

			auto station = std::make_unique<Station>();
			station->worker = std::move(worker);
			if (!_station_list.empty())
				station->prev = _station_list.back().get();

			_station_list.push_back(std::move(station));
		}

	private:
		void _DoStation(size_t station_idex) const
		{
			if (station_idex == _station_list.size())
				return;

			auto cur_station = _station_list[station_idex].get();
			auto finish = [&]()
			{
				while (!cur_station->fifo->Write(nullptr))
				{
					if (_stop)
						return;

					_DoStation(station_idex + 1);
				}
				_DoStation(station_idex + 1);
				cur_station->done = true;
			};

			for (;;)
			{
				std::unique_ptr<Code> code;
				if (cur_station->prev && !cur_station->prev->done)
				{
					while (_stop || !cur_station->prev->fifo->Read(code))
					{
						return;
					}

					if (!code)
					{
						finish();
						return;
					}
				}

				auto res = cur_station->worker->Do(std::move(code), [&](std::unique_ptr<Code> code)
												   {
					while (!cur_station->fifo->Write(std::move(code))) {
						if (_stop)
							return;

						_DoStation(station_idex + 1);
					} });

				if (station_idex == 0 || !res)
				{
					finish();
					return;
				}
			}
		}

	private:
		std::vector<std::unique_ptr<Station>> _station_list;
		std::atomic<bool> _stop = false;
		std::thread _thread;
	};
}