#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>
#include <algorithm>

#include "UnitTest++.h"

#include "Synthesis.h"
#include "Vector.h"
#include "ErrorDef.h"

#include "Vibrato.h"

SUITE(LFO)
{
    struct LFOData {
        LFOData()
        {
            CLfo::create(m_pCLfo);

        }
        
        ~LFOData()
        {
            CLfo::destroy(m_pCLfo);
            
        }
        
        CLfo *m_pCLfo;
        float **m_fBufferToFill;
    };
    
    //! call functions when not initialized or passing wrong parameters
    TEST_FIXTURE(LFOData, testParamAndInitialization)
    {
        const int iNumChannels = 1;
        const int iNumFrames = 1;
        float **ppfBufferToFill = new float*[iNumChannels];
        for (int i = 0; i < iNumChannels; i++) {
            ppfBufferToFill[i] = new float[iNumFrames]();
        }
        
        CHECK_EQUAL(kNotInitializedError, m_pCLfo->getFrequency());
        CHECK_EQUAL(kNotInitializedError, m_pCLfo->setFrequency(0));
        CHECK_EQUAL(kNotInitializedError, m_pCLfo->generateNextAudioBlock(ppfBufferToFill, iNumChannels, iNumFrames));
        
        m_pCLfo->init(CLfo::kSine, 1, 1);
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCLfo->setFrequency(-1));
        m_pCLfo->reset();
        CHECK_EQUAL(kNotInitializedError, m_pCLfo->setFrequency(1));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCLfo->init(CLfo::kSine, 0, 0));
        CHECK_EQUAL(kNoError, m_pCLfo->init(CLfo::kSine, 0, 1));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCLfo->init(CLfo::kSine, -1, 1));
        
    }
    TEST_FIXTURE(LFOData, testGenerateNextAudioBlock)
    {
        
        const int iNumChannels = 2;
        const int iNumFrames = 10;
        
        //! parameters for a frequency for which no interpolation is needed
        const int iBufferLength = 2048;
        const int iSampleRateForUnitInc = 2048;
        const int iDesiredLfoFrequencyForUnitInc = 1;
        
        //! parameters for a frequency for which interpolation is needed
        const int iRandomSampleRate = 44100;
        const int iRandomDesiredFrequency = 100;
        
        //! checks the non-interpolation case
        float *pfSigToCompare = new float[iBufferLength]();
        CSynthesis::generateSine(pfSigToCompare, iDesiredLfoFrequencyForUnitInc, iSampleRateForUnitInc, iBufferLength);

        m_fBufferToFill = new float* [iNumChannels];
        for (int i = 0; i < iNumChannels; i++) {
            m_fBufferToFill[i] = new float[iNumFrames]();
        }
        
        m_pCLfo->init(CLfo::kSine, iDesiredLfoFrequencyForUnitInc, iSampleRateForUnitInc);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-4);
        }
        
        
        //! checks the interpolation case with a random Fs and desired frequency and calling reset
        CSynthesis::generateSine(pfSigToCompare, iRandomDesiredFrequency, iSampleRateForUnitInc, iBufferLength);
        m_pCLfo->reset();
        m_pCLfo->init(CLfo::kSine, iRandomDesiredFrequency, iSampleRateForUnitInc);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-4);
        }

        //! checks the interpolation case with a random Fs and desired frequency and calling reset
        CSynthesis::generateSine(pfSigToCompare, iRandomDesiredFrequency, iRandomSampleRate, iBufferLength);
        m_pCLfo->reset();
        m_pCLfo->init(CLfo::kSine, iRandomDesiredFrequency, iRandomSampleRate);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-4);
        }
        
        
        //! successively processing
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare + iNumFrames, m_fBufferToFill[i], iNumFrames, 1e-4);
        }
        
        
        //! deallocates memory
        delete [] pfSigToCompare;
        pfSigToCompare = 0;
        for (int i = 0; i < iNumChannels; i++) {
            delete [] m_fBufferToFill[i];
        }
        delete [] m_fBufferToFill;
        m_fBufferToFill = 0;

    }
}

SUITE(Vibrato)
{
    struct VibratoData
    {
        VibratoData()
        {
            // setup
            // e.g., allocate a vibrato object and test signal (newly created for each test case)
        }

        ~VibratoData()
        {
            // teardown
            // e.g., deallocate the vibrato object and test signal
        }

        // e.g., a reusable process() function

        // e.g., a member vibrato object to be reused in each test
    };

    TEST(MyTestWithNoFixture)
    {
        // e.g., allocate & deallocate local data for testing
    }

    TEST_FIXTURE(VibratoData, MyTestWithFixture)
    {
        // e.g., you can use the "VibratoData" contents
    }
}

#endif //WITH_TESTS
