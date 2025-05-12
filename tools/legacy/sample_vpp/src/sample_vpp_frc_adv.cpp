/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "sample_vpp_frc_adv.h"
#include <cstdint>
#include <math.h>
#include <algorithm>
#include "vm/strings_defs.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

static const mfxU32 MFX_TIME_STAMP_FREQUENCY = 90000; // will go to mfxdefs.h

bool FRCAdvancedChecker::IsTimeStampsNear(mfxU64 timeStampRef, mfxU64 timeStampTst, mfxU64 eps) {
    mfxU32 absDiff = abs((mfxI32)(timeStampTst - (timeStampRef)));
    if (absDiff <= eps) {
        return true;
    }
    else {
        printf("\n\nError in FRC Advanced algorithm. \n");

        printf("Output frame number is %d\n", m_numOutputFrames - 1);

        int iPTS_Ref = (int)timeStampRef;
        int iPTS_Tst = (int)timeStampTst;
        int iAbsDiff = (int)absDiff;
        int iEps     = (int)eps;

        printf("Error: refTimeStamp, tstTimeStamp, Diff, Delta are: %d %d %u %d\n",
               iPTS_Ref,
               iPTS_Tst,
               iAbsDiff,
               iEps);

        return false;
    }

} // bool IsTimeStampsNear( mfxU64 timeStampTst, mfxU64 timeStampRef,  mfxU64 eps)

FRCAdvancedChecker::FRCAdvancedChecker()
        : m_minDeltaTime(0),
          m_bIsSetTimeOffset(false),
          m_timeOffset(0),
          m_expectedTimeStamp(0),
          m_timeStampJump(0),
          m_numOutputFrames(0),
          m_bReadyOutput(false),
          m_defferedInputTimeStamp(0),
          m_videoParam({ 0 }),
          m_ptsList() {} // FRCAdvancedChecker::FRCAdvancedChecker()

mfxStatus FRCAdvancedChecker::Init(mfxVideoParam* par, mfxU32 /*asyncDeep*/) {
    m_videoParam = *par;

    m_minDeltaTime = std::min(
        ((uint64_t)m_videoParam.vpp.In.FrameRateExtD * (uint64_t)MFX_TIME_STAMP_FREQUENCY) /
            (2 * (uint64_t)m_videoParam.vpp.In.FrameRateExtN),
        ((uint64_t)m_videoParam.vpp.Out.FrameRateExtD * (uint64_t)MFX_TIME_STAMP_FREQUENCY) /
            (2 * (uint64_t)m_videoParam.vpp.Out.FrameRateExtN));

    return MFX_ERR_NONE;

} // mfxStatus FRCAdvancedChecker::Init(mfxVideoParam *par, mfxU32 asyncDeep)

bool FRCAdvancedChecker::PutInputFrameAndCheck(mfxFrameSurface1* pSurface) {
    if (pSurface) {
        m_ptsList.push_back(pSurface->Data.TimeStamp);
    }

    return true;

} // bool FRCAdvancedChecker::PutInputFrameAndCheck(mfxFrameSurface1* pSurface)

bool FRCAdvancedChecker::PutOutputFrameAndCheck(mfxFrameSurface1* pSurface) {
    bool res;

    if (NULL == pSurface) {
        return false;
    }

    mfxU64 timeStampTst = pSurface->Data.TimeStamp;

    bool bRepeatAnalysis = false;
    do {
        //------------------------------------------------
        //           ReadyOutput
        //------------------------------------------------
        if (m_bReadyOutput) {
            m_expectedTimeStamp = GetExpectedPTS(m_numOutputFrames, m_timeOffset, m_timeStampJump);

            m_numOutputFrames++;

            res = IsTimeStampsNear(m_expectedTimeStamp, timeStampTst, m_minDeltaTime);

            return res;
        }
        else {
            //------------------------------------------------
            //           standard processing
            //------------------------------------------------
            if (0 == m_ptsList.size()) {
                if (m_numOutputFrames > 0) // last frame processing
                {
                    m_expectedTimeStamp =
                        GetExpectedPTS(m_numOutputFrames, m_timeOffset, m_timeStampJump);

                    m_numOutputFrames++;

                    res = IsTimeStampsNear(m_expectedTimeStamp, timeStampTst, m_minDeltaTime);

                    return res;
                }
                else {
                    return false; //?
                }
            }

            mfxU64 inputTimeStamp = m_ptsList.front();
            m_ptsList.pop_front();

            if (false == m_bIsSetTimeOffset) {
                m_bIsSetTimeOffset = true;
                m_timeOffset       = inputTimeStamp;
            }

            m_expectedTimeStamp = GetExpectedPTS(m_numOutputFrames, m_timeOffset, m_timeStampJump);

            mfxU32 timeStampDifference = abs((mfxI32)(inputTimeStamp - m_expectedTimeStamp));

            // process irregularity
            if (m_minDeltaTime > timeStampDifference) {
                inputTimeStamp = m_expectedTimeStamp;
            }

            if (inputTimeStamp < m_expectedTimeStamp) {
                m_bReadyOutput = false;

                // skip frame
                // request new one input surface
                //return MFX_ERR_MORE_DATA;
                bRepeatAnalysis = true;
            }
            else if (inputTimeStamp == m_expectedTimeStamp) // see above (minDelta)
            {
                m_bReadyOutput = false;

                m_numOutputFrames++;

                res = IsTimeStampsNear(m_expectedTimeStamp, timeStampTst, m_minDeltaTime);

                return res;
            }
            else // inputTimeStampParam > ptr->expectedTimeStamp
            {
                m_numOutputFrames++;

                res = IsTimeStampsNear(m_expectedTimeStamp, timeStampTst, m_minDeltaTime);

                return res;
            }
        }
    } while (bRepeatAnalysis);

    return false;

} // bool  FRCAdvancedChecker::PutOutputFrameAndCheck(mfxFrameSurface1* pSurface)

mfxU64 FRCAdvancedChecker::GetExpectedPTS(mfxU32 frameNumber, mfxU64 timeOffset, mfxU64 timeJump) {
    mfxU64 expectedPTS =
        (((mfxU64)frameNumber * m_videoParam.vpp.Out.FrameRateExtD * MFX_TIME_STAMP_FREQUENCY) /
             m_videoParam.vpp.Out.FrameRateExtN +
         timeOffset + timeJump);

    return expectedPTS;

} // mfxU64  FRCAdvancedChecker::GetExpectedPTS( mfxU32 frameNumber, mfxU64 timeOffset, mfxU64 timeJump )

/* EOF */
