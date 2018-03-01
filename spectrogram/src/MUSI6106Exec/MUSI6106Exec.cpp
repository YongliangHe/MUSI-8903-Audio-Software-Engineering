
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "RingBuffer.h"
#include "Fft.h"
using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 4096;
    static const int        kOverLapSize = 2048;
    static const int        kMagLength = kBlockSize / 2 + 1;

    clock_t                 time = 0;

    float                   **ppfNewDataJustRead = 0;
    float                   **ppfWholeBlockData = 0;
    float                   **ppfMagnitudeData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    CFft                    *pCFft = 0;
    
    //! ring buffer declaration
    CRingBuffer<float> **ppCRingBuffer;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
    }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        return -1;
    }
    //////////////////////////////////////////////////////////////////////////////
    //! set CFft instance
    CFft::createInstance(pCFft);
    pCFft->initInstance(kBlockSize, 1, CFft::kWindowHamming, CFft::kNoWindow);
    
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfNewDataJustRead = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfNewDataJustRead[i] = new float[kOverLapSize]();
    
    ppfWholeBlockData = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfWholeBlockData[i] = new float[kBlockSize]();
    }
    
    ppCRingBuffer = new CRingBuffer<float>* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppCRingBuffer[i] = new CRingBuffer<float>(kBlockSize);
    }
    
    
    ppfMagnitudeData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfMagnitudeData[i] = new float[kMagLength]();
    }
    

    time = clock();
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFile->isEof())
    {
        
        //! move the read pointer by the value of the overlap size
        int tempIndex = 0;
        for (int i = 0; i < stFileSpec.iNumChannels; i++) {
            tempIndex = ppCRingBuffer[i]->getReadIdx();
            ppCRingBuffer[i]->setReadIdx(tempIndex - kOverLapSize);
        }
        

        long long iNumFrames = kBlockSize - kOverLapSize;
        phAudioFile->readData(ppfNewDataJustRead, iNumFrames);
        
        for (int i = 0; i < stFileSpec.iNumChannels; i++) {
            ppCRingBuffer[i]->putPostInc(ppfNewDataJustRead[i], kOverLapSize);
            tempIndex = ppCRingBuffer[i]->getWriteIdx();
        }
        
        cout << "write index: " << tempIndex << endl;
        
        //! get the content of the ring buffer
        for (int i = 0; i < stFileSpec.iNumChannels; i++) {
            ppCRingBuffer[i]->getPostInc(ppfWholeBlockData[i], kBlockSize);
        }
        
        
        
        //! compute fft
        for (int i = 0; i < stFileSpec.iNumChannels; i++) {
            pCFft->doFft(ppfWholeBlockData[i], ppfWholeBlockData[i]);
            pCFft->getMagnitude(ppfMagnitudeData[i], ppfWholeBlockData[i]);
        }
        cout << "\r" << "reading and writing";

//        for (int i = 0; i < kBlockSize; i++)
        for (int i = 0; i < kMagLength; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
//                hOutputFile << ppfWholeBlockData[c][i] << "\t";
                hOutputFile << ppfMagnitudeData[c][i] << "\t";
            }
            hOutputFile << endl;
        }
        
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfNewDataJustRead[i];
    delete[] ppfNewDataJustRead;
    ppfNewDataJustRead = 0;
    
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        delete [] ppfWholeBlockData[i];
    }
    delete [] ppfWholeBlockData;
    ppfWholeBlockData = 0;
    
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        delete [] ppfMagnitudeData[i];
    }
    delete [] ppfMagnitudeData;
    ppfMagnitudeData = 0;
    
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        delete ppCRingBuffer[i];
    }
    delete [] ppCRingBuffer;
    ppCRingBuffer = 0;
    
    CFft::destroyInstance(pCFft);
    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2018 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

