/*
 * All the paper which used as algrithm is commented out
 * And there are some changes in the codes because of performance and accuracy
 * And because it is always do the test by midi, please do not reference the result in the paper
 * Remeber to delete all the commented paper information when doing publish
 */
#ifndef PITCH_ANALYSIS_H
#define PITCH_ANALYSIS_H

/*
 * Please do not define SOUND_ANALYSIS_LIBRARY or  when you want to use this library
 * SOUND_ANALYSIS_LIBRARY is used for generated dll library
 * SOUND_UNIT_TEST is used for automatically test and CPU/Memory performance test with VLD
 */

#if defined SOUND_ANALYSIS_LIBRARY
 #define PITCH_COMMON_DLLSPEC __declspec(dllexport)
#elif defined SOUND_UNIT_TEST
 #define PITCH_COMMON_DLLSPEC
#else
 #define PITCH_COMMON_DLLSPEC __declspec(dllimport)
#endif

#include <vector>

/**
 *  @brief
 *      Result structure of Pitch analysis
 *
 *  @param
 *      foremidi - the forecast midi value which will be analysis in the multi pitch analysis
 *
 *  @return
 *      getNote - get the note result, like "A2"/"C4" etc.
 *      getLoudness - get the decibel with new loudness algrithm, the getDecibels is still supported
 *      getDecibels - the original one to get decibel of the sound
 *
 *      SINGLE NOTE:
 *      getPitch - get the pitch result 'HZ' of the audio buffer for sigle pitch
 *
 *
 *      CHORD:
 *      getChordKey - get the main key index of the chord,
 *                    main key index - ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]
 *      getChordScale - get the scale of the chord
 *                    scale with - [Minor, Major, Suspended, Dominant, Dimished5th, Augmented5th, Other]
 *                    for some chord algrithms, there is only Minor and Major, and Other.
 *      getChordType - get the 2th, 7th type of the chord, 0 means no value for this type.
 *
 *
 *      MULTIPITCH:
 *
 *      multimidi - the midi value list order by magnitude
 *      midimagnitude -  the maginitude of the midi reference by the multimidi, please do the filter when it is too smaller
 *
 */
class PitchResult
{

public:

    PitchResult(){;}
    ~PitchResult(){;}

    enum ChordScale {
        MINOR = 0,
        MAJOR = 1,
        SUSPENDED = 2,
        DOMINANT = 3,
        DIMISHED5TH = 4,
        AUGMENTED5TH = 5,
        OTHER = 6
    };

    double getPitch(){ return pitch; }
    std::string getNote(){ return note; }
    int getDecibels(){  return decibels; }

    int getChordKey(){  return chordkey; }
    int getChordScale(){  return chordscale; }
    int getChordType(){  return chordtype; }
    float getLoudness(){  return loudness; }

    void setPitch(double pitch){ this->pitch = pitch; }
    void setNote(std::string note){ this->note = note; }
    void setDecibels(int decibels){ this->decibels = decibels; }

    void setChordKey(int chordkey){ this->chordkey = chordkey; }
    void setChordScale(int chordscale){ this->chordscale = chordscale; }
    void setChordType(int chordtype){ this->chordtype = chordtype; }
    void setLoudness(float loudness){ this->loudness = loudness; }


private:
    std::string note;
    double pitch;
    int decibels;

    /**
      * the parameter is only for the chord as below:
      * key - the index for the note ( C, C#, D etc.)
      * scale - Major/Minor as ChordScale
      * type - 0, 2th or 7th
      */
    int chordkey;
    int chordscale;
    int chordtype;

    /*
     * Psychophysics: introduction to its perceptual, neural, and social prospects,
     * Stevens, S.S. (1975), Geraldine Stevens.
     * Transaction Publishers, ISBN 978-0-88738-643-5
     */
    float loudness;

public:
    /**
      * the parameter is for the multi pitch as below:
      * midi - use midi in vector instead of note name because it is easy to record
      * magnitude - the power of this midi value
      */
    std::vector<int> multimidi;
    std::vector<float> midimagnitude;

    /**
     * @brief forecast of the midi value for multi pitch detect
     */
    std::vector<int> foremidi;
};



/**
 *  @brief
 *      Result structure of Pitch analysis
 *
 *  @param
 *      foremidi - the forecast midi value which will be analysis in the multi pitch analysis
 *
 *  @return
 *      getNote - get the note result, like "A2"/"C4" etc.
 *      getLoudness - get the decibel with new loudness algrithm, the getDecibels is still supported
 *      getDecibels - the original one to get decibel of the sound
 *
 *      SINGLE NOTE:
 *      getPitch - get the pitch result 'HZ' of the audio buffer for sigle pitch
 *
 *
 *      CHORD:
 *      getChordKey - get the main key index of the chord,
 *                    main key index - ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]
 *      getChordScale - get the scale of the chord
 *                    scale with - [Minor, Major, Suspended, Dominant, Dimished5th, Augmented5th, Other]
 *                    for some chord algrithms, there is only Minor and Major, and Other.
 *      getChordType - get the 2th, 7th type of the chord, 0 means no value for this type.
 *
 *
 *      MULTIPITCH:
 *
 *      multimidi - the midi value list order by magnitude
 *      midimagnitude -  the maginitude of the midi reference by the multimidi, please do the filter when it is too smaller
 *
 */

