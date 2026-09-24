/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MODULE_TAG "vdpu34x_com"

#include <string.h>
#include <stdlib.h>

#include "mpp_log.h"
#include "mpp_buffer.h"
#include "mpp_common.h"
#include "mpp_frame_impl.h"
#include "mpp_compat_impl.h"
#include "mpp_env.h"

#include "vdpu_com.h"
#include "vdpu34x_com.h"

static RK_U32 rcb_coeff[RCB_BUF_COUNT] = {
    [RCB_INTRA_ROW]     = 6,   /* RCB_INTRA_ROW_COEF */
    [RCB_TRANSD_ROW]    = 1,   /* RCB_TRANSD_ROW_COEF */
    [RCB_TRANSD_COL]    = 1,   /* RCB_TRANSD_COL_COEF */
    [RCB_STRMD_ROW]     = 3,   /* RCB_STRMD_ROW_COEF */
    [RCB_INTER_ROW]     = 6,   /* RCB_INTER_ROW_COEF */
    [RCB_INTER_COL]     = 3,   /* RCB_INTER_COL_COEF */
    [RCB_DBLK_ROW]      = 22,  /* RCB_DBLK_ROW_COEF */
    [RCB_SAO_ROW]       = 6,   /* RCB_SAO_ROW_COEF */
    [RCB_FBC_ROW]       = 11,  /* RCB_FBC_ROW_COEF */
    [RCB_FILT_COL]      = 67,  /* RCB_FILT_COL_COEF */
};

static RK_S32 update_size_offset(VdpuRcbInfo *info, RK_U32 reg,
                                 RK_S32 offset, RK_S32 len, RK_S32 idx)
{
    RK_S32 buf_size = 0;

    buf_size = MPP_ALIGN(len * rcb_coeff[idx], RCB_ALLINE_SIZE);
    info[idx].reg_idx = reg;
    info[idx].offset = offset;
    info[idx].size = buf_size;

    return buf_size;
}

RK_S32 vdpu34x_get_rcb_buf_size(VdpuRcbInfo *info, RK_S32 width, RK_S32 height)
{
    RK_S32 offset = 0;

    offset += update_size_offset(info, 139, offset, width, RCB_DBLK_ROW);
    offset += update_size_offset(info, 133, offset, width, RCB_INTRA_ROW);
    offset += update_size_offset(info, 134, offset, width, RCB_TRANSD_ROW);
    offset += update_size_offset(info, 136, offset, width, RCB_STRMD_ROW);
    offset += update_size_offset(info, 137, offset, width, RCB_INTER_ROW);
    offset += update_size_offset(info, 140, offset, width, RCB_SAO_ROW);
    offset += update_size_offset(info, 141, offset, width, RCB_FBC_ROW);
    /* col rcb */
    offset += update_size_offset(info, 135, offset, height, RCB_TRANSD_COL);
    offset += update_size_offset(info, 138, offset, height, RCB_INTER_COL);
    offset += update_size_offset(info, 142, offset, height, RCB_FILT_COL);

    return offset;
}

void vdpu34x_setup_rcb(Vdpu34xRegCommAddr *reg, MppDev dev, MppBuffer buf, VdpuRcbInfo *info)
{
    RK_S32 fd = mpp_buffer_get_fd(buf);

    reg->reg139_rcb_dblk_base           = fd;
    reg->reg133_rcb_intra_base          = fd;
    reg->reg134_rcb_transd_row_base     = fd;
    reg->reg136_rcb_streamd_row_base    = fd;
    reg->reg137_rcb_inter_row_base      = fd;
    reg->reg140_rcb_sao_base            = fd;
    reg->reg141_rcb_fbc_base            = fd;
    reg->reg135_rcb_transd_col_base     = fd;
    reg->reg138_rcb_inter_col_base      = fd;
    reg->reg142_rcb_filter_col_base     = fd;

    if (info[RCB_DBLK_ROW].offset)
        mpp_dev_set_reg_offset(dev, 139, info[RCB_DBLK_ROW].offset);
    if (info[RCB_INTRA_ROW].offset)
        mpp_dev_set_reg_offset(dev, 133, info[RCB_INTRA_ROW].offset);
    if (info[RCB_TRANSD_ROW].offset)
        mpp_dev_set_reg_offset(dev, 134, info[RCB_TRANSD_ROW].offset);
    if (info[RCB_STRMD_ROW].offset)
        mpp_dev_set_reg_offset(dev, 136, info[RCB_STRMD_ROW].offset);
    if (info[RCB_INTER_ROW].offset)
        mpp_dev_set_reg_offset(dev, 137, info[RCB_INTER_ROW].offset);
    if (info[RCB_SAO_ROW].offset)
        mpp_dev_set_reg_offset(dev, 140, info[RCB_SAO_ROW].offset);
    if (info[RCB_FBC_ROW].offset)
        mpp_dev_set_reg_offset(dev, 141, info[RCB_FBC_ROW].offset);
    if (info[RCB_TRANSD_COL].offset)
        mpp_dev_set_reg_offset(dev, 135, info[RCB_TRANSD_COL].offset);
    if (info[RCB_INTER_COL].offset)
        mpp_dev_set_reg_offset(dev, 138, info[RCB_INTER_COL].offset);
    if (info[RCB_FILT_COL].offset)
        mpp_dev_set_reg_offset(dev, 142, info[RCB_FILT_COL].offset);
}

