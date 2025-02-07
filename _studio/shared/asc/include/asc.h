// Copyright (c) 2017-2020 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef _ASC_H_
#define _ASC_H_

#include <list>
#include <string>
#include "asc_structures.h"

namespace ns_asc {

#if defined(__AVX2__)
#define ASC_CPU_DISP_INIT_AVX2(func)        (func = (func ## _AVX2))
#define ASC_CPU_DISP_INIT_AVX2_SSE4_C(func) (m_AVX2_available ? ASC_CPU_DISP_INIT_AVX2(func) : ASC_CPU_DISP_INIT_SSE4_C(func))
#define ASC_CPU_DISP_INIT_AVX2_C(func)      (m_AVX2_available ? ASC_CPU_DISP_INIT_AVX2(func) : ASC_CPU_DISP_INIT_C(func))
#else
#define ASC_CPU_DISP_INIT_AVX2_SSE4_C       ASC_CPU_DISP_INIT_SSE4_C
#define ASC_CPU_DISP_INIT_AVX2_C            ASC_CPU_DISP_INIT_C
#endif

#define ASC_CPU_DISP_INIT_C(func)           (func = (func ## _C))
#define ASC_CPU_DISP_INIT_SSE4(func)        (func = (func ## _SSE4))
#define ASC_CPU_DISP_INIT_SSE4_C(func)      (m_SSE4_available ? ASC_CPU_DISP_INIT_SSE4(func) : ASC_CPU_DISP_INIT_C(func))


typedef void(*t_GainOffset)(pmfxU8 *pSrc, pmfxU8 *pDst, mfxU16 width, mfxU16 height, mfxU16 pitch, mfxI16 gainDiff);
typedef void(*t_RsCsCalc)(pmfxU8 pSrc, int srcPitch, int wblocks, int hblocks, pmfxU16 pRs, pmfxU16 pCs);
typedef void(*t_RsCsCalc_bound)(pmfxU16 pRs, pmfxU16 pCs, pmfxU16 pRsCs, pmfxU32 pRsFrame, pmfxU32 pCsFrame, int wblocks, int hblocks);
typedef void(*t_RsCsCalc_diff)(pmfxU16 pRs0, pmfxU16 pCs0, pmfxU16 pRs1, pmfxU16 pCs1, int wblocks, int hblocks, pmfxU32 pRsDiff, pmfxU32 pCsDiff);
typedef void(*t_ImageDiffHistogram)(pmfxU8 pSrc, pmfxU8 pRef, mfxU32 pitch, mfxU32 width, mfxU32 height, mfxI32 histogram[5], mfxI64 *pSrcDC, mfxI64 *pRefDC);
typedef mfxI16(*t_AvgLumaCalc)(pmfxU32 pAvgLineVal, int len);
typedef void(*t_ME_SAD_8x8_Block_Search)(mfxU8 *pSrc, mfxU8 *pRef, int pitch, int xrange, int yrange, mfxU16 *bestSAD, int *bestX, int *bestY);
typedef void(*t_ME_SAD_8x8_Block_FSearch)(mfxU8 *pSrc, mfxU8 *pRef, int pitch, int xrange, int yrange, mfxU32 *bestSAD, int *bestX, int *bestY);
typedef mfxStatus(*t_Calc_RaCa_pic)(mfxU8 *pPicY, mfxI32 width, mfxI32 height, mfxI32 pitch, mfxF64 &RsCs);

class ASCimageData {
public:
    ASCimageData();
    ASCYUV Image;
    ASCMVector
        *pInteger;
    mfxI32
        var,
        jtvar,
        mcjtvar;
    mfxI16
        tcor,
        mcTcor;
    mfxU32
        CsVal,
        RsVal;
    mfxI16
        avgval;
    mfxU16
        *Cs,
        *Rs,
        *RsCs,
        *SAD;
    mfxStatus InitFrame(ASCImDetails *pDetails);
    mfxStatus InitAuxFrame(ASCImDetails *pDetails);
    void Close();
};

typedef struct ASCvideoBuffer {
    ASCimageData
        layer;
    mfxI32
        frame_number,
        forward_reference,
        backward_reference;
} ASCVidSample;

typedef struct ASCextended_storage {
    mfxI32
        average;
    mfxI32
        avgSAD;
    mfxU32
        gopSize,
        lastSCdetectionDistance,
        detectedSch,
        pendingSch;
    ASCTSCstat
        **logic;
    // For Pdistance table selection
    pmfxI8
        PDistanceTable;
    ASCLayers
        size;
    bool
        firstFrame;
    ASCimageData
        gainCorrection;
    mfxU8
        control;
    mfxU32
        frameOrder;
}ASCVidRead;

class ASC {
public:
    ASC();
protected:
    mfxU32
        m_gpuImPitch,
        m_threadsWidth,
        m_threadsHeight;
    mfxU8
        *m_frameBkp;
    mfxI32
        m_gpuwidth,
        m_gpuheight;

    static const int subWidth = 128;
    static const int subHeight = 64;

    ASCVidRead *m_support;
    ASCVidData *m_dataIn;
    bool
        m_dataReady,
        m_is_LTR_on,
        m_ASCinitialized;
    mfxI32
        m_width,
        m_height,
        m_pitch;
    /**
    ****************************************************************
    * \Brief List of long term reference friendly frame detection
    *
    * The list keeps the history of ltr friendly frame detections,
    * each element in the listis made of a frame number <mfxI32> and
    * its detection <bool> as a pair.
    *
    */
    std::list<std::pair<mfxI32, bool> >
        ltr_check_history;

