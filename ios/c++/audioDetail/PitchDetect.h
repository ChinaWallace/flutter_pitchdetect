#pragma once

#include "pitch.h"
#include <thread>
#include "SafeQueue.h"
#include <map>
#include <functional>
#include "../nlohmann/json.hpp"

#define MAX_CHANNEL 1

enum DetectedMode
{
	DM_NULL = 0x00,
	DM_SINGLE_TONE_LOW = 0x01,
	DM_SINGLE_TONE_HIGH = 0x02,
	DM_CHORD = 0x04,
	DM_DOUBLE_TONE = 0x08,
	DM_TUNER = 0x10,
	DM_NO_ANALYSIS = 0x20,
	DM_ALL = 0x30,
	DM_SINGLE_TONE = 0x40
};

struct Info
{
	Info() {}
	Info(const std::string& note, int pitch, int decibels, long long time)
	{
		this->note = note;
		this->pitch = pitch;
		this->decibels = decibels;
		this->time = time;
	}

	std::string note;
	int pitch = 0;
	int decibels = 0;
	long long time = 0;
};

typedef std::function<void(int, const std::string&, double, int, const std::string&, int detectorbuffsize)> DetectSignal;


class BaseDetector
{
public:
	BaseDetector();
	~BaseDetector();

public:
	void setFilterMode(bool on)
	{
		m_bFilterMode = on;
	}

	void setDetectMode(DetectedMode mode)
	{
		m_detectMode = mode;
	}

public:
	void start()
	{
		doStart();
	}

	void stop()
	{

	}

public:
	void setDetectSignal(DetectSignal signal)
	{
		m_detectSingal = signal;
	}

	void setNoiseDecibels(int decibels)
	{
		m_noiseDecibels = decibels;
	}

	void setSimpleRate(int simpleRate)
	{
		m_sampleRate = simpleRate;
	}

	void setTargetNote(const std::string& targetNote)
	{
		m_targetNote = targetNote;
	}

public:
	void push_data(int channel, unsigned char* data, int size)
	{
		if (channel == 0) //只使用第一通道的数据（陪练）
		{
			m_recordDatas.push(data, size);
		}	
	}

	void clear_data()
	{
		m_recordDatas.erase();
	}

protected:
	virtual void doStart() = 0;

protected:
	bool isValidate(int channel, PitchResult& result);

protected:
	void emitsignal(int channel, PitchResult& result, const std::string& type)
	{
		m_detectSingal(channel, result.getNote(), result.getPitch(), result.getDecibels(), type, m_detectorBuffSize);
	}

protected:
	bool m_stopDetect = false;

protected:
	AudioSafeQueue<unsigned char> m_recordDatas;

	std::map<std::string, ThreeElementQueue<Info>* > m_PitchResultQueue;

	std::map<std::string, ThreeElementQueue<Info>* > m_PitchResultQueue1[MAX_CHANNEL];
	DetectSignal m_detectSingal = nullptr;

	int m_noiseDecibels = 30 ;

	int m_sampleRate = 44100;

protected:
	bool m_bFilterMode = false;

private:
	DetectedMode m_detectMode = DM_NULL;

private:
	std::string m_targetNote;

protected:
	int m_detectorBuffSize;

protected:
	std::thread* m_detectThread;
};

class SingleToneDetector : public BaseDetector
{
public:
	SingleToneDetector(int detectorBuffSize);
	~SingleToneDetector();

protected:
	bool isValidate(int channel, PitchResult& result);

protected:
	virtual void doStart();

protected:
	bool m_stopFilter = false;
	std::thread* m_FilterThread;
};

typedef std::function<void(int, std::vector<int>, std::vector<float>)> DoubleToneDetectSignal;

class DoubleToneDetector : public BaseDetector
{
public:
	DoubleToneDetector();
	~DoubleToneDetector();

	void setReferenceFrequency(float ref)
	{
		_refFrequency = ref;
	}

	void setDoubleToneSignal(DoubleToneDetectSignal signal)
	{
		m_doubleTonesignal = signal;
	}

protected:
	bool isValidate(int channel, PitchResult& result);

protected:
	virtual void doStart();

	DoubleToneDetectSignal m_doubleTonesignal;

	float _refFrequency = 110.0f;
};

class ChordDetectorEx : public BaseDetector
{
public:
	ChordDetectorEx();
	~ChordDetectorEx();

protected:
	bool isValidate(int channel, PitchResult& result);

protected:
	virtual void doStart();

protected:
	bool m_stopFilter = false;
	std::thread* m_FilterThread;
};

class PitchDetect
{
public:

    using Callback = std::function<void(const std::string&)>;

    PitchDetect();
    PitchDetect(Callback callback);
	~PitchDetect();

public:

    Callback callback;

