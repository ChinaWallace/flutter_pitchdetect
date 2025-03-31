#include "PitchDetect.h"
#ifdef __ANDROID__
#include "../../android/app/src/main/cpp/pitch_wrapper.h"
#endif

#include <algorithm>

inline long long nowTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

BaseDetector::BaseDetector()
{
}

BaseDetector::~BaseDetector()
{
}

bool BaseDetector::isValidate(int channel, PitchResult& result)
{
	if (result.getDecibels() < m_noiseDecibels)
	{
		return false;
	}

	if (m_detectMode == DM_TUNER)
	{
		return true;
	}

	return true;

	if (!m_bFilterMode)
	{
		return true;
	}

	bool ret(false);

	std::string note = result.getNote();
	double pitch = result.getPitch();
	int decibels = result.getDecibels();

	auto& filterMap = m_PitchResultQueue;

	auto it = filterMap.find(note);

	if (it == filterMap.end())
	{
		filterMap[note] = new ThreeElementQueue<Info>();

		filterMap[note]->push(Info(note, pitch, decibels, nowTime()));
	}
	else
	{
		std::vector<Info> infos = filterMap[note]->get();

		if (std::abs(infos.back().decibels - decibels) > 1)
		{
			filterMap[note]->push(Info(note, pitch, decibels, nowTime()));

			if (filterMap[note]->isFull())
			{
				std::vector<Info> infos = filterMap[note]->get();

				if ((infos[0].decibels < infos[1].decibels) && (infos[2].decibels < infos[1].decibels))
				{
					ret = true;
				}

			}
		}
	}

	return ret;
}

SingleToneDetector::SingleToneDetector(int detectorBuffSize)
{
	m_detectorBuffSize = detectorBuffSize;
}

SingleToneDetector::~SingleToneDetector()
{
	m_stopDetect = true;
	m_detectThread->join();

	m_stopFilter = true;
	m_FilterThread->join();
}

bool SingleToneDetector::isValidate(int channel, PitchResult& result)
{
	if (result.getPitch() < 10)
	{
		return false;
	}

	return BaseDetector::isValidate(channel, result);
}

