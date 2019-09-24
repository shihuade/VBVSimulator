//
//  VBVOnDemand.cpp
//  VBVSimulation
//
//  Created by edward.shi on 4/2/19.
//  Copyright © 2019 edward.shi. All rights reserved.
//

#include "VBVOnDemand.hpp"
#include "string.h"
#include "stdlib.h"

CVBVSimulator::CVBVSimulator()
{
}

CVBVSimulator::~CVBVSimulator()
{
}

int CVBVSimulator::initOutputFile(FILE* pFile, string sFileName)
{
    pFile = fopen(sFileName.c_str(), "w+");
    if (pFile == NULL) {
        printf("failed to open file! \n");
        return -1;
    }
    
    fprintf(pFile, "EncOrder, DTS, PTS, Frmbits, ToltalBits, MaxInputBits, MinInputBits, DTS, MaxVBVFill, DTS, MinVBVFill, \n");

    return 0;
}

void CVBVSimulator::outputVBVStatus(TVBVSimulator* pVBVSimulator)
{
    int32_t iIdx = (pVBVSimulator->iEncNum - 1 + pVBVSimulator->iListLen) % pVBVSimulator->iListLen;

    fprintf(pVBVSimulator->pVBVStaticInfo, "%4d, %-5.3f, %-5.3f, %6d, %8d, %8d, %8d, %-5.3f, %4d, %4d,\n",
            pVBVSimulator->iEncNum,
            pVBVSimulator->pFrameInfo[iIdx].fDTS,
            pVBVSimulator->pFrameInfo[iIdx].fPTS,
            pVBVSimulator->pFrameInfo[iIdx].iBits,
            pVBVSimulator->iTotalDecBits,
            pVBVSimulator->iMaxInputbits,
            pVBVSimulator->iMinInputBits,
            pVBVSimulator->pFrameInfo[iIdx].fDTS,
            pVBVSimulator->iMaxVBVFill,
            pVBVSimulator->iMinVBVFill);
}

void CVBVSimulator::initVBVCfg(TVBVCfg* pCfg, int32_t iTargetBR, int32_t iVBVBufferSize, int32_t iMaxVBVBR,
                               double fInitalPoint, double fFPS)
{
    memset(pCfg, 0, sizeof(TVBVCfg));

    pCfg->iTargetBR = MAX(1,iTargetBR * 1000);
    pCfg->iVBVBufferSize = iVBVBufferSize * 1000;
    pCfg->iMaxVBVBitrate = MAX(pCfg->iTargetBR, iMaxVBVBR * 1000);
    pCfg->fInitalPoint = fInitalPoint;
    pCfg->fFPS = fFPS;
    pCfg->fVBVDuration = double (pCfg->iVBVBufferSize) / pCfg->iMaxVBVBitrate;
    pCfg->fVBVDuration = MIN(100, pCfg->fVBVDuration);
    pCfg->fVBVBuffingTime = pCfg->fVBVDuration * pCfg->fInitalPoint;
}

int32_t CVBVSimulator::inintVBVSimulator(TVBVSimulator* pVBVSimulator, TVBVCfg* pVBVCfg, string sFileName)
{
    memset(pVBVSimulator, 0, sizeof(TVBVSimulator));
    memcpy(&pVBVSimulator->sCfg, pVBVCfg, sizeof(TVBVCfg));
    
    pVBVSimulator->iListLen = (pVBVCfg->fFPS + 1.0) * (pVBVSimulator->sCfg.fVBVDuration + 1.0);
    
    pVBVSimulator->pFrameInfo = new TFrameInfo[pVBVSimulator->iListLen];
    if (!pVBVSimulator->pFrameInfo) {
        printf("  failed to allocate mem for frame info! \n");
        return -1;
    }

    pVBVSimulator->iMaxVBVFill = pVBVSimulator->sCfg.iVBVBufferSize * pVBVSimulator->sCfg.fInitalPoint
                                - pVBVSimulator->sCfg.iMaxVBVBitrate / pVBVSimulator->sCfg.fFPS;
    pVBVSimulator->iMinVBVFill =  pVBVSimulator->sCfg.iVBVBufferSize * pVBVSimulator->sCfg.fInitalPoint
                                - pVBVSimulator->sCfg.iTargetBR / pVBVSimulator->sCfg.fFPS;
    
    pVBVSimulator->pVBVStaticInfo = fopen(sFileName.c_str(), "w+");
    if (pVBVSimulator->pVBVStaticInfo == NULL) {
        printf("failed to open file! \n");
        return -1;
    }
    
    fprintf(pVBVSimulator->pVBVStaticInfo, "EncOrder, DTS, PTS, Frmbits, ToltalBits, MaxInputBits, MinInputBits, DTS, MaxVBVFill, MinVBVFill, \n");

    return 0;
}

