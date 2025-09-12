/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#include "ThreadPool.h"

namespace pdal
{
	void ThreadPool::go()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_running)
			return;

		m_running = true;

		for (std::size_t i(0); i < m_numThreads; ++i)
		{
			m_threads.emplace_back([this]() { work(); });
		}
	}

	void ThreadPool::work()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_consumeCv.wait(lock, [this]()
			{
				return m_tasks.size() || !m_running;
			});

			if (m_tasks.size())
			{
				++m_outstanding;
				auto task(std::move(m_tasks.front()));
				m_tasks.pop();

				lock.unlock();

				// Notify add(), which may be waiting for a spot in the queue.
				m_produceCv.notify_all();

				std::string err;

				task();

				/**
				try
				{
					task();
				}
				catch (std::exception& e)
				{
					err = e.what();
				}
				catch (...)
				{
					err = "Unknown error";
				}
				**/

				lock.lock();
				--m_outstanding;
				if (err.size())
				{
					if (m_verbose)
						std::cout << "Exception in pool task: " << err << std::endl;
					m_errors.push_back(err);
				}
				lock.unlock();

				// Notify await(), which may be waiting for a running task.
				m_produceCv.notify_all();
			}
			else if (!m_running)
			{
				return;
			}
		}
	}
}