    int m_AVX2_available;
    int m_SSE4_available;
    t_GainOffset               GainOffset;
    t_RsCsCalc                 RsCsCalc_4x4;
    t_RsCsCalc_bound           RsCsCalc_bound;
    t_RsCsCalc_diff            RsCsCalc_diff;
    t_ImageDiffHistogram       ImageDiffHistogram;
    t_ME_SAD_8x8_Block_Search  ME_SAD_8x8_Block_Search;
    t_Calc_RaCa_pic            Calc_RaCa_pic;

    virtual void VidSample_dispose();

    void SubSample_Point(
        pmfxU8 pSrc, mfxU32 srcWidth, mfxU32 srcHeight, mfxU32 srcPitch,
        pmfxU8 pDst, mfxU32 dstWidth, mfxU32 dstHeight, mfxU32 dstPitch,
        mfxI16 &avgLuma);
    mfxStatus RsCsCalc();
    mfxI32 ShotDetect(ASCimageData& Data, ASCimageData& DataRef, ASCImDetails& imageInfo, ASCTSCstat *current, ASCTSCstat *reference, mfxU8 controlLevel);
    void MotionAnalysis(ASCVidSample *videoIn, ASCVidSample *videoRef, mfxU32 *TSC, mfxU16 *AFD, mfxU32 *MVdiffVal, mfxU32 *AbsMVSize, mfxU32 *AbsMVHSize, mfxU32 *AbsMVVSize, ASCLayers lyrIdx);

    typedef void(ASC::*t_resizeImg)(mfxU8 *frame, mfxI32 srcWidth, mfxI32 srcHeight, mfxI32 inputPitch, ns_asc::ASCLayers dstIdx, mfxU32 parity);
    t_resizeImg resizeFunc;

    void VidRead_dispose();
    mfxStatus SetWidth(mfxI32 Width);
    mfxStatus SetHeight(mfxI32 Height);
    mfxStatus SetPitch(mfxI32 Pitch);
    void SetNextField();
    mfxStatus SetDimensions(mfxI32 Width, mfxI32 Height, mfxI32 Pitch);
    void Setup_Environment();
    void SetUltraFastDetection();
    void Params_Init();
    void InitStruct();
    mfxStatus VidRead_Init();
    void VidSample_Init();
    virtual mfxStatus VidSample_Alloc();
    void SubSampleASC_ImagePro(mfxU8 *frame, mfxI32 srcWidth, mfxI32 srcHeight, mfxI32 inputPitch, ASCLayers dstIdx, mfxU32 parity);
    void SubSampleASC_ImageInt(mfxU8 *frame, mfxI32 srcWidth, mfxI32 srcHeight, mfxI32 inputPitch, ASCLayers dstIdx, mfxU32 parity);
    bool CompareStats(mfxU8 current, mfxU8 reference);
    bool DenoiseIFrameRec();
    bool FrameRepeatCheck();
    bool DoMCTFFilteringCheck();
    void DetectShotChangeFrame();
    void GeneralBufferRotation();
    void Put_LTR_Hint();
    ASC_LTR_DEC Continue_LTR_Mode(mfxU16 goodLTRLimit, mfxU16 badLTRLimit);
    void AscFrameAnalysis();
    mfxStatus SetInterlaceMode(ASCFTS interlaceMode);
public:

    virtual mfxStatus Init(mfxI32 Width, 
        mfxI32 Height, 
        mfxI32 Pitch, 
        mfxU32 PicStruct, 
        bool isCmSupported);
    virtual void Close();
    bool IsASCinitialized();

    void SetControlLevel(mfxU8 level);
    mfxStatus SetGoPSize(mfxU32 GoPSize);
    void ResetGoPSize();

    inline void SetParityTFF() { SetInterlaceMode(ASCtopfieldfirst_frame); }
    inline void SetParityBFF() { SetInterlaceMode(ASCbotfieldFirst_frame); }
    inline void SetProgressiveOp() { SetInterlaceMode(ASCprogressive_frame); }

    bool   Get_Last_frame_Data();
    mfxU16 Get_asc_subsampling_width();
    mfxU16 Get_asc_subsampling_height();
    mfxU32 Get_starting_frame_number();
    mfxU32 Get_frame_number();
    mfxU32 Get_frame_shot_Decision();
    mfxU32 Get_frame_last_in_scene();
    bool   Get_GoPcorrected_frame_shot_Decision();
    mfxI32 Get_frame_Spatial_complexity();
    mfxI32 Get_frame_Temporal_complexity();
    bool   Get_intra_frame_denoise_recommendation();
    mfxU32 Get_PDist_advice();
    bool   Get_LTR_advice();
    bool   Get_RepeatedFrame_advice();
    bool   Get_Filter_advice();
    mfxStatus get_LTR_op_hint(ASC_LTR_DEC& scd_LTR_hint);

    mfxStatus calc_RaCa_pic(mfxU8 *pSrc, mfxI32 width, mfxI32 height, mfxI32 pitch, mfxF64 &RsCs);

    bool Check_last_frame_processed(mfxU32 frameOrder);
    void Reset_last_frame_processed();

    static mfxI32 Get_CpuFeature_AVX2();
    static mfxI32 Get_CpuFeature_SSE41();

    virtual mfxStatus calc_RaCa_Surf(mfxHDLPair surface, mfxF64& rscs);

    mfxStatus RunFrame(mfxU8* frame, mfxU32 parity);
    mfxStatus PutFrameProgressive(mfxU8* frame, mfxI32 Pitch);

protected:
    ASCVidSample** m_videoData;
};
};

#endif //_ASC_H_