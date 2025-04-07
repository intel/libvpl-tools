/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    #if defined _DEBUG
        #define DISPATCHER_DLL_NAME "libvpld.dll"
    #else
        #define DISPATCHER_DLL_NAME "libvpl.dll"
    #endif

#endif

#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <string>

#include "vpl/mfx.h"
#include "vpl/mfxcamera.h"

#define DECODE_FOURCC(ch) ch & 0xff, ch >> 8 & 0xff, ch >> 16 & 0xff, ch >> 24 & 0xff

#define DECODE_FOURCC_2(ch, s) \
    s[0] = ch & 0xff;          \
    s[1] = ch >> 8 & 0xff;     \
    s[2] = ch >> 16 & 0xff;    \
    s[3] = ch >> 24 & 0xff;

#define STRING_OPTION(x) \
    case x:              \
        return #x

const char *_print_fourcc(int ch) {
    static char str[5];
    if (0 == ch) {
        str[0] = 'U';
        str[1] = 'N';
        str[2] = 'K';
        str[3] = 'N';
        str[4] = '\0';
    }
    else if (41 == ch) {
        str[0] = 'P';
        str[1] = '8';
        str[2] = '\0';
    }
    else {
        DECODE_FOURCC_2(ch, str);
        str[4] = '\0';
    }
    return str;
}

const char *_print_Impl(mfxIMPL impl) {
    switch (impl) {
        STRING_OPTION(MFX_IMPL_TYPE_SOFTWARE);
        STRING_OPTION(MFX_IMPL_TYPE_HARDWARE);

        default:
            break;
    }

    return "<unknown implementation>";
}

const char *_print_AccelMode(mfxAccelerationMode mode) {
    switch (mode) {
        STRING_OPTION(MFX_ACCEL_MODE_NA);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D9);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_GLX);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_X11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_HDDLUNITE);

        default:
            break;
    }

    return "<unknown acceleration mode>";
}

const char *_print_PoolPolicy(mfxPoolAllocationPolicy policy) {
    switch (policy) {
        STRING_OPTION(MFX_ALLOCATION_OPTIMAL);
        STRING_OPTION(MFX_ALLOCATION_UNLIMITED);
        STRING_OPTION(MFX_ALLOCATION_LIMITED);

        default:
            break;
    }

    return "<unknown pool allocation policy>";
}

const char *_print_MediaAdapterType(mfxMediaAdapterType type) {
    switch (type) {
        STRING_OPTION(MFX_MEDIA_UNKNOWN);
        STRING_OPTION(MFX_MEDIA_INTEGRATED);
        STRING_OPTION(MFX_MEDIA_DISCRETE);

        default:
            break;
    }

    return "<unknown media adapter type>";
}

#ifdef ONEVPL_EXPERIMENTAL
const char *_print_SurfaceType(mfxSurfaceType type) {
    switch (type) {
        STRING_OPTION(MFX_SURFACE_TYPE_UNKNOWN);
        STRING_OPTION(MFX_SURFACE_TYPE_D3D11_TEX2D);
        STRING_OPTION(MFX_SURFACE_TYPE_D3D12_TEX2D);
        STRING_OPTION(MFX_SURFACE_TYPE_VAAPI);
        STRING_OPTION(MFX_SURFACE_TYPE_OPENCL_IMG2D);
        STRING_OPTION(MFX_SURFACE_TYPE_VULKAN_IMG2D);

        default:
            break;
    }

    return "<unknown surface type>";
}

const char *_print_SurfaceComponent(mfxSurfaceComponent type) {
    switch (type) {
        STRING_OPTION(MFX_SURFACE_COMPONENT_UNKNOWN);
        STRING_OPTION(MFX_SURFACE_COMPONENT_ENCODE);
        STRING_OPTION(MFX_SURFACE_COMPONENT_DECODE);
        STRING_OPTION(MFX_SURFACE_COMPONENT_VPP_INPUT);
        STRING_OPTION(MFX_SURFACE_COMPONENT_VPP_OUTPUT);

        default:
            break;
    }

    return "<unknown surface component>";
}

const char *_print_SurfaceFlags(mfxU32 type) {
    switch (type) {
        STRING_OPTION(MFX_SURFACE_FLAG_IMPORT_SHARED);
        STRING_OPTION(MFX_SURFACE_FLAG_IMPORT_COPY);
        STRING_OPTION(MFX_SURFACE_FLAG_EXPORT_SHARED);
        STRING_OPTION(MFX_SURFACE_FLAG_EXPORT_COPY);

        default:
            break;
    }

    return "<unknown surface flag type>";
}
#endif

const char *_print_ResourceType(mfxResourceType type) {
    switch (type) {
        STRING_OPTION(MFX_RESOURCE_SYSTEM_SURFACE);
        STRING_OPTION(MFX_RESOURCE_VA_SURFACE_PTR);
        STRING_OPTION(MFX_RESOURCE_VA_BUFFER_PTR);
        STRING_OPTION(MFX_RESOURCE_DX9_SURFACE);
        STRING_OPTION(MFX_RESOURCE_DX11_TEXTURE);
        STRING_OPTION(MFX_RESOURCE_DX12_RESOURCE);
        STRING_OPTION(MFX_RESOURCE_DMA_RESOURCE);
        STRING_OPTION(MFX_RESOURCE_HDDLUNITE_REMOTE_MEMORY);

        default:
            break;
    }

    return "<unknown resource type>";
}

