#ifndef PITCH_ANALYSIS_H
#define PITCH_ANALYSIS_H

#define POW        powf
#define FLOOR      floor
#define COS        cosf

#define SQR(_a)   ((_a)*(_a))
#define DB2LIN(g) (POW(10.0,(g)*0.05))
#define ROUND(x)   FLOOR(x+.5)

#ifndef M_PI
#define PI         (3.14159265358979323846)
#else
#define PI         (M_PI)
#endif

#include <iostream>
#include <sstream>
#include <vector>

#if defined SOUND_ANALYSIS_LIBRARY
 #define PITCH_COMMON_DLLSPEC __declspec(dllexport)
#else
 #define PITCH_COMMON_DLLSPEC __declspec(dllimport)
#endif

/**
 *  @brief
 *      Result structure of Pitch analysis
 *
 *  @return
 *      getPitch - get the pitch result 'HZ' of the audio buffer
 *      getNote - get the note result, like "A2"/"C4" etc.
 *
 */


class /*PITCH_COMMON_DLLSPEC*/ PitchResult
{

public:

    PitchResult(){;}
    ~PitchResult(){;}

    double getPitch(){ return pitch; }
    std::string getNote(){ return note; }
    int getDecibels(){  return decibels;}

    void setPitch(double pitch){ this->pitch = pitch; }
    void setNote(std::string note){ this->note = note; }
    void setDecibels(int decibels){ this->decibels = decibels; }


private:
    std::string note;
    double pitch;
    int decibels;
};


/**
 *  @brief
 *      Do single tone analysis and chord analysis for the audio buffer
 *
 *  @param
 *      bufsize - buffer size of the audio binary buffer, unit: Byte, default: 4096
 *      samplerate - sample rate, default:44100
 *      bitpersample - the bit value of sample, 16bit/24bit etc.
 *      numOfChannel - number of channel, default: 1
 *
 *  @return
 *      PitchResult - the structure of value
 *
 *
 *  @details
 *      There is one suggest for the bufsize value, since it is better to get 2048 float
 *      array at least, so the buffer size should be larger than that, the formula about
 *      bufsize is as below:
 *
 *      float array size = bufsize * 8 / bitpersample / numOfChannel
 *
 *      if float array size is 2048, bitpersample is 16bit, 2 channel for the audio buffer
 *      the bufsize should be larger than 8192, better 16384, but 8192 should be enough
 *
 */

class  Pitch
{

public:

    Pitch(int bufsize = 4096, int samplerate = 44100, int bitpersample = 16, short numOfChannel = 1);
    ~Pitch();

    void setSampleRate(int samplerate){this->samplerate = samplerate;}
    void setNumOfChannel(int numOfChannel){this->numOfChannel = numOfChannel;}
    void setSampleSize(int bitpersample){this->bitpersample = bitpersample;}
    void setBufferSize(int bufsize){this->bufsize = bufsize;}

    /**
     * @brief
     *      set debug mode with printf to console
     * @param
     *      debugMode - the debug message will be printed when it sets true
     * @return
     *
     */
    void setDebugMode(bool debugMode){this->debugMode = debugMode;}

    /**
     * @brief
     *      do single tone analysis
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult
     *      tolerance - the tolerance about the pitch detect, hard to detect when it is smaller
     * @return
     *      boolean - true means the result is correct
     */
    bool doPitchAnalysis(unsigned char* data, PitchResult *result, double tolerance=0.85);

    /**
     * @brief
     *      do chord analysis
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult
     *      tolerance - the tolerance about the pitch detect, hard to detect when it is larger
     * @return
     *      boolean - true means the result is correct
     */
    bool doChordAnalysis(unsigned char* data, PitchResult *result , double tolerance = 1.5);

    /**
     * @brief
     *      do double note analysis
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult
     *      tolerance - the tolerance about the pitch detect, hard to detect when it is larger
     * @return
     *      boolean - true means the result is correct
     */
    bool doDoubleAnalysis(unsigned char* data, PitchResult *result , double tolerance = 1.5);



private:

    void doFFT(double* input, double* output);
    void initweight();
    void initWithBitValue();
    double getPeakPos(double * x, int pos);
    int getMinValue(double * data);
    void doHanningz(double* input, double* output);
    void handleData(unsigned char* input, double* output);
    int getDecibels(unsigned char* data);

    int pitchtomidi(double pitch);
    std::string miditonote(int midi);

    double doYin(double* data);
    double doYinFFT(double* data);
    std::string toString(int n);

    int bufsize;
    int samplerate;
    int fftsize;
    int bitpersample;
    int datasize;

    bool debugMode;

    double *weight;
    int short_period;
    double tol;

    short numOfChannel;


    double freqsdata[34] = {
         0.,    20.,    25.,   31.5,    40.,    50.,    63.,    80.,   100.,   125.,
       160.,   200.,   250.,   315.,   400.,   500.,   630.,   800.,  1000.,  1250.,
      1600.,  2000.,  2500.,  3150.,  4000.,  5000.,  6300.,  8000.,  9000., 10000.,
     12500., 15000., 20000., 25100
    };

    double weightdata[34] = {
      -75.8,  -70.1,  -60.8,  -52.1,  -44.2,  -37.5,  -31.3,  -25.6,  -20.9,  -16.5,
      -12.6,  -9.60,  -7.00,  -4.70,  -3.00,  -1.80,  -0.80,  -0.20,  -0.00,   0.50,
       1.60,   3.20,   5.40,   7.80,   8.10,   5.30,  -2.40,  -11.1,  -12.8,  -12.2,
      -7.40,  -17.8,  -17.8,  -17.8
    };

    std::string notestring[12] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };

};

#endif
