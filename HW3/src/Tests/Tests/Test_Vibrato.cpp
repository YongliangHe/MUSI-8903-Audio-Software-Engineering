#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <iostream>

#include "UnitTest++.h"

#include "Synthesis.h"
#include "Vector.h"
#include "ErrorDef.h"
#include "Vibrato.h"
#include "Lfo.h"

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

        CLfo *m_pCLfo = 0;
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
        const int iNumFrames = 999;

        /*! parameters for a frequency for which no interpolation
         *  is needed (in the implementation, the reference
         *  wavetable has a length of 2048);
         */
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
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-3);
        }


        //! checks the interpolation case with a random Fs and desired frequency and calling reset
        CSynthesis::generateSine(pfSigToCompare, iRandomDesiredFrequency, iSampleRateForUnitInc, iBufferLength);
        m_pCLfo->reset();
        m_pCLfo->init(CLfo::kSine, iRandomDesiredFrequency, iSampleRateForUnitInc);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-3);
        }

        //! checks the interpolation case with a random Fs and desired frequency and calling reset
        CSynthesis::generateSine(pfSigToCompare, iRandomDesiredFrequency, iRandomSampleRate, iBufferLength);
        m_pCLfo->reset();
        m_pCLfo->init(CLfo::kSine, iRandomDesiredFrequency, iRandomSampleRate);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-3);
        }


        //! successively processing
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare + iNumFrames, m_fBufferToFill[i], iNumFrames, 1e-3);
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

    TEST_FIXTURE(LFOData, testTwoCycleCase)
    {
        const int iNumChannels = 2;
        const int iNumFrames = 4096;

        //! parameters for a frequency for which no interpolation is needed
        const int iSampleRateForUnitInc = 2048;
        const int iDesiredLfoFrequencyForUnitInc = 1;

        //! generates the expected signal
        float *pfSigToCompare = new float [iNumFrames];
        CSynthesis::generateSine(pfSigToCompare, iDesiredLfoFrequencyForUnitInc, iSampleRateForUnitInc, iNumFrames);

        //! generates two cycles of sinewave using the LFO instance
        m_fBufferToFill = new float* [iNumChannels];
        for (int i = 0; i < iNumChannels; i++) {
            m_fBufferToFill[i] = new float[iNumFrames]();
        }

        m_pCLfo->init(CLfo::kSine, iDesiredLfoFrequencyForUnitInc, iSampleRateForUnitInc);
        m_pCLfo->generateNextAudioBlock(m_fBufferToFill, iNumChannels, iNumFrames);
        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-4);
        }

    }
    TEST_FIXTURE(LFOData, testGenerateNextValue)
    {
        const int iNumChannels = 3;
        const int iNumFrames = 999;
        const int iRandomSampleRate = 3423;
        const int iRandomLfoFrequency = 123;

        //! generates the expected signal
        float *pfSigToCompare = new float [iNumFrames];
        CSynthesis::generateSine(pfSigToCompare, iRandomLfoFrequency, iRandomSampleRate, iNumFrames);

        //! allocate memory to store the output signal
        m_fBufferToFill = new float* [iNumChannels];
        for (int i = 0; i < iNumChannels; i++) {
            m_fBufferToFill[i] = new float[iNumFrames]();
        }

        m_pCLfo->init(CLfo::kSine, iRandomLfoFrequency, iRandomSampleRate);
        for (int i = 0; i < iNumFrames; i++) {
            float temp = m_pCLfo->generateNextValue();
            for (int j = 0; j < iNumChannels; j++) {
                m_fBufferToFill[j][i] = temp;
            }
        }

        for (int i = 0; i < iNumChannels; i++) {
            CHECK_ARRAY_CLOSE(pfSigToCompare, m_fBufferToFill[i], iNumFrames, 1e-3);
        }

    }
}