const char *_print_CodecID(mfxU32 codecID) {
    switch (codecID) {
        STRING_OPTION(MFX_CODEC_AV1);
        STRING_OPTION(MFX_CODEC_AVC);
        STRING_OPTION(MFX_CODEC_JPEG);
        STRING_OPTION(MFX_CODEC_HEVC);
        STRING_OPTION(MFX_CODEC_MPEG2);
        STRING_OPTION(MFX_CODEC_VC1);
        STRING_OPTION(MFX_CODEC_VP8);
        STRING_OPTION(MFX_CODEC_VP9);
        default:
            break;
    }

    // unknown format - print fourCC
    return _print_fourcc(codecID);
}

const char *_print_ExtbufID(mfxU32 extbufID) {
    switch (extbufID) {
        STRING_OPTION(MFX_EXTBUFF_ALLOCATION_HINTS);
        STRING_OPTION(MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM);
        STRING_OPTION(MFX_EXTBUFF_AV1_SEGMENTATION);
        STRING_OPTION(MFX_EXTBUFF_AVC_REFLIST_CTRL);
        STRING_OPTION(MFX_EXTBUFF_AVC_REFLISTS);
        STRING_OPTION(MFX_EXTBUFF_AVC_ROUNDING_OFFSET);
        STRING_OPTION(MFX_EXTBUFF_AVC_TEMPORAL_LAYERS);
        STRING_OPTION(MFX_EXTBUFF_BRC);
        STRING_OPTION(MFX_EXTBUF_CAM_3DLUT);
        STRING_OPTION(MFX_EXTBUF_CAM_BAYER_DENOISE);
        STRING_OPTION(MFX_EXTBUF_CAM_BLACK_LEVEL_CORRECTION);
        STRING_OPTION(MFX_EXTBUF_CAM_COLOR_CORRECTION_3X3);
        STRING_OPTION(MFX_EXTBUF_CAM_CSC_YUV_RGB);
        STRING_OPTION(MFX_EXTBUF_CAM_FORWARD_GAMMA_CORRECTION);
        STRING_OPTION(MFX_EXTBUF_CAM_HOT_PIXEL_REMOVAL);
        STRING_OPTION(MFX_EXTBUF_CAM_LENS_GEOM_DIST_CORRECTION);
        STRING_OPTION(MFX_EXTBUF_CAM_PADDING);
        STRING_OPTION(MFX_EXTBUF_CAM_PIPECONTROL);
        STRING_OPTION(MFX_EXTBUF_CAM_TOTAL_COLOR_CONTROL);
        STRING_OPTION(MFX_EXTBUF_CAM_VIGNETTE_CORRECTION);
        STRING_OPTION(MFX_EXTBUF_CAM_WHITE_BALANCE);
        STRING_OPTION(MFX_EXTBUFF_CENC_PARAM);
        STRING_OPTION(MFX_EXTBUFF_CHROMA_LOC_INFO);
        STRING_OPTION(MFX_EXTBUFF_CODING_OPTION);
        STRING_OPTION(MFX_EXTBUFF_CODING_OPTION_SPSPPS);
        STRING_OPTION(MFX_EXTBUFF_CODING_OPTION_VPS);
        STRING_OPTION(MFX_EXTBUFF_CODING_OPTION2);
        STRING_OPTION(MFX_EXTBUFF_CODING_OPTION3);
        STRING_OPTION(MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO);
        STRING_OPTION(MFX_EXTBUFF_CROPS);
        STRING_OPTION(MFX_EXTBUFF_DEC_VIDEO_PROCESSING);
        STRING_OPTION(MFX_EXTBUFF_DECODE_ERROR_REPORT);
        STRING_OPTION(MFX_EXTBUFF_DECODED_FRAME_INFO);
        STRING_OPTION(MFX_EXTBUFF_DEVICE_AFFINITY_MASK);
        STRING_OPTION(MFX_EXTBUFF_DIRTY_RECTANGLES);
        STRING_OPTION(MFX_EXTBUFF_ENCODED_FRAME_INFO);
        STRING_OPTION(MFX_EXTBUFF_ENCODED_SLICES_INFO);
        STRING_OPTION(MFX_EXTBUFF_ENCODED_UNITS_INFO);
        STRING_OPTION(MFX_EXTBUFF_ENCODER_CAPABILITY);
        STRING_OPTION(MFX_EXTBUFF_ENCODER_IPCM_AREA);
        STRING_OPTION(MFX_EXTBUFF_ENCODER_RESET_OPTION);
        STRING_OPTION(MFX_EXTBUFF_ENCODER_ROI);
        STRING_OPTION(MFX_EXTBUFF_HEVC_PARAM);
        STRING_OPTION(MFX_EXTBUFF_HEVC_REGION);
        STRING_OPTION(MFX_EXTBUFF_HEVC_TILES);
        STRING_OPTION(MFX_EXTBUFF_INSERT_HEADERS);
        STRING_OPTION(MFX_EXTBUFF_JPEG_HUFFMAN);
        STRING_OPTION(MFX_EXTBUFF_JPEG_QT);
        STRING_OPTION(MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME);
        STRING_OPTION(MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN);
        STRING_OPTION(MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT);
        STRING_OPTION(MFX_EXTBUFF_MB_DISABLE_SKIP_MAP);
        STRING_OPTION(MFX_EXTBUFF_MB_FORCE_INTRA);
        STRING_OPTION(MFX_EXTBUFF_MBQP);
        STRING_OPTION(MFX_EXTBUFF_MOVING_RECTANGLES);
        STRING_OPTION(MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES);
        STRING_OPTION(MFX_EXTBUFF_MVC_SEQ_DESC);
        STRING_OPTION(MFX_EXTBUFF_MVC_TARGET_VIEWS);
        STRING_OPTION(MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM);
        STRING_OPTION(MFX_EXTBUFF_PICTURE_TIMING_SEI);
        STRING_OPTION(MFX_EXTBUFF_PRED_WEIGHT_TABLE);
        STRING_OPTION(MFX_EXTBUFF_THREADS_PARAM);
        STRING_OPTION(MFX_EXTBUFF_TIME_CODE);
        STRING_OPTION(MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS);
        STRING_OPTION(MFX_EXTBUFF_VIDEO_SIGNAL_INFO);
        STRING_OPTION(MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN);
        STRING_OPTION(MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT);
        STRING_OPTION(MFX_EXTBUFF_VP8_CODING_OPTION);
        STRING_OPTION(MFX_EXTBUFF_VP9_PARAM);
        STRING_OPTION(MFX_EXTBUFF_VP9_SEGMENTATION);
        STRING_OPTION(MFX_EXTBUFF_VP9_TEMPORAL_LAYERS);
        STRING_OPTION(MFX_EXTBUFF_VPP_3DLUT);
        STRING_OPTION(MFX_EXTBUFF_VPP_AUXDATA);
        STRING_OPTION(MFX_EXTBUFF_VPP_COLOR_CONVERSION);
        STRING_OPTION(MFX_EXTBUFF_VPP_COLORFILL);
        STRING_OPTION(MFX_EXTBUFF_VPP_COMPOSITE);
        STRING_OPTION(MFX_EXTBUFF_VPP_DEINTERLACING);
        STRING_OPTION(MFX_EXTBUFF_VPP_DENOISE2);
        STRING_OPTION(MFX_EXTBUFF_VPP_DETAIL);
        STRING_OPTION(MFX_EXTBUFF_VPP_DONOTUSE);
        STRING_OPTION(MFX_EXTBUFF_VPP_DOUSE);
        STRING_OPTION(MFX_EXTBUFF_VPP_FIELD_PROCESSING);
        STRING_OPTION(MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION);
        STRING_OPTION(MFX_EXTBUFF_VPP_IMAGE_STABILIZATION);
        STRING_OPTION(MFX_EXTBUFF_VPP_MCTF);
        STRING_OPTION(MFX_EXTBUFF_VPP_MIRRORING);
        STRING_OPTION(MFX_EXTBUFF_VPP_PROCAMP);
        STRING_OPTION(MFX_EXTBUFF_VPP_ROTATION);
        STRING_OPTION(MFX_EXTBUFF_VPP_SCALING);
        STRING_OPTION(MFX_EXTBUFF_VPP_SCENE_ANALYSIS);
        STRING_OPTION(MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO);
#ifdef ONEVPL_EXPERIMENTAL
        STRING_OPTION(MFX_EXTBUFF_ENCODESTATS);
        STRING_OPTION(MFX_EXTBUFF_TUNE_ENCODE_QUALITY);
        STRING_OPTION(MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER);
#endif
        default:
            break;
    }

    // unknown format - print fourCC
    return _print_fourcc(extbufID);
}