void CVBVSimulator::updateVBVSimulator(TVBVSimulator* pVBVSimulator, int32_t iFrameBits, double fPTS, double fDTS)
{
    int32_t iIdx = pVBVSimulator->iEncNum % pVBVSimulator->iListLen;
    int32_t iPreIdx = (pVBVSimulator->iEncNum -1 + pVBVSimulator->iListLen) % pVBVSimulator->iListLen;
    double fDelataDTS = pVBVSimulator->iEncNum > 0 ? (fDTS - pVBVSimulator->pFrameInfo[iPreIdx].fDTS)
                                                   : (1 / pVBVSimulator->sCfg.fFPS);
    
//    printf(" --EncNum=%3d, iPreIdx=%d, iIdx=%d, preDTS=%-5.3f, curDTS=%-5.3f, DelataDTS=%-5.3f, \n",
//           pVBVSimulator->iEncNum, iPreIdx, iIdx,
//           pVBVSimulator->pFrameInfo[iPreIdx].fDTS,
//           pVBVSimulator->pFrameInfo[iIdx].fDTS,
//           fDelataDTS);
    
    pVBVSimulator->pFrameInfo[iIdx].fPTS = fPTS;
    pVBVSimulator->pFrameInfo[iIdx].fDTS = fDTS;
    pVBVSimulator->pFrameInfo[iIdx].iBits = iFrameBits;

    
    pVBVSimulator->iMaxInputbits += (pVBVSimulator->sCfg.iMaxVBVBitrate * fDelataDTS);
    pVBVSimulator->iMinInputBits += (pVBVSimulator->sCfg.iTargetBR * fDelataDTS);

    printf("  Max-Min[%d, %d] -->", pVBVSimulator->iMaxVBVFill, pVBVSimulator->iMinVBVFill);

    pVBVSimulator->iMaxVBVFill += (pVBVSimulator->sCfg.iMaxVBVBitrate * fDelataDTS);
    pVBVSimulator->iMinVBVFill += (pVBVSimulator->sCfg.iTargetBR * fDelataDTS);
    
    printf(" Max-Min[%d, %d] -->", pVBVSimulator->iMaxVBVFill, pVBVSimulator->iMinVBVFill);

    pVBVSimulator->iMaxVBVFill = MIN(pVBVSimulator->iMaxVBVFill, pVBVSimulator->sCfg.iVBVBufferSize);
    pVBVSimulator->iMinVBVFill = MIN(pVBVSimulator->iMinVBVFill, pVBVSimulator->sCfg.iVBVBufferSize);
    
    printf(" Max-Min[%d, %d] -->", pVBVSimulator->iMaxVBVFill, pVBVSimulator->iMinVBVFill);


    pVBVSimulator->iMaxVBVFill -= iFrameBits;
    pVBVSimulator->iMinVBVFill -= iFrameBits;
    printf(" Max-Min[%d, %d] -->",  pVBVSimulator->iMaxVBVFill, pVBVSimulator->iMinVBVFill);

    pVBVSimulator->iMaxVBVFill = MAX(0,  pVBVSimulator->iMaxVBVFill);
    pVBVSimulator->iMinVBVFill = MAX(0,  pVBVSimulator->iMinVBVFill);

    printf("Max-Min[%d, %d] \n", pVBVSimulator->iMaxVBVFill, pVBVSimulator->iMinVBVFill);
    
    pVBVSimulator->iEncNum++;
}

void CVBVSimulator::releaseVBVSimulator(TVBVSimulator* pVBVSimulator)
{
    if (pVBVSimulator && pVBVSimulator->pFrameInfo) {
        delete [] pVBVSimulator->pFrameInfo;
    }
    pVBVSimulator->pFrameInfo = NULL;
    
    if (pVBVSimulator->pVBVStaticInfo) {
        fclose(pVBVSimulator->pVBVStaticInfo);
        pVBVSimulator->pVBVStaticInfo = NULL;
    }
}

void CVBVSimulator::constraintBRForVBV()
{
    int32_t iFrameNum = 1200;
    int32_t iFrameBits = 0;
    int32_t iRandSymb = 0;
    double fRandBits = 0;
    double fPTS = 0, fDTS = 0;
    
    srand((unsigned)(time(NULL)));
    
    
    TVBVCfg sCfg01, sCfg02;
    TVBVSimulator sVBVSimlu01, sVBVSimlu02;
    
    initVBVCfg(&sCfg01, 1000, 2000, 1500, 0.9, 10);
    initVBVCfg(&sCfg02, 1000, 2000, 1500, 0.6, 10);

    inintVBVSimulator(&sVBVSimlu01, &sCfg01, "VBV_01.csv");
    inintVBVSimulator(&sVBVSimlu02, &sCfg02, "VBV_04.csv");

    for(int32_t i = 0; i < iFrameNum; i++) {
        fPTS = i / sCfg01.fFPS;
        fDTS = i / sCfg01.fFPS;
        iFrameBits = sCfg01.iTargetBR / sCfg01.fFPS;
        updateVBVSimulator(&sVBVSimlu01, iFrameBits, fPTS, fDTS);
        outputVBVStatus(&sVBVSimlu01);

        fPTS = i / sCfg02.fFPS;
        fDTS = i / sCfg02.fFPS;
        iRandSymb = rand();
        iRandSymb = (iRandSymb % 2 == 0) ? (-1) : 1;
        if ((i > 200 && i < 400) || (i > 700 && i < 800)) {
            iRandSymb = -1;
        } else if ((i > 400 &&i < 600) || (i > 900 && i < 1000)) {
            iRandSymb = 1;
        }
        fRandBits = (rand() % 80) / 100.0;

        iFrameBits = sCfg02.iTargetBR / sCfg02.fFPS * 1.3;
        iFrameBits = iFrameBits * ( 1 + iRandSymb * fRandBits);
        updateVBVSimulator(&sVBVSimlu02, iFrameBits, fPTS, fDTS);
        outputVBVStatus(&sVBVSimlu02);
    }
    
    releaseVBVSimulator(&sVBVSimlu01);
    releaseVBVSimulator(&sVBVSimlu02);
}


void CVBVSimulator::vbvOnDemandModel()
{
    
}