	void setReferenceFrequency(float ref)
	{
		m_doubleToneDetector->setReferenceFrequency(ref);
	}

	void setDoubleToneSignal(DoubleToneDetectSignal signal)
	{
		m_doubleToneDetector->setDoubleToneSignal(signal);
	}

	void setRecordDeviceName(const std::wstring& rcordDevice)
	{
		m_recordDeviceName = rcordDevice;
	}

	void setNeedRecord(bool bNeedRecord)
	{
		m_bNeedRecord = bNeedRecord;
	}

	void setDetectMode(DetectedMode mode)
	{
		m_detectMode = mode;

		m_singleToneDetector->clear_data();
		m_kilesingleToneDetector->clear_data();
		m_k8192singleToneDetector->clear_data();
		m_chordDetector->clear_data();
		m_doubleToneDetector->clear_data();

		m_singleToneDetector->setDetectMode(mode);
		m_kilesingleToneDetector->setDetectMode(mode);
		m_k8192singleToneDetector->setDetectMode(mode);
		m_chordDetector->setDetectMode(mode);
		m_doubleToneDetector->setDetectMode(mode);
	}

	void setFilterMode(bool on)
	{
		m_singleToneDetector->setFilterMode(on);
		m_kilesingleToneDetector->setFilterMode(on);
		m_k8192singleToneDetector->setFilterMode(on);
		m_chordDetector->setFilterMode(on);
		m_doubleToneDetector->setFilterMode(on);
	}

	void setTargetNote(const std::string& mode, const std::string& note)
	{
		if (mode == "singletone")
		{
			m_singleToneDetector->setTargetNote(note);
			m_kilesingleToneDetector->setTargetNote(note);
			m_k8192singleToneDetector->setTargetNote(note);
		}
		else if (mode == "doubletone")
		{
			m_doubleToneDetector->setTargetNote(note);
		}
	}

public:
	void setDetectSignal(DetectSignal signal)
	{
		m_singleToneDetector->setDetectSignal(signal) ;
		m_kilesingleToneDetector->setDetectSignal(signal);
		m_k8192singleToneDetector->setDetectSignal(signal);
		m_chordDetector->setDetectSignal(signal);
		m_doubleToneDetector->setDetectSignal(signal);
	}

	void setNoiseDecibels(int decibels)
	{
		m_singleToneDetector->setNoiseDecibels(decibels);
		m_kilesingleToneDetector->setNoiseDecibels(decibels);
		m_k8192singleToneDetector->setNoiseDecibels(decibels);
		m_chordDetector->setNoiseDecibels(decibels);
		m_doubleToneDetector->setNoiseDecibels(decibels);
	}

public:
	void setDeviceReadySignal(std::function<void(bool)> signal)
	{
		m_deviceReadySignal = signal;
	}

public:
	void startRecord();

	void stopRecord();

	bool isRecording()
	{
		return m_bRecording;
	}

private:
	void doStart();

private:
	void recordAudioStreamThread();

public:
	void recvData(int channel, unsigned char* data, int size);

private:
	bool m_bRecording = false;

private:
	bool m_bStopRecord = false;
	std::thread* m_recordThread;

private:
	SingleToneDetector* m_singleToneDetector;
	SingleToneDetector* m_kilesingleToneDetector;
	SingleToneDetector* m_k8192singleToneDetector;
	ChordDetectorEx* m_chordDetector;
	DoubleToneDetector* m_doubleToneDetector;

private:
	DetectedMode m_detectMode = DM_NULL;

	std::wstring m_recordDeviceName;

private:
	std::function<void(bool)> m_deviceReadySignal;

private:
	bool m_bNeedRecord = false;

	bool m_bStart = false;
};

extern "C"{

//    void* initPitchDetect(){
//
//        PitchDetect* pit = new PitchDetect();
//        pit->setDetectMode(DM_NULL);
//        pit->setNoiseDecibels(50);
//        pit->setFilterMode(false);
//
//        pit->setDetectSignal([&](int channel, const std::string& note, double pitch, int decibel, const std::string& type, int detectorbuffsize)
//                             {
//                                 nlohmann::json j;
//                                 j["channel"] = channel;
//                                 j["noteName"] = note;
//                                 j["pitch"] = pitch;
//                                 j["decibel"] = decibel;
//                                 j["noteType"] = type;
//                                 j["detectorbuffsize"] = detectorbuffsize;
//
//                                 // 输出 JSON 对象为字符串
//                                 std::string message = j.dump();
//                                 callback(message);
//                             });
//
//        return pit;
//    }
//
//    void startPitchDetect(void* obj){
//        static_cast<PitchDetect*>(obj)->startRecord();
//    }
//    void stopPitchDetect(void* obj){
//        static_cast<PitchDetect*>(obj)->stopRecord();
//    }
}