const char *_print_ColorFormat(mfxU32 colorFormat) {
    switch (colorFormat) {
        STRING_OPTION(MFX_FOURCC_A2RGB10);
        STRING_OPTION(MFX_FOURCC_ABGR16);
        STRING_OPTION(MFX_FOURCC_ABGR16F);
        STRING_OPTION(MFX_FOURCC_ARGB16);
        STRING_OPTION(MFX_FOURCC_AYUV);
        STRING_OPTION(MFX_FOURCC_AYUV_RGB4);
        STRING_OPTION(MFX_FOURCC_BGR4);
        STRING_OPTION(MFX_FOURCC_BGRP);
        STRING_OPTION(MFX_FOURCC_I010);
        STRING_OPTION(MFX_FOURCC_I210);
        STRING_OPTION(MFX_FOURCC_I422);
        STRING_OPTION(MFX_FOURCC_IYUV);
        STRING_OPTION(MFX_FOURCC_NV12);
        STRING_OPTION(MFX_FOURCC_NV16);
        STRING_OPTION(MFX_FOURCC_NV21);
        STRING_OPTION(MFX_FOURCC_P010);
        STRING_OPTION(MFX_FOURCC_P016);
        STRING_OPTION(MFX_FOURCC_P210);
        STRING_OPTION(MFX_FOURCC_P8);
        STRING_OPTION(MFX_FOURCC_P8_TEXTURE);
        STRING_OPTION(MFX_FOURCC_R16);
        STRING_OPTION(MFX_FOURCC_RGB4);
        STRING_OPTION(MFX_FOURCC_RGB565);
        STRING_OPTION(MFX_FOURCC_RGBP);
        STRING_OPTION(MFX_FOURCC_UYVY);
        STRING_OPTION(MFX_FOURCC_XYUV);
        STRING_OPTION(MFX_FOURCC_Y210);
        STRING_OPTION(MFX_FOURCC_Y216);
        STRING_OPTION(MFX_FOURCC_Y410);
        STRING_OPTION(MFX_FOURCC_Y416);
        STRING_OPTION(MFX_FOURCC_YUY2);
        STRING_OPTION(MFX_FOURCC_YV12);
        default:
            break;
    }

    // unknown format - print fourCC
    return _print_fourcc(colorFormat);
}