RK_S32 vdpu34x_set_rcbinfo(MppDev dev, VdpuRcbInfo *rcb_info)
{
    MppDevRcbInfoCfg rcb_cfg;
    RK_U32 i;
    VdpuRcbSetMode set_rcb_mode = RCB_SET_BY_PRIORITY_MODE;
    static const RK_U32 rcb_priority[RCB_BUF_COUNT] = {
        RCB_DBLK_ROW,
        RCB_INTRA_ROW,
        RCB_SAO_ROW,
        RCB_INTER_ROW,
        RCB_FBC_ROW,
        RCB_TRANSD_ROW,
        RCB_STRMD_ROW,
        RCB_INTER_COL,
        RCB_FILT_COL,
        RCB_TRANSD_COL,
    };

    switch (set_rcb_mode) {
    case RCB_SET_BY_SIZE_SORT_MODE : {
        VdpuRcbInfo info[RCB_BUF_COUNT];

        memcpy(info, rcb_info, sizeof(info));
        qsort(info, MPP_ARRAY_ELEMS(info),
              sizeof(info[0]), vdpu_compare_rcb_size);

        for (i = 0; i < MPP_ARRAY_ELEMS(info); i++) {
            rcb_cfg.reg_idx = info[i].reg_idx;
            rcb_cfg.size = info[i].size;
            if (rcb_cfg.size > 0) {
                mpp_dev_ioctl(dev, MPP_DEV_RCB_INFO, &rcb_cfg);
            } else
                break;
        }
    } break;
    case RCB_SET_BY_PRIORITY_MODE : {
        VdpuRcbInfo *info = rcb_info;
        RK_U32 index = 0;

        for (i = 0; i < MPP_ARRAY_ELEMS(rcb_priority); i ++) {
            index = rcb_priority[i];
            /*
             * If the inter row rcb buffer is placed in sram,
             * may conflict with other buffer in ddr,
             * will result in slower access to data and degraded decoding performance.
             * The issue will be resolved in chips after rk3588.
             */
            if (index == RCB_INTER_ROW)
                continue;

            rcb_cfg.reg_idx = info[index].reg_idx;
            rcb_cfg.size = info[index].size;
            if (rcb_cfg.size > 0) {
                mpp_dev_ioctl(dev, MPP_DEV_RCB_INFO, &rcb_cfg);
            }
        }
    } break;
    default:
        break;
    }

    return 0;
}

void vdpu34x_setup_statistic(Vdpu34xRegComm *com, Vdpu34xRegStatistic *sta)
{
    com->reg011.pix_range_detection_e = 1;

    memset(sta, 0, sizeof(*sta));

    sta->reg256.axi_perf_work_e = 1;
    sta->reg256.axi_perf_clr_e = 1;
    sta->reg256.axi_cnt_type = 1;

    sta->reg257.addr_align_type = 1;

    /* set hurry */
    sta->reg270.axi_rd_hurry_level = 3;
    sta->reg270.axi_wr_hurry_level = 1;
    sta->reg270.axi_wr_qos = 1;
    sta->reg270.axi_rd_qos = 3;
    sta->reg270.bus2mc_buffer_qos_level = 255;
    sta->reg271_wr_wait_cycle_qos = 0;
}

RK_U32 vdpu34x_get_colmv_size(RK_U32 width, RK_U32 height, RK_U32 ctu_size,
                              RK_U32 colmv_bytes, RK_U32 colmv_size, RK_U32 compress)
{
    RK_U32 colmv_total_size;

    if (compress) {
        RK_U32 segment_w = (64 * colmv_size * colmv_size) / ctu_size;
        RK_U32 segment_h = ctu_size;
        RK_U32 seg_cnt_w = MPP_ALIGN(width, segment_w) / segment_w;
        RK_U32 seg_cnt_h = MPP_ALIGN(height, segment_h) / segment_h;
        RK_U32 seg_head_size = MPP_ALIGN(seg_cnt_w, 16) * seg_cnt_h;
        RK_U32 seg_payload_size = seg_cnt_w * seg_cnt_h * 64 * colmv_bytes;

        colmv_total_size = seg_head_size + seg_payload_size;
    } else {
        RK_U32 colmv_block_size_w = MPP_ALIGN(width, 64) / colmv_size;
        RK_U32 colmv_block_size_h = MPP_ALIGN(height, 64) / colmv_size;

        colmv_total_size = colmv_block_size_w * colmv_block_size_h * colmv_bytes;
    }

    return MPP_ALIGN(colmv_total_size, 128);
}

