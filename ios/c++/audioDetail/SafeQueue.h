#pragma once

#include <vector>
#include <mutex>

template<class T>
class SafeQueue
{
public:
	SafeQueue() {}
	~SafeQueue() {}

public:
	void push(const T& t)
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		m_Queue.push_back(t);
	}

	void push(T* t, int size)
	{
		std::lock_guard<std::mutex> locker(m_Mutex);
		m_Queue.insert(m_Queue.end(), t, t + size);
	}

	const T& first()
	{
		return m_Queue.front();
	}

	void pop()
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		if (!m_Queue.empty())
		{
			m_Queue.erase(m_Queue.begin());
		}
	}

	void erase()
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		m_Queue.clear();
	}

	int size()
	{
		return m_Queue.size();
	}

	bool isEmpty()
	{
		return m_Queue.empty();
	}

protected:
	std::vector<T> m_Queue;
	std::mutex m_Mutex;
};

template<class T>
class AudioSafeQueue : public SafeQueue<T>
{
public:
	bool get(std::vector<T>& data, int size)
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		if (m_Queue.size() >= size)
		{
			int eraseSize = size / 4;
			if (eraseSize < 1024)
			{
				eraseSize = 1024;
			}

			data.insert(data.begin(), m_Queue.begin(), m_Queue.begin() + size);
			m_Queue.erase(m_Queue.begin(), m_Queue.begin() + eraseSize);

			return true;
		}

		return false;
	}



    void push(const T& t)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);

        m_Queue.push_back(t);
    }

    void push(T* t, int size)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_Queue.insert(m_Queue.end(), t, t + size);
    }

    const T& first()
    {
        return m_Queue.front();
    }

    void pop()
    {
        std::lock_guard<std::mutex> locker(m_Mutex);

        if (!m_Queue.empty())
        {
            m_Queue.erase(m_Queue.begin());
        }
    }

    void erase()
    {
        std::lock_guard<std::mutex> locker(m_Mutex);

        m_Queue.clear();
    }

    int size()
    {
        return m_Queue.size();
    }

    bool isEmpty()
    {
        return m_Queue.empty();
    }

protected:
    std::vector<T> m_Queue;
    std::mutex m_Mutex;
};

template<class T>
class ThreeElementQueue
{
public:
	ThreeElementQueue() {}
	~ThreeElementQueue() {}

public:
	std::vector<T> get()
	{
		return m_Queue;
	}

	void push(const T& t)
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		m_Queue.push_back(t);

		if (m_Queue.size() > 3)
		{
			m_Queue.erase(m_Queue.begin());
		}
	}

	bool isFull()
	{
		return m_Queue.size() == 3;
	}

	void clear()
	{
		std::lock_guard<std::mutex> locker(m_Mutex);

		m_Queue.clear();
	}

	int size()
	{
		return m_Queue.size();
	}

	bool isEmpty()
	{
		return m_Queue.empty();
	}

protected:
	std::vector<T> m_Queue;
	std::mutex m_Mutex;
};


