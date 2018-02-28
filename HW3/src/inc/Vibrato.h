#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "Lfo.h"
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
    /*! processes one block of audio
     \param ppfInputBuffer: input buffer [numChannels][iNumberOfFrames]
     \param ppfOutputBuffer: output buffer [numChannels][iNumberOfFrames]
     \param iNumberOfFrames buffer length (per channel)
     \return Error_t
     */

private:
    CLfo *m_pCLfo;
    CRingBuffer<float> *m_pCRingBuffer;
    
    float m_fSampleRate;
};

#endif // #if !defined(__Vibrato_hdr__)