struct OfflineNote
{
    int duration = 0; // The duration for this note
    int startpos = 0; // The start postion in one bar
    int midi = 0; // Midi value for this music
    int bar = 0; //The bar for the music
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
 *
 *      CHORD:
 *      useThreeChord - if it is true, then only A/Am chord will be recognized, when it is false, all the chords instead.
 *      chordAnalysisFunction - All the chord algrithm will be added into this library later
 *
 *      MULTIPITCH:
 *      fundamental_midi_range means the range for the multi pitch, like if there is no octave multi pitch , it should be 11
 *
 * **********************************************************************************************
 *      Please call function reinit() when change the parameter in runtime!!!!
 * **********************************************************************************************
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

class Pitch
{

public:

    enum ChordFunction {
        DEFAULT = 0,
        /*
         * Key Estimation in Electronic
         * Á. Faraldo, E. Gómez, S. Jordà, P.Herrera,
         * 2016.
         */
        KEIE = 1,
        /*
         * A multi-profile method for key estimation in edm.
         * Faraldo, Á., Jordà, S., & Herrera, P.
         * (2017, June).
         */
        EDM = 2

    };

    Pitch(int bufsize = 4096, int samplerate = 44100, int bitpersample = 16, short numOfChannel = 1);
    ~Pitch();

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
    /*
     * YIN, a fundamental frequency estimator for speech and music.
     * De Cheveigné, A., & Kawahara, H. 2002
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

    /*
     *
     * Real-Time Chord Recognition For Live Performance
     * A. M. Stark and M. D. Plumbley
     */
    bool doChordAnalysis(unsigned char* data, PitchResult *result , double tolerance = 3.0);

    /**
     * @deprecated
     *      because of performance issue, suggest to use caculateMidiSpectrum and caculateNoteSpectrum directly
     * @brief
     *      do double note analysis
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult
     *      tolerance - the tolerance about the pitch detect, hard to detect when it is larger
     * @return
     *      boolean - true means the result is correct
     */
    /*
     * 1) Multiple Fundamental Frequency Estimation by Summing Harmonic Amplitudes
     *    A. Klapuri (2006)
     * 2) Melody extraction from polyphonic music
     *    J. Salamon and E. Gómez (2012)
     * 3) Musicians and Machines: Bridging the Semantic Gap in Live Performance
     *    Adam M. Stark, University of London (2011)
     * 4) Peak Detection
     *    Julius Orion Smith, Standford
     * 5) Melody Extraction from Polyphonic Music Signals using Pitch Contour Characteristics.
     *    IEEE Transactions on Audio, Speech and Language Processing.
     * 6) Efficient spectral envelope estimation and its application to pitch shifting and envelope preservation
     *    A. Röbel and X. Rodet 2005.
     * 7) A perceptually motivated multiple-F0 estimation method
     *    Klapuri, Anssi
     */
    bool doDoubleAnalysis(unsigned char* data, PitchResult *result , double tolerance = 3.0);

    /**
     * @brief
     *      do dependent algrithm for note spectrum to easy to do quickly check
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult, since it just caculate the note value,
     *                  so multimidi is from 0-12, means C to B
     * @return
     *      boolean - true means the result is correct
     */
    bool caculateNoteSpectrum(unsigned char* data, PitchResult *result);

    /**
     * @brief
     *      do dependent midi for note spectrum to easy to do quickly check
     * @param
     *      data - binary data, unit Byte
     *      result - pitch result @see class PitchResult
     *      referenceFrequency - there are four values 55.0f/110.0f/220.0f/440.0f - A1/A2/A3/A4
     *                           e.g: B4-F4, since for midi, F4 < A4, so A3 is the choice
     * @return
     *      boolean - true means the result is correct
     */
    bool caculateMidiSpectrum(unsigned char* data, PitchResult *result, float referenceFrequency=110.0f);

    /**
     * @brief do the analysis for the whole song with CNN or RNN algrithm by deep learning.
     *
     * @return the seriers of the music midi value
     */
    /*
     * 1) An End-to-End Neural Network for Polyphonic Piano Music Transcription
     *    Siddharth Sigtia, Emmanouil Benetos, Simon Dixon
     * 2) On the Potential of Simple Framewise Approaches to Piano Transcription
     *    Rainer Kelz, Matthias Dorfer, Filip Korzeniowski, Sebastian Böck, Andreas Arzt, Gerhard Widmer
     * 3) An Experimental Analysis of the Entanglement Problem in Neural-Network-based Music Transcription Systems
     *    Rainer Kelz, Gerhard Widmer
     * 4) Modeling Temporal Dependencies in High-Dimensional Sequences: Application to Polyphonic Music Generation and Transcription
     *    Nicolas Boulanger-Lewandowski
     *
     */
    std::vector<OfflineNote> doOfflineAnalysis(unsigned char* data);

    /**
     *  Please call this parameter when the parameter is changed, like charbuffersize etc.
     */
    void reinit();

public:

    bool useThreeChord = true;
    ChordFunction chordAnalysisFunction = DEFAULT;


    int fundamental_midi_range = 11;

    /**
      * These three parameters is used for finding fundamental cent
      * It seems there should be different between guitar and zheng
      */
    float cent5DCent3 = 10.0f;
    float cent4DCent3 = 8.0f;
    float cent5DCent4 = 10.0f;

public:
    int charbuffersize;
    int samplerate;
    int bitpersample;
    short numOfChannel;


private:
    void* analysis;
    int getDecibels(unsigned char* data);

};

#endif
