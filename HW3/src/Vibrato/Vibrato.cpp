
// standard headers

// project headers
#include <limits>
#include "MUSI6106Config.h"
#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Vibrato.h"
#include "Lfo.h"

/*!
 * Interface
 **/
Error_t CVibrato::create(CVibrato *&pCVibrato)
{
    pCVibrato = new CVibrato();
    
    if (!pCVibrato) {
        return kUnknownError;
    }
    return kNoError;

}

Error_t CVibrato::destroy(CVibrato *&pCVibrato)
{
    if (!pCVibrato) {
        return kUnknownError;
    }
    
    delete pCVibrato;
    pCVibrato = 0;
    
    return kNoError;
}

Error_t CVibrato::init(float fSampleRate, int iNumChannels, float fModFrequency = 0, float fAmplitude = 0)
{
    //! the modulation frequency should not be too large and the amplitude should be within the range [0, 1]
    if (fSampleRate <= 0 ||
        iNumChannels <= 0 ||
        fModFrequency < 0 || fModFrequency > (fSampleRate / 4) ||
        fAmplitude < 0 || fAmplitude > 1) {
        return kFunctionInvalidArgsError;
    }

    //! if arguments are valid
    reset();
    m_bIsInitialized = true;
    
    //! set again the range
    m_aafParamRange[kModulationFrequency][upperBound] = fSampleRate / 4;
    m_aafParamRange[kModulationFrequency][lowerBound] = 0;
    m_aafParamRange[kAmplitude][upperBound] = 1;
    m_aafParamRange[kAmplitude][lowerBound] = 0;
    
    //! compute the ring buffer length
    m_iDelayInSample = (int)(delayFactor * fSampleRate);
    int iDelayLineLength = 2 * m_iDelayInSample + 1;

    //! initialize the ring buffer
    m_ppCRingBuffer = new CRingBuffer<float>*[iNumChannels];
    for (int i = 0; i < iNumChannels; i++) {
        m_ppCRingBuffer[i] = new CRingBuffer<float>(iDelayLineLength);
        
        //! move the write pointer
        m_ppCRingBuffer[i]->setWriteIdx(m_iDelayInSample);
    }
    
    
    //! initialize the LFO
    CLfo::create(m_pCLfo);
    m_pCLfo->init(CLfo::kSine, fModFrequency, fSampleRate);

    //! set the value of parameters
    m_afParam[kModulationFrequency] = fModFrequency;
    m_afParam[kAmplitude] = fAmplitude;
    
    m_fSampleRate = fSampleRate;
    m_iNumerOfChannels = iNumChannels;
    
    return kNoError;
}

Error_t CVibrato::reset()
{
    m_bIsInitialized = false;
    
    //! delete LFO
    CLfo::destroy(m_pCLfo);
    
    //! delete 2-D ring buffer
    for (int i = 0; i < m_iNumerOfChannels; i++) {
        delete m_ppCRingBuffer[i];
    }
    delete [] m_ppCRingBuffer;
    m_ppCRingBuffer = 0;
    
    //! set other memnbers to the default value
    for (int i = 0; i < kNumVibratoParams; i++) {
        m_afParam[i] = 0;
        m_aafParamRange[i][upperBound] = 0;
        m_aafParamRange[i][lowerBound] = 0;
    }
    m_fSampleRate = 0;
    m_iNumerOfChannels = 0;
    m_iDelayInSample = 0;

    return kNoError;
}

Error_t CVibrato::setParam(CVibrato::VibratoParam_t eParam, float fParamValue)
{
    if (!m_bIsInitialized) {
        return kNotInitializedError;
    }
    
    if (!isInParamRange(eParam, fParamValue)) {
        return kFunctionInvalidArgsError;
    }
    
    switch (eParam) {
        case kModulationFrequency:
            m_afParam[eParam] = fParamValue;
            m_pCLfo->setFrequency(fParamValue);
            break;
        case kAmplitude:
            m_afParam[eParam] = fParamValue;
            break;
        default:
            return kFunctionInvalidArgsError;
    }
    return kNoError;
}

float CVibrato::getParam(CVibrato::VibratoParam_t eParam)
{
    if (!m_bIsInitialized) {
        return kNotInitializedError;
    }
    switch (eParam) {
        case kModulationFrequency:
            return m_afParam[eParam];
        case kAmplitude:
            return m_afParam[eParam];
        default:
            return kFunctionIllegalCallError;
    }
}

Error_t CVibrato::process(float **ppfInputBuffer, float **ppfOutputBuffer, float iNumberOfFrames)
{
    for (int i = 0; i < iNumberOfFrames; i++) {
        float fSineValue = m_pCLfo->generateNextValue();
        float fIndexOffset = fSineValue * m_afParam[kAmplitude] * m_iDelayInSample;
        for (int j = 0; j < m_iNumerOfChannels; j++) {
            m_ppCRingBuffer[j]->putPostInc(ppfInputBuffer[j][i]);
            ppfOutputBuffer[j][i] = m_ppCRingBuffer[j]->get(fIndexOffset);
            m_ppCRingBuffer[j]->getPostInc();

        }
    }
    return kNoError;
}

/*!
 * Private members
 **/
CVibrato::CVibrato():
    m_bIsInitialized(false),
    m_pCLfo(0),
    m_ppCRingBuffer(0),
    m_fSampleRate(0),
    m_iNumerOfChannels(0)
{
    for (int i = 0; i < kNumVibratoParams; i++) {
        m_afParam[i] = 0;
        m_aafParamRange[i][upperBound] = 0;
        m_aafParamRange[i][lowerBound] = 0;
    }
}

CVibrato::~CVibrato()
{
    reset();
}

bool CVibrato::isInParamRange(CVibrato::VibratoParam_t eParam, float fValue)
{
    if (fValue < m_aafParamRange[eParam][lowerBound] ||
        fValue > m_aafParamRange[eParam][upperBound])
    {
        return false;
    }
    else
    {
        return true;
    }
}

