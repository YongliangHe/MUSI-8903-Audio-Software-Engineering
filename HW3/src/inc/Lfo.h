#if !defined(__Lfo_hdr__)
#define __Lfo_hdr__

#define _USE_MATH_DEFINES
#include <math.h>

#include "ErrorDef.h"
#include "Synthesis.h"
#include "RingBuffer.h"
#include <iostream>

class CLfo
{
public:
    enum Waveform
    {
        kSine,
        
        kNumWaveforms
    };
    
    
    /*! creates a new LFO instance
     \param pCLfo: pointer to the new class
     \return Error_t
     */
    static Error_t create (CLfo*& pCLfo);
    
    /*! destroys a LFO instance
     \param pCLfo: pointer to the class to be destroyed
     \return Error_t
     */
    static Error_t destroy (CLfo*& pCLfo);
    
    /*! initializes a LFO instance
     \param eWaveform: waveform of the LFO
     \param fFrequency: frequency of the LFO
     \param dSampleRate: sample rate of the stream
     \return Error_t
     */
    Error_t init (Waveform eWaveform, float fFrequency, double dSampleRate);
    
    /*! resets the internal variables (requires new call of init)
     \return Error_t
     */
    Error_t reset ();
    
    /*! sets the frequency of the LFO
     \param fFrequency: frequency of the LFO
     \return Error_t
     */
    Error_t setFrequency (float fFrequency);
    
    /*! gets the current frequency of the LFO
     \return float
     */
    float getFrequency () const;
    
    /*! generates next block of signal
     \param ppfBufferToFill: [channels][iNumFrames]
     \param iNumChannels: number of channels to process
     \return Error_t
     */
    Error_t generateNextAudioBlock (float **ppfBufferToFill, int iNumChannels, int iNumberOfFrames);

private:
    
    CLfo();
    virtual ~CLfo();
    
    const int BUFFER_LENGTH = 2048;

    bool m_bIsInitialized;
    
//    float m_iIncInSample;   //!< increment value used to skip through the buffer per sample
    float m_fFrequency;
    
    double m_dSampleRate;
    
    CRingBuffer<float> *m_pCRingbuffer;
    
    void writeWaveformToBuffer (Waveform eWaveform);    //!< fill one cycle of waveform signal into buffer
    void writeSine();

};

//==========================================================
// public function definitions

Error_t CLfo::create(CLfo *&pCLfo)
{
    pCLfo = new CLfo;
    
    if (!pCLfo) {
        return kUnknownError;
    }
    return kNoError;
}

//==========================================================
Error_t CLfo::destroy(CLfo *&pCLfo)
{
    if (!pCLfo) {
        return kUnknownError;
    }
    
    pCLfo->reset();
    
    delete pCLfo;
    pCLfo = 0;
    
    return kNoError;
    
}

//==========================================================
Error_t CLfo::reset()
{
    delete m_pCRingbuffer;
    m_pCRingbuffer = 0;
    
//    m_iIncInSample = 0;
    m_fFrequency = 0;
    m_bIsInitialized = false;
    m_dSampleRate = 0;
    return kNoError;
}

//==========================================================
float CLfo::getFrequency() const
{
    if (!m_bIsInitialized) {
        return kNotInitializedError;
    }
    return m_fFrequency;
}

//==========================================================
CLfo::~CLfo()
{
    reset();
}

//==========================================================
CLfo::CLfo():
    m_bIsInitialized(false),
//    m_iIncInSample(0),
    m_fFrequency(0),
    m_dSampleRate(0),
    m_pCRingbuffer(0)
{}

//==========================================================
Error_t CLfo::setFrequency(float fFrequency)
{
    if (!m_bIsInitialized) {
        return kNotInitializedError;
    }
    
    if (fFrequency < 0) {
        return kFunctionInvalidArgsError;
    }
    m_fFrequency = fFrequency;
//    m_iIncInSample = BUFFER_LENGTH / m_dSampleRate * m_fFrequency;
    return kNoError;
}

//==========================================================
Error_t CLfo::init(Waveform eWaveform, float fFrequency, double dSampleRate)
{
    reset();
    
    if (fFrequency < 0 ||
        dSampleRate <= 0) {
        return kFunctionInvalidArgsError;
    }
    
    m_dSampleRate = dSampleRate;
    m_fFrequency = fFrequency;
    
    m_pCRingbuffer = new CRingBuffer<float>(BUFFER_LENGTH);

    writeWaveformToBuffer(eWaveform);
    
    m_bIsInitialized = true;

    return kNoError;

}

//==========================================================
Error_t CLfo::generateNextAudioBlock(float **ppfBufferToFill, int iNumChannels, int iNumberOfFrames)
{
    if (!m_bIsInitialized) {
        return kNotInitializedError;
    }
    

    float fIncInSample = BUFFER_LENGTH / m_dSampleRate * m_fFrequency;
    for (int i = 0; i < iNumChannels; i++) {
        for (int j = 0; j < iNumberOfFrames; j++) {
            
            ppfBufferToFill[i][j] = m_pCRingbuffer->get(j * fIncInSample);
        }
    }
    
    return kNoError;
}

//!==========================================================
//! private function definitions
void CLfo::writeWaveformToBuffer(Waveform eWaveform)
{
    switch (eWaveform) {
        case kSine:
            writeSine();
            break;
            
        default:
            break;
    }
}

//==========================================================
void CLfo::writeSine()
{
    float angleDelta = 2 * M_PI / BUFFER_LENGTH;
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        m_pCRingbuffer->putPostInc(std::sin(i * angleDelta));
    }
    
}



#endif // __Lfo_hdr__
