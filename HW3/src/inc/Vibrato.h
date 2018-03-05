#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"

// forward declaration
class CLfo;
template <class T>
class CRingBuffer;

/*
 * brief explanation about your class-interface design
 */

class CVibrato
{
public:
    
    //! list of paramters for the vibrato
    enum VibratoParam_t
    {
        kAmplitude,
        
        kModulationFrequency,
        
        kNumVibratoParams
    };
    
    /*! creates a new vibrato instance
     \param pCVibrato: pointer to the new class
     \return Error_t
     */
    static Error_t create (CVibrato*& pCVibrato);
    
    /*! destroys a vibrato instance
     \param pCVibrato: pointer to the instance to be destroyed
     \return Error_t
     */
    static Error_t destroy(CVibrato*& pCVibrato);
    
    /*! initializes a vibrato instance
     \param fSampleRate
     \param iNumChannels: number of channels of signal to process
     \param fModFrequency: modulation frequency of the vibrato
     \param fAmplitude: normalized vibrato amplitude from 0 to 1
     \return Error_t
     */
    Error_t init (float fSampleRate, int iNumChannels, float fModFrequency, float fAmplitude);
    
    /*! resets the internal variables （requires new call of init)
     \return Error_t
     */
    Error_t reset ();
    
    /*! sets a comb filter paramter
     \param eParam: the paramter type to set
     \param fParamValue: value of the paramter to set
     \return Error_t
     */
    Error_t setParam (VibratoParam_t eParam, float fParamValue);
    
    /*! return the value of the specified paramter
     \param eParam: the paramter type to get
     \return float
     */
    float getParam (VibratoParam_t eParam);

    /*! processes one block of audio
     \param ppfInputBuffer: input buffer [numChannels][iNumberOfFrames]
     \param ppfOutputBuffer: output buffer [numChannels][iNumberOfFrames]
     \param iNumberOfFrames: buffer length (per channel)
     \param iNumberOfChannesl
     \return Error_t
     */
    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, float iNumberOfFrames);

private:
    CVibrato();
    virtual ~CVibrato();
    bool isInParamRange (VibratoParam_t eParam, float fValue);
    
    //! member variables
    
    const int lowerBound = 0;
    const int upperBound = 1;
    
    /*! the vibrato amplitude is related to Fs
     *  since the pitch shift is the difference of the delay length,
     *  this factor used to compute the delay line length
     */
    
//    both 0.0003 and 0.5 get tested, 0.0003 is better from a perception perspective and 0.5 is better from a test perspective (0.5 will result in a bigger delay length). In practice 0.0003 should be used in order to get a good vibrato effect
    const float delayFactor = 0.0003;
//    const float delayFactor = 0.5;
    
    
    bool m_bIsInitialized;  //!< internal bool to check whether the init function has been called
    
    CLfo *m_pCLfo;
    CRingBuffer<float> **m_ppCRingBuffer;
    
    float m_afParam[kNumVibratoParams];     //!< stores the amplitude in sample and modulation frequency
    float m_aafParamRange[kNumVibratoParams][2];

    float m_fSampleRate;
    int   m_iNumerOfChannels;
    int   m_iDelayInSample;
    
};

#endif // #if !defined(__Vibrato_hdr__)