void SingleToneDetector::doStart()
{

	m_detectThread = new std::thread([&] 
	{
        std::shared_ptr<Pitch> pitchDector = std::shared_ptr<Pitch>(new Pitch(m_detectorBuffSize, m_sampleRate, 16, 1.0));

		for (;; )
		{
			if (m_stopDetect)
			{
				break;
			}

			bool hasData = false;

			auto& safeQueue = m_recordDatas;

			if (safeQueue.size() > m_detectorBuffSize)
			{
				std::vector<unsigned char> data;

				if (!safeQueue.get(data, m_detectorBuffSize))
				{
					continue;
				}

                if (m_detectorBuffSize==2048 || m_detectorBuffSize==8192)
                {
                    if (m_bFilterMode)
                    {
                        continue;
                    }
                }

				PitchResult result;

				if (!pitchDector->doPitchAnalysis((unsigned char*)&data[0], &result))
				{
					continue;
				}
				if (isValidate(0, result))
				{
                    emitsignal(0, result, "singletone");
//                  cocos2d::log("note = %s,result.getNote().c_str());
				}
			}else{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	});
}

ChordDetectorEx::ChordDetectorEx()
{
	m_detectorBuffSize = 8192;
}

ChordDetectorEx::~ChordDetectorEx()
{
	m_stopDetect = true;


	m_detectThread->join();

}

bool ChordDetectorEx::isValidate(int channel, PitchResult& result)
{
	if (result.getDecibels() < m_noiseDecibels)
	{
		return false;
	}

	if (!m_bFilterMode)
	{
		return true;
	}

	bool ret(false);

	std::string note = result.getNote();
	double pitch = result.getPitch();
	int decibels = result.getDecibels();

	auto& filterMap = m_PitchResultQueue;

	auto it = filterMap.find(note);

	if (it == filterMap.end())
	{
		filterMap[note] = new ThreeElementQueue<Info>();
	}

	if (filterMap[note]->isEmpty())
	{
		ret = true;
	}
	else
	{
		std::vector<Info> infos = filterMap[note]->get();

		if (decibels > infos.back().decibels + 2)
		{
			ret = true;
		}
	}

	filterMap[note]->push(Info(note, pitch, decibels, nowTime()));

	return ret;
}

void ChordDetectorEx::doStart()
{

	m_detectThread = new std::thread([&]
	{
		std::shared_ptr<Pitch> pitchDector = std::shared_ptr<Pitch>(new Pitch(m_detectorBuffSize, m_sampleRate, 16, 1.0));

		for (;; )
		{
			if (m_stopDetect)
			{
				break;
			}

			auto& safeQueue = m_recordDatas;

			if (safeQueue.size() > m_detectorBuffSize)
			{
				std::vector<unsigned char> data;

				if (!safeQueue.get(data, m_detectorBuffSize))
				{
					continue;
				}

				PitchResult result;

				if (!pitchDector->doChordAnalysis((unsigned char*)&data[0], &result, 1.0))
				{
					continue;
				}

				if (isValidate(0, result))
				{
					emitsignal(0, result, "chord");
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DoubleToneDetector::DoubleToneDetector()
{
	m_detectorBuffSize = 8192;
}

DoubleToneDetector::~DoubleToneDetector()
{
	m_stopDetect = true;


	m_detectThread->join();

}

bool DoubleToneDetector::isValidate(int channel, PitchResult& result)
{
	if (result.getDecibels() < m_noiseDecibels)
	{
		return false;
	}

	return true;
}

void DoubleToneDetector::doStart()
{
	m_detectThread = new std::thread([&]
		{

		std::shared_ptr<Pitch> pitchDector = std::shared_ptr<Pitch>(new Pitch(m_detectorBuffSize, m_sampleRate, 16, 1.5));

			for (;; )
			{
				if (m_stopDetect)
				{
					break;
				}

				auto& safeQueue = m_recordDatas;

				if (safeQueue.size() > m_detectorBuffSize)
				{
					std::vector<unsigned char> data;

					if (!safeQueue.get(data, m_detectorBuffSize))
					{
						continue;
					}

					PitchResult result;

					if (!pitchDector->caculateMidiSpectrum((unsigned char*)&data[0], &result, _refFrequency))
					{
						continue;
					}

					m_doubleTonesignal(0, result.multimidi, result.midimagnitude);
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PitchDetect::PitchDetect()
{
    m_bStart = false;

    m_singleToneDetector = new SingleToneDetector(4096);
    m_kilesingleToneDetector = new SingleToneDetector(2048);
    m_k8192singleToneDetector = new SingleToneDetector(8192);

    m_chordDetector = new ChordDetectorEx;
    m_doubleToneDetector = new DoubleToneDetector;

    if (!m_bStart)
    {
        m_bStart = true;

        m_singleToneDetector->start();
        m_kilesingleToneDetector->start();
        m_k8192singleToneDetector->start();
        m_chordDetector->start();
        m_doubleToneDetector->start();
    }
}

PitchDetect::PitchDetect(Callback callback) : callback(callback)
{
    PitchDetect();
}


PitchDetect::~PitchDetect()
{
	m_singleToneDetector->stop();
	m_kilesingleToneDetector->stop();
	m_k8192singleToneDetector->stop();
	m_chordDetector->stop();

	m_doubleToneDetector->stop();

	if (m_bRecording)
	{
		stopRecord();
	}
}


void PitchDetect::startRecord()
{

    setDetectMode(DM_TUNER);
    setNoiseDecibels(50);
    setFilterMode(false);

    setDetectSignal([&](int channel, const std::string& note, double pitch, int decibel, const std::string& type, int detectorbuffsize)
                         {
                             nlohmann::json j;
                             j["channel"] = channel;
                             j["noteName"] = note;
                             j["pitch"] = pitch;
                             j["decibel"] = decibel;
                             j["noteType"] = type;
                             j["detectorbuffsize"] = detectorbuffsize;
                             j["type"] = 2;

                             // 输出 JSON 对象为字符串
                             std::string message = j.dump();

#ifdef __ANDROID__
                        sendStringToKotlin(message);
#else
                        callback(message);
#endif

                         });

}


void PitchDetect::stopRecord()
{
	m_bStopRecord = true;
}


void PitchDetect::recvData(int channel, unsigned char* data, int size)
{

	/*if (channel != 0)
	{
		return;
	}*/


	if (m_detectMode & DM_SINGLE_TONE_LOW)
	{
		m_singleToneDetector->push_data(channel, data, size);
	}

	if (m_detectMode & DM_SINGLE_TONE_HIGH)
	{
		m_kilesingleToneDetector->push_data(channel, data, size);
	}

	if (m_detectMode & DM_CHORD)
	{
		m_chordDetector->push_data(channel, data, size);
	}

	if (m_detectMode & DM_DOUBLE_TONE)
	{
		m_doubleToneDetector->push_data(channel, data, size);
	}

	if (m_detectMode & DM_TUNER)
	{
		m_singleToneDetector->push_data(channel, data, size);
	}

	if (m_detectMode & DM_NO_ANALYSIS)
	{

	}

}