SUITE(Vibrato)
{
    struct VibratoData
    {
        VibratoData()
        {
            CVibrato::create(m_pCVibrato);
        }

        ~VibratoData()
        {
            CVibrato::destroy(m_pCVibrato);
        }

        // e.g., a reusable process() function
        float ** newTwoDimensionalArray(int iNumchannels, int iNumFrames)
        {
            float **ppRes = new float*[iNumchannels];
            for (int i = 0; i < iNumchannels; i++) {
                ppRes[i] = new float[iNumFrames]();
            }
            return ppRes;
        }

        void deleteTwoDimensionalArray(float **ppArrayToDelete, int iNumchannels, int iNumFrames)
        {
            for (int i = 0; i < iNumchannels; i++) {
                delete[] ppArrayToDelete[i];
            }
            delete[] ppArrayToDelete;
            ppArrayToDelete = 0;
        }

        // e.g., a member vibrato object to be reused in each test
        CVibrato *m_pCVibrato;
    };

    TEST(MyTestWithNoFixture)
    {
        // e.g., allocate & deallocate local data for testing
    }

    TEST_FIXTURE(VibratoData, TestInitialization)
    {
        CHECK_EQUAL(kNotInitializedError, m_pCVibrato->getParam(CVibrato::kModulationFrequency));
        CHECK_EQUAL(kNotInitializedError, m_pCVibrato->setParam(CVibrato::kAmplitude, 0.5));
    }

    TEST_FIXTURE(VibratoData, TestInitArguments)
    {
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(0, 1, 1, 0.5));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(1, 0, 1, 0.5));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(1, 1, -1, 0.5));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(1024, 1, 512, 0.5));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(1024, 1, -1, 0.5));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->init(1024, 1, 100, 5));
        CHECK_EQUAL(kNoError, m_pCVibrato->init(1024, 1, 100, 0.5));
        CHECK_EQUAL(kNoError, m_pCVibrato->init(1024, 1, 256, 0.5));
    }

    TEST_FIXTURE(VibratoData, TestSetParam)
    {
        m_pCVibrato->init(44100, 2, 10, 0.5);
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->setParam(CVibrato::kModulationFrequency, 44100 / 4 + 1));
        CHECK_EQUAL(kNoError, m_pCVibrato->setParam(CVibrato::kModulationFrequency, 44100 / 4));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->setParam(CVibrato::kAmplitude, -1));
        CHECK_EQUAL(kNoError, m_pCVibrato->setParam(CVibrato::kAmplitude, 1));
        CHECK_EQUAL(kNoError, m_pCVibrato->setParam(CVibrato::kAmplitude, 0));
        CHECK_EQUAL(kFunctionInvalidArgsError, m_pCVibrato->setParam(CVibrato::kNumVibratoParams, 0));
    }

    TEST_FIXTURE(VibratoData, TestGetParam)
    {
        m_pCVibrato->init(44100, 2, 0, 0);
        CHECK_EQUAL(0, m_pCVibrato->getParam(CVibrato::kModulationFrequency));
        m_pCVibrato->setParam(CVibrato::kModulationFrequency, 10);
        CHECK_EQUAL(10, m_pCVibrato->getParam(CVibrato::kModulationFrequency));

        CHECK_EQUAL(0, m_pCVibrato->getParam(CVibrato::kAmplitude));
        m_pCVibrato->setParam(CVibrato::kAmplitude, 1);
        CHECK_EQUAL(1, m_pCVibrato->getParam(CVibrato::kAmplitude));

        m_pCVibrato->setParam(CVibrato::kAmplitude, 1.1);
        CHECK_EQUAL(1, m_pCVibrato->getParam(CVibrato::kAmplitude));
    }

    TEST_FIXTURE(VibratoData, TestReset)
    {
        m_pCVibrato->init(44100, 2, 0, 0);
        CHECK_EQUAL(0, m_pCVibrato->getParam(CVibrato::kAmplitude));
        m_pCVibrato->reset();
        CHECK_EQUAL(kNotInitializedError, m_pCVibrato->getParam(CVibrato::kAmplitude));
        m_pCVibrato->init(100, 1, 1, 0.5);
        m_pCVibrato->setParam(CVibrato::kAmplitude, 1);
        CHECK_EQUAL(1, m_pCVibrato->getParam(CVibrato::kAmplitude));
    }

    /////////////////////////////////////////////////////////////////

    //! below are some tests for the algorithms
    TEST_FIXTURE(VibratoData, TestZeroAmplitude)
    {
        const float fSampleRate = 100;
        const int iNumChannels = 1;
        const float fModFrequency = 10;
        const float fAmplitude = 0;
        const int iSignalLengthInSample = 123;

        //! this is what in the implementation
        const float fDelayFactor = 0.0003;
//        const float fDelayFactor = 0.5;
        const int iDelayInSample = (int)(fDelayFactor * fSampleRate);

        m_pCVibrato->init(fSampleRate, iNumChannels, fModFrequency, fAmplitude);

        float **ppInput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);
        float **ppOutput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);

        for (int i = 0; i < iNumChannels; i++) {
            ppInput[i][0] = 10;
            ppInput[i][1] = 20;
            ppInput[i][2] = 30;
        }
        m_pCVibrato->process(ppInput, ppOutput, iSignalLengthInSample);
        for (int i = 0; i < iDelayInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput[j][i]);
            }
        }
        for (int i = 0; i < 3; i++) {       //!< 3 is signal length(10, 20, 30,)
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(ppInput[j][i], ppOutput[j][i + iDelayInSample]);
            }
        }
        for (int i = iDelayInSample + 3; i < iSignalLengthInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput[j][i]);
            }
        }
    }

    TEST_FIXTURE(VibratoData, TestDC)
    {
        const float fSampleRate = 100;
        const int iNumChannels = 7;
        const float fModFrequency = 20;
        const float fAmplitude = 0.2;
        const int iSignalLengthInSample = 200;

        //! this is what in the implementation
        const float fDelayFactor = 0.0003;
//        const float fDelayFactor = 0.5;
        const int iDelayInSample = (int)(fDelayFactor * fSampleRate);

        m_pCVibrato->init(fSampleRate, iNumChannels, fModFrequency, fAmplitude);

        float **ppInput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);
        float **ppOutput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);
        for (int i = 0 ; i < iNumChannels; i++) {
            for (int j = 0; j < iSignalLengthInSample; j++) {
                ppInput[i][j] = 1;
            }
        }

        //! the expected value after the delay line is filled up
        m_pCVibrato->process(ppInput, ppOutput, iSignalLengthInSample);
        for (int i = 2 * iDelayInSample + 1; i < iSignalLengthInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(1, ppOutput[j][i]);
            }
        }
    }

    TEST_FIXTURE(VibratoData, TestVaringInputBlockSize)
    {
        const int iSize1 = 22;
        const int iSize2 = 321;
        const int iNumChannels = 4;
        const float fSampleRate = 13;
        const float fModeFrequency = 2;
        const float fAmplitude = 0;

        const float fDelayFactor = 0.0003;
//        const float fDelayFactor = 0.5;
        const int iDelayInSample = (int)(fDelayFactor * fSampleRate);

        float **ppInput1 = newTwoDimensionalArray(iNumChannels, iSize1);
        float **ppOutput1 = newTwoDimensionalArray(iNumChannels, iSize1);
        float **ppInput2 = newTwoDimensionalArray(iNumChannels, iSize2);
        float **ppOutput2 = newTwoDimensionalArray(iNumChannels, iSize2);

        m_pCVibrato->init(fSampleRate, iNumChannels, fModeFrequency, fAmplitude);
        //! initialization
        ppInput1[1][0] = 111;
        ppInput1[3][0] = 333;
        ppInput2[2][0] = 22;
        ppInput2[0][0] = 44;

        m_pCVibrato->process(ppInput1, ppOutput1, iSize1);
        for (int i = 0; i < iDelayInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput1[j][i]);
            }
        }
        for (int i = 0; i < 3; i++) {       //!< 3 is signal length(10, 20, 30,)
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(ppInput1[j][i], ppOutput1[j][i + iDelayInSample]);
            }
        }
        for (int i = iDelayInSample + 3; i < iSize1; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput1[j][i]);
            }
        }


        //! change the input block size
        m_pCVibrato->process(ppInput2, ppOutput2, iSize2);
        for (int i = 0; i < iDelayInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput2[j][i]);
            }
        }
        for (int i = 0; i < 3; i++) {       //!< 3 is signal length(10, 20, 30,)
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(ppInput2[j][i], ppOutput2[j][i + iDelayInSample]);
            }
        }
        for (int i = iDelayInSample + 3; i < iSize2; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput2[j][i]);
            }
        }
    }

    TEST_FIXTURE(VibratoData, TestZeroInput)
    {
        const int fSampleRate = 7654;
        const int iNumChannels = 5;
        const float fModFrequency = 1112;
        const float fAmplitude = 1;
        const int iSignalLengthInSample = 12345;

        m_pCVibrato->init(fSampleRate, iNumChannels, fModFrequency, fAmplitude);

        float **ppInput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);
        float **ppOutput = newTwoDimensionalArray(iNumChannels, iSignalLengthInSample);

        m_pCVibrato->process(ppInput, ppOutput, iSignalLengthInSample);
        for (int i = 0; i < iSignalLengthInSample; i++) {
            for (int j = 0; j < iNumChannels; j++) {
                CHECK_EQUAL(0, ppOutput[j][i]);
            }
        }
    }
}

#endif //WITH_TESTS