MPP_RET vdpu34x_set_colmv_size(MppBufSlots frame_slots, RK_S32 output,
                               RK_U32 size)
{
    MppFrame frame = NULL;
    MppMeta meta = NULL;

    if (!frame_slots || output < 0)
        return MPP_NOK;

    mpp_buf_slot_get_prop(frame_slots, output, SLOT_FRAME_PTR, &frame);
    if (!frame)
        return MPP_NOK;

    meta = mpp_frame_get_meta(frame);
    if (!meta)
        return MPP_NOK;

    return mpp_meta_set_s32(meta, KEY_DEC_COLMV_SIZE, (RK_S32)size);
}

MPP_RET vdpu34x_export_colmv(MppBufSlots frame_slots, RK_S32 output,
                             MppBuffer colmv, RK_S32 fmt, RK_U32 valid)
{
    MppFrame frame = NULL;
    MppMeta meta = NULL;
    MPP_RET ret = MPP_OK;

    if (!frame_slots || output < 0)
        return MPP_NOK;

    mpp_buf_slot_get_prop(frame_slots, output, SLOT_FRAME_PTR, &frame);
    if (!frame)
        return MPP_NOK;

    if (!valid || !colmv ||
        fmt <= MPP_DEC_COLMV_FMT_NONE ||
        fmt >= MPP_DEC_COLMV_FMT_BUTT) {
        valid = 0;
        colmv = NULL;
        fmt = MPP_DEC_COLMV_FMT_NONE;
    }

    /*
     * The frame owns an extra reference while raw COLMV is exported.
     * Invalid completion clears any previously attached reference first.
     */
    if (!valid)
        mpp_frame_set_colmv_buffer(frame, NULL);

    meta = mpp_frame_get_meta(frame);
    if (!meta)
        return MPP_NOK;

    if (valid)
        mpp_frame_set_colmv_buffer(frame, colmv);

    ret |= mpp_meta_set_buffer(meta, KEY_DEC_COLMV, colmv);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_COLMV_FMT, fmt);

    if (!valid)
        ret |= mpp_meta_set_s32(meta, KEY_DEC_COLMV_SIZE, 0);

    return ret;
}
MPP_RET vdpu34x_export_statistic(MppBufSlots frame_slots, RK_S32 output,
                                 const Vdpu34xRegStatistic *stat,
                                 RK_U32 valid)
{
    MppFrame frame = NULL;
    MppMeta meta = NULL;
    MPP_RET ret = MPP_OK;
    RK_S64 rd_max_latency = 0;
    RK_S64 rd_latency_thr_count = 0;
    RK_S64 rd_latency_acc_sum = 0;
    RK_S64 rd_axi_bytes = 0;
    RK_S64 wr_axi_bytes = 0;
    RK_S64 working_count = 0;
    RK_S32 y_min = 0;
    RK_S32 y_max = 0;
    RK_S32 u_min = 0;
    RK_S32 u_max = 0;
    RK_S32 v_min = 0;
    RK_S32 v_max = 0;
    RK_S64 err_spread = 0;

    if (!frame_slots || output < 0)
        return MPP_NOK;

    mpp_buf_slot_get_prop(frame_slots, output, SLOT_FRAME_PTR, &frame);
    if (!frame)
        return MPP_NOK;

    if (!valid || !stat)
        valid = 0;

    if (valid) {
        rd_max_latency = stat->reg258.rd_max_latency_num;
        rd_latency_thr_count = stat->reg259_rd_latency_thr_num_ch0;
        rd_latency_acc_sum = stat->reg260_rd_latency_acc_sum;
        rd_axi_bytes = stat->reg261_perf_rd_axi_total_byte;
        wr_axi_bytes = stat->reg262_perf_wr_axi_total_byte;
        working_count = stat->reg263_perf_working_cnt;
        y_min = stat->reg274_y_min_value;
        y_max = stat->reg274_y_max_value;
        u_min = stat->reg275_u_min_value;
        u_max = stat->reg275_u_max_value;
        v_min = stat->reg276_v_min_value;
        v_max = stat->reg276_v_max_value;
        err_spread = stat->reg277.err_spread_cnt_sum;
    }

    meta = mpp_frame_get_meta(frame);
    if (!meta)
        return MPP_NOK;

    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_VERSION,
                            valid ? MPP_DEC_HW_STAT_VERSION_1 :
                            MPP_DEC_HW_STAT_VERSION_NONE);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_RD_MAX_LATENCY,
                            rd_max_latency);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_RD_LATENCY_THR_COUNT,
                            rd_latency_thr_count);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_RD_LATENCY_ACC_SUM,
                            rd_latency_acc_sum);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_RD_AXI_BYTES,
                            rd_axi_bytes);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_WR_AXI_BYTES,
                            wr_axi_bytes);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_WORKING_COUNT,
                            working_count);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_Y_MIN, y_min);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_Y_MAX, y_max);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_U_MIN, u_min);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_U_MAX, u_max);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_V_MIN, v_min);
    ret |= mpp_meta_set_s32(meta, KEY_DEC_HW_STAT_V_MAX, v_max);
    ret |= mpp_meta_set_s64(meta, KEY_DEC_HW_STAT_ERR_SPREAD,
                            err_spread);

    return ret;
}