//
//  VBVOnDemand.hpp
//  VBVSimulation
//
//  Created by edward.shi on 4/2/19.
//  Copyright © 2019 edward.shi. All rights reserved.
//

#ifndef VBVOnDemand_hpp
#define VBVOnDemand_hpp

#include <stdio.h>
#include "string"

using namespace std;

#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b) )

struct TFrameInfo {
    int32_t iBits;
    double fPTS;
    double fDTS;
};

struct TVBVCfg {
    int32_t iTargetBR;
    int32_t iMaxVBVBitrate;
    int32_t iVBVBufferSize;
    double fFPS;
    double fInitalPoint;
    double fVBVDuration;
    double fVBVBuffingTime;
};

struct TVBVSimulator {
    TVBVCfg sCfg;

    int32_t iEncNum;
    int32_t iListLen;
    TFrameInfo* pFrameInfo;

    //VBV status
    int32_t iMaxInputbits;
    int32_t iMinInputBits;
    int32_t iTotalDecBits;
    int32_t iMaxVBVFill;
    int32_t iMinVBVFill;
    
    FILE* pVBVStaticInfo;
};

class CVBVSimulator {
public:
    CVBVSimulator();
    ~CVBVSimulator();
    
    int initOutputFile(FILE* pFile, string sFileName);
    void outputVBVStatus(TVBVSimulator* pVBVSimulator);
    
    void initVBVCfg(TVBVCfg* pCfg, int32_t iTargetBR, int32_t iVBVBufferSize, int32_t iMaxVBVBR,
                    double fInitalPoint, double fFPS);
    int32_t inintVBVSimulator(TVBVSimulator* pVBVSimulator, TVBVCfg* pVBVCfg, string sFileName);
    void updateVBVSimulator(TVBVSimulator* pVBVSimulator, int32_t iFrameBits, double fPTS, double fDTS);
    void releaseVBVSimulator(TVBVSimulator* pVBVSimulator);

    void constraintBRForVBV();
    void vbvOnDemandModel();
    
private:

};


#endif /* VBVOnDemand_hpp */
