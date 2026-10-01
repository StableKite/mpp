/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2015 Rockchip Electronics Co., Ltd.
 */

#ifndef MPP_ENC_CFG_H
#define MPP_ENC_CFG_H

#include "rk_venc_cfg.h"
#include "rk_venc_cmd.h"
#include "rk_venc_ref.h"
#include "mpp_rc_defs.h"
#include "mpp_err.h"

#include "kmpp_obj.h"

/*
 * MppEncCfgSet shows the relationship between different configuration
 * Due to the huge amount of configurable parameters we need to setup
 * only minimum amount of necessary parameters.
 *
 * For normal user rc and prep config are enough.
 */
#define POS_TO_FLAG(p, pos) ((rk_u8*)(p) + ((rk_u32)(pos) & 0xffff))
#define POS_TO_ELEM(p, pos) ((rk_u8*)(p) + ((rk_u32)(pos) >> 16))

typedef struct MppEncCfgSet_t {
    MppEncBaseCfg       base;

    // esential config
    MppEncPrepCfg       prep;
    MppEncRcCfg         rc;

    // hardware related config
    MppEncHwCfg         hw;

    // codec detail config
    MppEncH264Cfg       h264;
    MppEncH264ScalingListCfg h264_scaling_list_cfg;
    MppEncH265Cfg       h265;
    MppEncH265ScalingListCfg h265_scaling_list_cfg;
    MppEncJpegCfg       jpeg;
    MppEncVp8Cfg        vp8;

    MppEncSliceSplit    split;
    MppEncRefCfg        ref_cfg;
    union {
        MppEncROICfg    roi;
        /* for kmpp venc roi */
        MppEncROICfgLegacy roi_legacy;
    };
    /* for kmpp venc osd */
    MppEncOSDData3      osd;
    MppEncOSDPltCfg     plt_cfg;
    MppEncOSDPlt        plt_data;
    /* for kmpp venc ref */
    MppEncRefParam      ref_param;

    // quality fine tuning config
    MppEncFineTuneCfg   tune;
} MppEncCfgSet;

static inline MPP_RET mpp_enc_h264_scaling_list_check(const MppEncH264ScalingListCfg *cfg)
{
    RK_S32 i;

    if (!cfg || cfg->version != MPP_ENC_H264_SCALING_LIST_VERSION_1)
        return MPP_ERR_VALUE;

    for (i = 0; i < 64; i++) {
        if (!cfg->intra8x8[i] || !cfg->inter8x8[i])
            return MPP_ERR_VALUE;
    }

    return MPP_OK;
}

static inline MPP_RET mpp_enc_h265_scaling_list_check(const MppEncH265ScalingListCfg *cfg)
{
    RK_S32 m;
    RK_S32 i;

    if (!cfg || cfg->version != MPP_ENC_H265_SCALING_LIST_VERSION_1)
        return MPP_ERR_VALUE;

    for (m = 0; m < 6; m++) {
        for (i = 0; i < 64; i++) {
            if (!cfg->tu8[m][i] || !cfg->tu16[m][i])
                return MPP_ERR_VALUE;
        }
    }

    for (m = 0; m < 2; m++) {
        for (i = 0; i < 64; i++) {
            if (!cfg->tu32[m][i])
                return MPP_ERR_VALUE;
        }
    }

    for (i = 0; i < 8; i++) {
        if (!cfg->dc[i])
            return MPP_ERR_VALUE;
    }

    return MPP_OK;
}

#ifdef __cplusplus
extern "C" {
#endif

rk_u32 *mpp_enc_cfg_prep_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_rc_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_hw_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_tune_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_h264_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_h265_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_jpeg_change(MppEncCfgSet *cfg);
rk_u32 *mpp_enc_cfg_vp8_change(MppEncCfgSet *cfg);

#define KMPP_OBJ_NAME               mpp_enc_cfg
#define KMPP_OBJ_INTF_TYPE          MppEncCfg
#include "kmpp_obj_func.h"

#ifdef __cplusplus
}
#endif

#endif /* MPP_ENC_CFG_H */