const char *_print_ProfileType(mfxU32 fourcc, mfxU32 type) {
    switch (fourcc) {
        case MFX_CODEC_JPEG: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_JPEG_BASELINE);

                default:
                    return "<unknown MFX_CODEC_JPEG profile>";
            }
        }

        case MFX_CODEC_AVC: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);

                STRING_OPTION(MFX_PROFILE_AVC_BASELINE);
                STRING_OPTION(MFX_PROFILE_AVC_MAIN);
                STRING_OPTION(MFX_PROFILE_AVC_EXTENDED);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH10);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH_422);
                STRING_OPTION(MFX_PROFILE_AVC_CONSTRAINED_BASELINE);
                STRING_OPTION(MFX_PROFILE_AVC_CONSTRAINED_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_PROGRESSIVE_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_MULTIVIEW_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_STEREO_HIGH);

                default:
                    return "<unknown MFX_CODEC_AVC profile>";
            }
        }

        case MFX_CODEC_HEVC: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_HEVC_MAIN);
                STRING_OPTION(MFX_PROFILE_HEVC_MAIN10);
                STRING_OPTION(MFX_PROFILE_HEVC_MAINSP);
                STRING_OPTION(MFX_PROFILE_HEVC_REXT);
                STRING_OPTION(MFX_PROFILE_HEVC_SCC);

                default:
                    return "<unknown MFX_CODEC_HEVC profile>";
            }
        }

        case MFX_CODEC_MPEG2: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_MPEG2_SIMPLE);
                STRING_OPTION(MFX_PROFILE_MPEG2_MAIN);
                STRING_OPTION(MFX_PROFILE_MPEG2_HIGH);
                STRING_OPTION(MFX_LEVEL_MPEG2_HIGH);
                STRING_OPTION(MFX_LEVEL_MPEG2_HIGH1440);

                default:
                    return "<unknown MFX_CODEC_MPEG2 profile>";
            }
        }

        case MFX_CODEC_VP8: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VP8_0);
                STRING_OPTION(MFX_PROFILE_VP8_1);
                STRING_OPTION(MFX_PROFILE_VP8_2);
                STRING_OPTION(MFX_PROFILE_VP8_3);

                default:
                    return "<unknown MFX_CODEC_VP9 profile>";
            }
        }

        case MFX_CODEC_VC1: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VC1_SIMPLE);
                STRING_OPTION(MFX_PROFILE_VC1_MAIN);
                STRING_OPTION(MFX_PROFILE_VC1_ADVANCED);

                default:
                    return "<unknown MFX_CODEC_VC1 profile>";
            }
        }

        case MFX_CODEC_VP9: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VP9_0);
                STRING_OPTION(MFX_PROFILE_VP9_1);
                STRING_OPTION(MFX_PROFILE_VP9_2);
                STRING_OPTION(MFX_PROFILE_VP9_3);

                default:
                    return "<unknown MFX_CODEC_VP9 profile>";
            }
        }

        case MFX_CODEC_AV1: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_AV1_MAIN);
                STRING_OPTION(MFX_PROFILE_AV1_HIGH);
                STRING_OPTION(MFX_PROFILE_AV1_PRO);

                default:
                    return "<unknown MFX_CODEC_AV1 profile>";
            }
        }
    }

    return "<unknown codec format>";
}

// clang-format off

#ifdef ONEVPL_EXPERIMENTAL
typedef struct {
    mfxU32 propData;
    mfxU8 *propStr;
} ConfigPropInfo;

