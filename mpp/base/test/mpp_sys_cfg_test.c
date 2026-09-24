/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_sys_cfg_test"

#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_time.h"
#include "mpp_common.h"

#include "rk_mpp_cfg.h"
#include "mpp_sys_cfg.h"

int main(void)
{
    MPP_RET ret = MPP_OK;
    MppSysCfg cfg;
    RK_S64 end = 0;
    RK_S64 start = 0;
    MppCodingType type = MPP_VIDEO_CodingHEVC;
    RK_U32 width = 4096;
    RK_U32 height = 2304;
    RK_U32 h_stride_by_byte;
    RK_U32 h_stride_by_pixel;
    RK_U32 v_stride;
    RK_U32 size_total;
    RK_U32 size_fbc_hdr;
    RK_U32 size_fbc_bdy;
    RK_U32 cap_version = 0;
    RK_U32 cap_supported = 0;
    RK_U32 cap_features = 0;
    RK_U32 cap_core_num = 0;

    mpp_sys_cfg_show();

    mpp_log("mpp_sys_cfg_test start\n");

    start = mpp_time();

    ret = mpp_sys_cfg_get(&cfg);
    if (ret) {
        mpp_err("mpp_sys_cfg_get failed\n");
        goto DONE;
    }

    /* set correct parameter */
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:type", 1);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:enable", 1);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:type", type);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:fmt_codec", MPP_FMT_YUV420SP);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:fmt_fbc", MPP_FRAME_FBC_AFBC_V1);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:width", width);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:height", height);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:h_stride_by_byte", 0);
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:h_stride_by_pixel", 0);

    /* try get readonly parameter */
    ret = mpp_sys_cfg_set_u32(cfg, "dec_buf_chk:cap_fbc", 1);
    if (!ret) {
        mpp_log("set readonly success, should be a failure\n");
        goto DONE;
    }

    /* get result */
    mpp_sys_cfg_ioctl(cfg);

    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:h_stride_by_byte", &h_stride_by_byte);
    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:h_stride_by_pixel", &h_stride_by_pixel);
    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:v_stride", &v_stride);
    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:size_total", &size_total);
    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:size_fbc_hdr", &size_fbc_hdr);
    ret = mpp_sys_cfg_get_u32(cfg, "dec_buf_chk:size_fbc_bdy", &size_fbc_bdy);

    /* decoder capability query */
    ret = mpp_sys_cfg_set_u32(cfg, "dec_cap:type", type);
    if (ret) {
        mpp_err("set dec_cap:type failed\n");
        goto DONE;
    }

    ret = mpp_sys_cfg_set_u32(cfg, "dec_cap:enable", 1);
    if (ret) {
        mpp_err("set dec_cap:enable failed\n");
        goto DONE;
    }

    ret = mpp_sys_cfg_ioctl(cfg);
    if (ret) {
        mpp_err("dec_cap ioctl failed\n");
        goto DONE;
    }

    ret = mpp_sys_cfg_get_u32(cfg, "dec_cap:version", &cap_version);
    if (ret)
        goto DONE;

    ret = mpp_sys_cfg_get_u32(cfg, "dec_cap:supported", &cap_supported);
    if (ret)
        goto DONE;

    ret = mpp_sys_cfg_get_u32(cfg, "dec_cap:features", &cap_features);
    if (ret)
        goto DONE;

    ret = mpp_sys_cfg_get_u32(cfg, "dec_cap:core_num", &cap_core_num);
    if (ret)
        goto DONE;

    if (cap_version != MPP_DEC_CAP_VERSION_1) {
        mpp_err("invalid dec_cap version %u\n", cap_version);
        ret = MPP_NOK;
        goto DONE;
    }

    if ((cap_supported && !cap_core_num) ||
        (!cap_supported && cap_core_num)) {
        mpp_err("invalid dec_cap support %u core_num %u\n",
                cap_supported, cap_core_num);
        ret = MPP_NOK;
        goto DONE;
    }

    mpp_log("dec cap version %u supported %u features %08x core_num %u\n",
            cap_version, cap_supported, cap_features, cap_core_num);

    /* all query outputs must remain read-only */
    ret = mpp_sys_cfg_set_u32(cfg, "dec_cap:version", 0);
    if (!ret) {
        mpp_log("set dec_cap readonly success, should be a failure\n");
        ret = MPP_NOK;
        goto DONE;
    }

    ret = MPP_OK;

    ret = mpp_sys_cfg_put(cfg);
    if (ret) {
        mpp_err("mpp_sys_cfg_put failed\n");
        goto DONE;
    }

    end = mpp_time();
    mpp_log("set u32 time %lld us\n", end - start);

DONE:
    mpp_log("mpp_sys_cfg_test done %s\n", ret ? "failed" : "success");
    return ret;
}
