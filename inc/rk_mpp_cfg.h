/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 */

#ifndef RK_MPP_CFG_H
#define RK_MPP_CFG_H

#include "rk_type.h"
#include "mpp_err.h"

typedef enum MppCfgStrFmt_e {
    MPP_CFG_STR_FMT_LOG,
    MPP_CFG_STR_FMT_JSON,
    MPP_CFG_STR_FMT_TOML,
    MPP_CFG_STR_FMT_BUTT,
} MppCfgStrFmt;

typedef void* MppSysCfg;

/*
 * Decoder capability query through MppSysCfg.
 *
 * Input keys:
 *   dec_cap:type              u32 MppCodingType
 *   dec_cap:enable            u32, set to 1 before mpp_sys_cfg_ioctl()
 *
 * Read-only output keys:
 *   dec_cap:version
 *   dec_cap:supported
 *   dec_cap:features
 *   dec_cap:core_num
 *   dec_cap:cap_fbc
 *   dec_cap:cap_4k
 *   dec_cap:cap_8k
 *   dec_cap:cap_10bit
 *   dec_cap:cap_colmv_compress
 *   dec_cap:cap_down_scale
 *
 * Hardware fields are the union / maximum capability of decoder hardware
 * blocks on the current SoC that support the requested coding type.
 *
 * MPP_DEC_CAP_FEATURE_MULTICORE reports availability of multiple physical
 * decoder cores. It does not imply or expose userspace core selection.
 */
typedef enum MppDecCapVersion_e {
    MPP_DEC_CAP_VERSION_NONE = 0,
    MPP_DEC_CAP_VERSION_1    = 1,
} MppDecCapVersion;

typedef enum MppDecCapFeature_e {
    MPP_DEC_CAP_FEATURE_COLMV_META = (1U << 0),
    MPP_DEC_CAP_FEATURE_HW_STAT    = (1U << 1),
    MPP_DEC_CAP_FEATURE_THUMBNAIL  = (1U << 2),
    MPP_DEC_CAP_FEATURE_MULTICORE  = (1U << 3),
} MppDecCapFeature;

#ifdef __cplusplus
extern "C" {
#endif

rk_s32 mpp_sys_cfg_get(MppSysCfg *cfg);
rk_s32 mpp_sys_cfg_put(MppSysCfg cfg);
MPP_RET mpp_sys_cfg_ioctl(MppSysCfg cfg);

MPP_RET mpp_sys_cfg_set_s32(MppSysCfg cfg, const char *name, RK_S32 val);
MPP_RET mpp_sys_cfg_set_u32(MppSysCfg cfg, const char *name, RK_U32 val);
MPP_RET mpp_sys_cfg_set_s64(MppSysCfg cfg, const char *name, RK_S64 val);
MPP_RET mpp_sys_cfg_set_u64(MppSysCfg cfg, const char *name, RK_U64 val);
MPP_RET mpp_sys_cfg_set_ptr(MppSysCfg cfg, const char *name, void *val);

MPP_RET mpp_sys_cfg_get_s32(MppSysCfg cfg, const char *name, RK_S32 *val);
MPP_RET mpp_sys_cfg_get_u32(MppSysCfg cfg, const char *name, RK_U32 *val);
MPP_RET mpp_sys_cfg_get_s64(MppSysCfg cfg, const char *name, RK_S64 *val);
MPP_RET mpp_sys_cfg_get_u64(MppSysCfg cfg, const char *name, RK_U64 *val);
MPP_RET mpp_sys_cfg_get_ptr(MppSysCfg cfg, const char *name, void **val);

void mpp_sys_cfg_show(void);

#ifdef __cplusplus
}
#endif

#endif /* RK_MPP_CFG_H */