static const std::map<std::string, ConfigPropInfo> ConfigPropMap = {
    { "basic:all", { 0, (mfxU8 *)"mfxImplDescription" } },  // no codec/vpp info, just query minimal (basic) info from mfxImplDescription

    { "dec:all",   { 0, (mfxU8 *)"mfxImplDescription.mfxDecoderDescription" } },
    { "enc:all",   { 0, (mfxU8 *)"mfxImplDescription.mfxEncoderDescription" } },
    { "vpp:all",   { 0, (mfxU8 *)"mfxImplDescription.mfxVPPDescription"     } },

    { "dec:av1",   { MFX_CODEC_AV1,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:avc",   { MFX_CODEC_AVC,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:hevc",  { MFX_CODEC_HEVC,  (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:mpeg2", { MFX_CODEC_MPEG2, (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:vc1",   { MFX_CODEC_VC1,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:vp8",   { MFX_CODEC_VP8,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:vp9",   { MFX_CODEC_VP9,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },
    { "dec:vvc",   { MFX_CODEC_VVC,   (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID" } },

    { "enc:av1",   { MFX_CODEC_AV1,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:avc",   { MFX_CODEC_AVC,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:hevc",  { MFX_CODEC_HEVC,  (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:mpeg2", { MFX_CODEC_MPEG2, (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:vc1",   { MFX_CODEC_VC1,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:vp8",   { MFX_CODEC_VP8,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:vp9",   { MFX_CODEC_VP9,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },
    { "enc:vvc",   { MFX_CODEC_VVC,   (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID" } },

    { "vpp:3dlut", { MFX_EXTBUFF_VPP_3DLUT,                  (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:aifrc", { MFX_EXTBUFF_VPP_AI_FRAME_INTERPOLATION, (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:aisr",  { MFX_EXTBUFF_VPP_AI_SUPER_RESOLUTION,    (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:auxd",  { MFX_EXTBUFF_VPP_AUXDATA,                (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:csc",   { MFX_EXTBUFF_VPP_COLOR_CONVERSION,       (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:cfill", { MFX_EXTBUFF_VPP_COLORFILL,              (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:comp",  { MFX_EXTBUFF_VPP_COMPOSITE,              (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:deint", { MFX_EXTBUFF_VPP_DEINTERLACING,          (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:dns2",  { MFX_EXTBUFF_VPP_DENOISE2,               (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:det",   { MFX_EXTBUFF_VPP_DETAIL,                 (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:douse", { MFX_EXTBUFF_VPP_DOUSE,                  (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:fproc", { MFX_EXTBUFF_VPP_FIELD_PROCESSING,       (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:frc",   { MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION,  (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:imgst", { MFX_EXTBUFF_VPP_IMAGE_STABILIZATION,    (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:mctf",  { MFX_EXTBUFF_VPP_MCTF,                   (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:mirr",  { MFX_EXTBUFF_VPP_MIRRORING,              (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:perc",  { MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER,     (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:proc",  { MFX_EXTBUFF_VPP_PROCAMP,                (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:rot",   { MFX_EXTBUFF_VPP_ROTATION,               (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:scale", { MFX_EXTBUFF_VPP_SCALING,                (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:scnan", { MFX_EXTBUFF_VPP_SCENE_ANALYSIS,         (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
    { "vpp:vsig",  { MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO,      (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC" } },
};
#endif

static void Usage(void) {
    printf("\nUsage: vpl-inspect [options]\n");
    printf("\nIf no options are specified, print default capabilities report (MFX_IMPLCAPS_IMPLDESCSTRUCTURE)\n");
    printf("\nOptions:\n");
    printf("   -?, -help ...... print help message\n");
    printf("   -b ............. print brief output (do not print decoder, encoder, and VPP capabilities)\n");
    printf("   -ex ............ print extended device ID info (MFX_IMPLCAPS_DEVICE_ID_EXTENDED)\n");
    printf("   -f ............. print list of implemented functions (MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS)\n");
    printf("   -d3d9 .......... only enumerate implementations supporting D3D9\n");
#ifdef ONEVPL_EXPERIMENTAL
    printf("   -props ......... list of props as KV pairs, separated with commas (ex: -props dec:all,enc:av1)\n");
    printf("                    use '-props list' to print list of available properties\n");
#endif
#if defined(_WIN32) || defined(_WIN64)
    printf("   -disp .......... print path to loaded dispatcher library\n");
#endif
}
// clang-format on

int main(int argc, char *argv[]) {
    mfxLoader loader = MFXLoad();
    if (loader == NULL) {
        printf("Error - MFXLoad() returned null - no libraries found\n");
        return -1;
    }

    bool bPrintImplementedFunctions = false;
    bool bFullInfo                  = true;
    bool bRequireD3D9               = false;
    bool bPrintExtendedDeviceID     = false;
    bool bPrintDispInfo             = false;
    bool bPropsQuery                = false;
#ifdef ONEVPL_EXPERIMENTAL
    bool bPrintSurfaceTypes = true;
    std::list<std::string> propStrList;
#endif

    for (int argIdx = 1; argIdx < argc; argIdx++) {
        std::string nextArg(argv[argIdx]);

        if (nextArg == "-f") {
            bPrintImplementedFunctions = true;
        }
        else if (nextArg == "-ex") {
            bPrintExtendedDeviceID = true;
        }
        else if (nextArg == "-b") {
            bFullInfo = false;
        }
        else if (nextArg == "-d3d9") {
            bRequireD3D9 = true;
        }
#ifdef ONEVPL_EXPERIMENTAL
        else if (nextArg == "-props") {
            bPropsQuery        = true;
            bPrintSurfaceTypes = false;
            std::string propStr;
            if (++argIdx < argc) {
                propStr = argv[argIdx];

                std::string s;
                std::stringstream propSS(propStr);
                while (getline(propSS, s, ',')) {
                    propStrList.push_back(s);
                }
            }
            else {
                printf("Error - must specify which props\n");
                return -1;
            }

            if (propStrList.front() == "list") {
                printf("Available props arguments:\n");
                for (auto const &it : ConfigPropMap)
                    printf("  %s\n", it.first.c_str());
                return 0;
            }
        }
#endif
        else if (nextArg == "-?" || nextArg == "-help") {
            Usage();
            return -1;
        }
#if defined(_WIN32) || defined(_WIN64)
        else if (nextArg == "-disp") {
            bPrintDispInfo = true;
        }
#endif
        else {
            printf("Error - unknown option %s\n", nextArg.c_str());
            Usage();
            return -1;
        }
    }

    if (bPrintDispInfo) {
#if defined(_WIN32) || defined(_WIN64)
        HMODULE handle = GetModuleHandleA(DISPATCHER_DLL_NAME);
        if (!handle) {
            printf("Error - Failed to get dispatcher handle\n");
            return -1;
        }

        char dispatcherPath[1024] = {};
        DWORD pathLen = GetModuleFileNameA(handle, dispatcherPath, sizeof(dispatcherPath));
        if (pathLen == 0 || pathLen >= sizeof(dispatcherPath)) {
            printf("Error - Failed to get dispatcher path\n");
            return -1;
        }

        // print full path to dispatcher DLL loaded by vpl-inspect, then exit
        // this is primarily for scripting and debugging
        printf("%s\n", dispatcherPath);
        return 0;
#endif
    }

    if (bRequireD3D9) {
        printf("Warning - Enumerating D3D9 implementations ONLY\n");
        mfxConfig cfg = MFXCreateConfig(loader);
        if (!cfg) {
            printf("Error - MFXCreateConfig() returned null\n");
            return -1;
        }

        mfxVariant var      = {};
        var.Version.Version = MFX_VARIANT_VERSION;
        var.Type            = MFX_VARIANT_TYPE_U32;
        var.Data.U32        = MFX_ACCEL_MODE_VIA_D3D9;

        mfxStatus sts =
            MFXSetConfigFilterProperty(cfg,
                                       (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                       var);
        if (sts) {
            printf("Error - MFXSetConfigFilterProperty() returned %d\n", sts);
            return -1;
        }
    }

#ifdef ONEVPL_EXPERIMENTAL
    if (bPropsQuery) {
        mfxStatus sts;
        mfxVariant var      = {};
        var.Version.Version = MFX_VARIANT_VERSION;

        for (const std::string &propStr : propStrList) {
            mfxConfig cfg = MFXCreateConfig(
                loader); // create new config for every property, for e.g. multiple codecs

            auto it = ConfigPropMap.find(propStr);
            if (it == ConfigPropMap.end()) {
                printf("Error - invalid property string %s\n", propStr.c_str());
                printf("run 'vplinspect -props list' for list of supported properties\n");
                return -1;
            }

            // add property with MFX_VARIANT_TYPE_QUERY
            var.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
            var.Data.U32 = it->second.propData;
            sts          = MFXSetConfigFilterProperty(cfg, it->second.propStr, var);

            if (sts) {
                printf("Error - MFXSetConfigFilterProperty() returned %d\n", sts);
                return -1;
            }
        }
    }
#endif

    int i = 0;
    mfxImplDescription *idesc;
    while (MFX_ERR_NONE == MFXEnumImplementations(loader,
                                                  i,
                                                  MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                                  reinterpret_cast<mfxHDL *>(&idesc))) {
        printf("\nImplementation #%d: %s\n", i, idesc->ImplName);

        // get path if supported (available starting with API 2.4)
        mfxHDL hImplPath = nullptr;
        if (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLPATH, &hImplPath)) {
            if (hImplPath) {
                printf("%2sLibrary path: %s\n", "", reinterpret_cast<mfxChar *>(hImplPath));
                MFXDispReleaseImplDescription(loader, hImplPath);
            }
        }

        printf("%2sAccelerationMode: %s\n", "", _print_AccelMode(idesc->AccelerationMode));
        printf("%2sApiVersion: %hu.%hu\n", "", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
        printf("%2sImpl: %s\n", "", _print_Impl(idesc->Impl));
        printf("%2sVendorImplID: 0x%04X\n", "", idesc->VendorImplID);
        printf("%2sImplName: %s\n", "", idesc->ImplName);
        printf("%2sLicense: %s\n", "", idesc->License);
        printf("%2sVersion: %hu.%hu\n", "", idesc->Version.Major, idesc->Version.Minor);
        printf("%2sKeywords: %s\n", "", idesc->Keywords);
        printf("%2sVendorID: 0x%04X\n", "", idesc->VendorID);

        /* mfxAccelerationModeDescription */
        mfxAccelerationModeDescription *accel = &idesc->AccelerationModeDescription;
        printf("%2smfxAccelerationModeDescription:\n", "");
        printf("%4sVersion: %hu.%hu\n", "", accel->Version.Major, accel->Version.Minor);
        for (int mode = 0; mode < accel->NumAccelerationModes; mode++) {
            printf("%4sMode: %s\n", "", _print_AccelMode(accel->Mode[mode]));
        }

        /* mfxPoolPolicyDescription */
        if (idesc->Version.Version >= MFX_STRUCT_VERSION(1, 2)) {
            mfxPoolPolicyDescription *poolPolicies = &idesc->PoolPolicies;
            printf("%2smfxPoolPolicyDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n",
                   "",
                   poolPolicies->Version.Major,
                   poolPolicies->Version.Minor);
            for (int policy = 0; policy < poolPolicies->NumPoolPolicies; policy++) {
                printf("%4sPolicy: %s\n", "", _print_PoolPolicy(poolPolicies->Policy[policy]));
            }
        }

        /* mfxDeviceDescription */
        mfxDeviceDescription *dev = &idesc->Dev;
        printf("%2smfxDeviceDescription:\n", "");
        if (dev->Version.Version >= MFX_STRUCT_VERSION(1, 1)) {
            printf("%4sMediaAdapterType: %s\n",
                   "",
                   _print_MediaAdapterType((mfxMediaAdapterType)dev->MediaAdapterType));
        }
        printf("%4sDeviceID: %s\n", "", dev->DeviceID);
        printf("%4sVersion: %hu.%hu\n", "", dev->Version.Major, dev->Version.Minor);
        for (int subdevice = 0; subdevice < dev->NumSubDevices; subdevice++) {
            printf("%4sIndex: %u\n", "", dev->SubDevices[subdevice].Index);
            printf("%4sSubDeviceID: %s\n", "", dev->SubDevices[subdevice].SubDeviceID);
        }

        if (bFullInfo || bPropsQuery) {
            /* mfxDecoderDescription */
            mfxDecoderDescription *dec = &idesc->Dec;
            printf("%2smfxDecoderDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", dec->Version.Major, dec->Version.Minor);
            for (int codec = 0; codec < dec->NumCodecs; codec++) {
                printf("%4sCodecID: %s\n", "", _print_CodecID(dec->Codecs[codec].CodecID));
                printf("%4sMaxcodecLevel: %hu\n", "", dec->Codecs[codec].MaxcodecLevel);
                for (int profile = 0; profile < dec->Codecs[codec].NumProfiles; profile++) {
                    printf("%6sProfile: %s\n",
                           "",
                           _print_ProfileType(dec->Codecs[codec].CodecID,
                                              dec->Codecs[codec].Profiles[profile].Profile));
                    for (int memtype = 0;
                         memtype < dec->Codecs[codec].Profiles[profile].NumMemTypes;
                         memtype++) {
                        printf("%8sMemHandleType: %s\n",
                               "",
                               _print_ResourceType(dec->Codecs[codec]
                                                       .Profiles[profile]
                                                       .MemDesc[memtype]
                                                       .MemHandleType));
                        printf("%10sWidth Min: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                        printf("%10sWidth Max: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                        printf("%10sWidth Step: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                        printf("%10sHeight Min: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                        printf("%10sHeight Max: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                        printf("%10sHeight Step: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                        printf("%10sColorFormats: ", "");
                        for (int colorformat = 0;
                             colorformat <
                             dec->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                             colorformat++) {
                            if (0 != colorformat)
                                printf(", ");
                            printf("%s",
                                   _print_ColorFormat(dec->Codecs[codec]
                                                          .Profiles[profile]
                                                          .MemDesc[memtype]
                                                          .ColorFormats[colorformat]));
                        }
                        printf("\n");
                    }
                }
            }

            /* mfxEncoderDescription */
            mfxEncoderDescription *enc = &idesc->Enc;
            printf("%2smfxEncoderDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", enc->Version.Major, enc->Version.Minor);
            for (int codec = 0; codec < enc->NumCodecs; codec++) {
                printf("%4sCodecID: %s\n", "", _print_CodecID(enc->Codecs[codec].CodecID));
                printf("%4sMaxcodecLevel: %hu\n", "", enc->Codecs[codec].MaxcodecLevel);
                printf("%4sBiDirectionalPrediction: %hu\n",
                       "",
                       enc->Codecs[codec].BiDirectionalPrediction);

                for (int profile = 0; profile < enc->Codecs[codec].NumProfiles; profile++) {
                    printf("%6sProfile: %s\n",
                           "",
                           _print_ProfileType(enc->Codecs[codec].CodecID,
                                              enc->Codecs[codec].Profiles[profile].Profile));
                    for (int memtype = 0;
                         memtype < enc->Codecs[codec].Profiles[profile].NumMemTypes;
                         memtype++) {
                        printf("%8sMemHandleType: %s\n",
                               "",
                               _print_ResourceType(enc->Codecs[codec]
                                                       .Profiles[profile]
                                                       .MemDesc[memtype]
                                                       .MemHandleType));
                        printf("%10sWidth Min: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                        printf("%10sWidth Max: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                        printf("%10sWidth Step: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                        printf("%10sHeight Min: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                        printf("%10sHeight Max: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                        printf("%10sHeight Step: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                        printf("%10sColorFormats: ", "");
                        for (int colorformat = 0;
                             colorformat <
                             enc->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                             colorformat++) {
                            if (0 != colorformat)
                                printf(", ");
                            printf("%s",
                                   _print_ColorFormat(enc->Codecs[codec]
                                                          .Profiles[profile]
                                                          .MemDesc[memtype]
                                                          .ColorFormats[colorformat]));
                        }
                        printf("\n");
                    }
                }
            }

            /* mfxVPPDescription */
            mfxVPPDescription *vpp = &idesc->VPP;
            printf("%2smfxVPPDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", vpp->Version.Major, vpp->Version.Minor);
            for (int filter = 0; filter < vpp->NumFilters; filter++) {
                printf("%4sFilterFourCC: %s\n",
                       "",
                       _print_ExtbufID(vpp->Filters[filter].FilterFourCC));
                printf("%4sMaxDelayInFrames: %hu\n", "", vpp->Filters[filter].MaxDelayInFrames);
                for (int memtype = 0; memtype < vpp->Filters[filter].NumMemTypes; memtype++) {
                    printf(
                        "%6sMemHandleType: %s\n",
                        "",
                        _print_ResourceType(vpp->Filters[filter].MemDesc[memtype].MemHandleType));
                    printf("%6sWidth Min: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Min);
                    printf("%6sWidth Max: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Max);
                    printf("%6sWidth Step: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Step);
                    printf("%6sHeight Min: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Min);
                    printf("%6sHeight Max: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Max);
                    printf("%6sHeight Step: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Step);
                    for (int informat = 0;
                         informat < vpp->Filters[filter].MemDesc[memtype].NumInFormats;
                         informat++) {
                        printf(
                            "%8sInFormat: %s\n",
                            "",
                            _print_ColorFormat(
                                vpp->Filters[filter].MemDesc[memtype].Formats[informat].InFormat));
                        printf("%10sOutFormats: ", "");
                        for (int outformat = 0;
                             outformat <
                             vpp->Filters[filter].MemDesc[memtype].Formats[informat].NumOutFormat;
                             outformat++) {
                            if (0 != outformat)
                                printf(", ");
                            printf("%s",
                                   _print_ColorFormat(vpp->Filters[filter]
                                                          .MemDesc[memtype]
                                                          .Formats[informat]
                                                          .OutFormats[outformat]));
                        }
                        printf("\n");
                    }
                }
            }

            printf("%2sNumExtParam: %d\n", "", idesc->NumExtParam);
        }

        MFXDispReleaseImplDescription(loader, idesc);

        if (bPrintImplementedFunctions) {
            mfxImplementedFunctions *fdesc;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS,
                                                   reinterpret_cast<mfxHDL *>(&fdesc));

            if (sts == MFX_ERR_NONE) {
                // print out list of functions' name
                printf("%2sImplemented functions:\n", "");
                std::for_each(fdesc->FunctionsName,
                              fdesc->FunctionsName + fdesc->NumFunctions,
                              [](mfxChar *functionName) {
                                  printf("%4s%s\n", "", functionName);
                              });

                MFXDispReleaseImplDescription(loader, fdesc);
            }
            else {
                printf("%2sWarning - MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS not supported\n", "");
            }
        }

        if (bPrintExtendedDeviceID) {
            mfxExtendedDeviceId *idescDevice;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                                   reinterpret_cast<mfxHDL *>(&idescDevice));
            if (sts == MFX_ERR_NONE) {
                printf("%2sExtended DeviceID's:\n", "");
                printf("%6sVendorID: 0x%04X\n", "", idescDevice->VendorID);
                printf("%6sDeviceID: 0x%04X\n", "", idescDevice->DeviceID);

                printf("%6sPCIDomain: 0x%08X\n", "", idescDevice->PCIDomain);
                printf("%6sPCIBus: 0x%08X\n", "", idescDevice->PCIBus);
                printf("%6sPCIdevice: 0x%08X\n", "", idescDevice->PCIDevice);
                printf("%6sPCIFunction: 0x%08X\n", "", idescDevice->PCIFunction);

                if (idescDevice->LUIDValid) {
                    printf("%6sDeviceLUID: ", "");
                    for (mfxU32 idx = 0; idx < 8; idx++) {
                        printf("%02x", idescDevice->DeviceLUID[7 - idx]);
                    }
                    printf("\n");
                    printf("%6sLUIDDeviceNodeMask: 0x%04X\n", "", idescDevice->LUIDDeviceNodeMask);
                }

                printf("%6sLUIDValid: 0x%04X\n", "", idescDevice->LUIDValid);

                printf("%6sDRMRenderNodeNum: %d\n", "", idescDevice->DRMRenderNodeNum);
                printf("%6sDRMPrimaryNodeNum: 0x%04X\n", "", idescDevice->DRMPrimaryNodeNum);

                printf("%6sRevisionID: 0x%04X\n", "", idescDevice->RevisionID);

                printf("%6sDeviceName: %s\n", "", idescDevice->DeviceName);
                MFXDispReleaseImplDescription(loader, idescDevice);
            }

            else {
                printf("%2sWarning - MFX_IMPLCAPS_DEVICE_ID_EXTENDED not supported\n", "");
            }
        }

#ifdef ONEVPL_EXPERIMENTAL
        if (bPrintSurfaceTypes) {
            mfxSurfaceTypesSupported *surf;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_SURFACE_TYPES,
                                                   reinterpret_cast<mfxHDL *>(&surf));
            if (sts == MFX_ERR_NONE) {
                printf("%2smfxSurfaceTypesSupported:\n", "");
                printf("%4sVersion: %hu.%hu\n", "", surf->Version.Major, surf->Version.Minor);
                for (int surfIdx = 0; surfIdx < surf->NumSurfaceTypes; surfIdx++) {
                    printf("%4sSurfaceType: %s\n",
                           "",
                           _print_SurfaceType(surf->SurfaceTypes[surfIdx].SurfaceType));
                    for (int compIdx = 0;
                         compIdx < surf->SurfaceTypes[surfIdx].NumSurfaceComponents;
                         compIdx++) {
                        printf("%6sSurfaceComponent: %s\n",
                               "",
                               _print_SurfaceComponent(surf->SurfaceTypes[surfIdx]
                                                           .SurfaceComponents[compIdx]
                                                           .SurfaceComponent));
                        mfxU32 surfFlags =
                            surf->SurfaceTypes[surfIdx].SurfaceComponents[compIdx].SurfaceFlags;
                        if (surfFlags) {
                            for (mfxU32 flagMask = 1; flagMask != 0; flagMask <<= 1) {
                                if (surfFlags & flagMask) {
                                    const char *flagStr = _print_SurfaceFlags(flagMask);
                                    printf("%8sSurfaceFlags:     %s\n", "", flagStr);
                                }
                            }
                        }
                    }
                }
                MFXDispReleaseImplDescription(loader, surf);
            }

            else {
                printf("%2sWarning - MFX_IMPLCAPS_SURFACE_TYPES not supported\n", "");
            }
        }
#endif

        i++;
    }

    if (i == 0)
        printf("\nWarning - no implementations found by MFXEnumImplementations()\n");
    else
        printf("\nTotal number of implementations found = %d\n", i);

    MFXUnload(loader);
    return 0;
}
