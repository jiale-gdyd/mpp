/*
 * Copyright 2016 Rockchip Electronics Co. LTD
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

#ifndef __RC_BASE_H__
#define __RC_BASE_H__

#include "mpp_list.h"
#include "mpp_enc_cfg.h"
#include "mpp_rc.h"

/*
 * mpp rate control contain common caculation methd
 */

/*
 * 1. MppData - data statistic struct
 *    size  - max valid data number
 *    len   - valid data number
 *    pos_r - current data read position
 *    pos_w - current data write position
 *    val   - buffer array pointer
 *
 *    When statistic length is less than 8 use direct save mode which will move
 *    all the data on each update.
 *    When statistic length is larger than 8 use loop save mode which will
 *    cyclically reuse the data position.
 */
typedef struct MppDataV2_t {
    RK_S32  size;
    RK_S32  len;
    RK_S32  pos_w;
    RK_S32  pos_r;
    RK_S32  *val;
    RK_S64  sum;
} MppDataV2;

/*
 * 2. Proportion Integration Differentiation (PID) control
 */
typedef struct MppPIDCtxV2_t {
    RK_S32  p;
    RK_S32  i;
    RK_S32  d;
    RK_S32  coef_p;
    RK_S32  coef_i;
    RK_S32  coef_d;
    RK_S32  div;
    RK_S32  len;
    RK_S32  count;
} MppPIDCtxV2;

/*
 * 3. linear module
 */
// #define LINEAR_MODEL_STATISTIC_COUNT    15

/*
 * Linear regression
 * C = a * x * r + b * x * x * r
 */
typedef struct MppLinRegV2_t {
    RK_S32 size;        /* elements max size */
    RK_S32 n;           /* elements count */
    RK_S32 i;           /* elements index for store */

    double a;           /* coefficient */
    double b;           /* coefficient */
    double c;

    RK_S32 *x;          /* x */
    RK_S32 *r;          /* r */
    RK_S64 *y;          /* y = x * x * r */
    RK_S32 weight_mode; /* different weight ratio*/
} MppLinRegV2;

/* Virtual buffer */
typedef struct MppVirtualBufferV2_t {
    RK_S32 bufferSize;          /* size of the virtual buffer */
    RK_S32 bitRate;             /* input bit rate per second */
    RK_S32 bitPerPic;           /* average number of bits per picture */
    RK_S32 picTimeInc;          /* timeInc since last coded picture */
    RK_S32 timeScale;           /* input frame rate numerator */
    RK_S32 unitsInTic;          /* input frame rate denominator */
    RK_S32 virtualBitCnt;       /* virtual (channel) bit count */
    RK_S32 realBitCnt;          /* real bit count */
    RK_S32 bufferOccupancy;     /* number of bits in the buffer */
    RK_S32 skipFrameTarget;     /* how many frames should be skipped in a row */
    RK_S32 skippedFrames;       /* how many frames have been skipped in a row */
    RK_S32 nonZeroTarget;
    RK_S32 bucketFullness;      /* Leaky Bucket fullness */
    RK_S32 gopRem;
    RK_S32 windowRem;
} MppVirtualBufferV2;

typedef struct MppRateControlV2_s {
    /* control parameter from external config */
    RK_S32 fps_in_flex;
    RK_S32 fps_in_num;
    RK_S32 fps_in_denom;
    RK_S32 fps_out;
    RK_S32 fps_num;
    RK_S32 fps_denom;

    /* I frame gop len */
    RK_S32 igop;
    /* visual gop len */
    RK_S32 vgop;

    /* bitrate parameter */
    RK_S32 bps_min;
    RK_S32 bps_target;
    RK_S32 bps_max;
    /* max I frame bit ratio to P frame bit */
    RK_S32 max_i_bit_prop;
    RK_S32 min_i_bit_prop;
    /* layer bitrate proportion */
    RK_S32 layer_bit_prop[4];

    /* quality parameter */
    RK_S32 max_quality;
    RK_S32 min_quality;
    RK_S32 max_i_quality;
    RK_S32 min_i_quality;
    RK_S32 i_quality_delta;
    /* layer quality proportion */
    RK_S32 layer_quality_delta[4];

    /* reencode parameter */
    RK_S32 max_reencode_times;

    /* still / motion desision parameter */
    RK_S32 min_still_prop;
    RK_S32 max_still_quality;

    /*
     * vbr parameter
     *
     * vbr_hi_prop  - high proportion bitrate for reduce quality
     * vbr_lo_prop  - low proportion bitrate for increase quality
     */
    RK_S32 vbr_hi_prop;
    RK_S32 vbr_lo_prop;


    /*
     * derivation parameter
     * bits_per_pic - average bit count in gop
     * bits_per_intra   - target intra frame size
     *                if not intra frame is encoded default set to a certain
     *                times of inter frame according to gop
     * bits_per_inter   - target inter frame size
     */
    MppEncGopMode gop_mode;
    RK_S32 bits_per_pic;
    RK_S32 bits_per_intra;
    RK_S32 bits_per_inter;
    RK_S32 mb_per_frame;

    /* bitrate window which tries to match target */
    RK_S32 window_len;
    RK_S32 intra_to_inter_rate;

    RK_S32 acc_intra_bits_in_fps;
    RK_S32 acc_inter_bits_in_fps;
    RK_S32 acc_total_bits;
    RK_U32 time_in_second;

    RK_S32 acc_intra_count;
    RK_S32 acc_inter_count;
    RK_S32 acc_total_count;
    RK_S32 last_fps_bits;
    float last_intra_percent;

    RK_S32 prev_intra_target;
    RK_S32 prev_inter_target;

    /* runtime status parameter */
    ENC_FRAME_TYPE cur_frmtype;
    ENC_FRAME_TYPE pre_frmtype;

    /*
     * intra     - intra frame bits record
     * inter     - inter frame bits record
     * pid_intra - intra frame bits record
     * pid_inter - inter frame bits record
     * pid_gop   - serial frame bits record
     */
    MppDataV2 *intra;
    MppDataV2 *inter;
    MppDataV2 *gop_bits;
    MppDataV2 *intra_percent;
    MppPIDCtxV2 pid_intra;
    MppPIDCtxV2 pid_inter;
    MppPIDCtxV2 pid_fps;
    /*
     * Vbv buffer control
    */
    MppVirtualBufferV2  vb;
    RK_S32              hrd;

    /*
     * output target bits on current status
     * 0        - do not do rate control
     * non-zero - have rate control
     */
    RK_S32 bits_target;
    RK_S32 pre_gop_left_bit;
    float max_rate;
    float min_rate;

    /* start from ONE */
    RK_U32 frm_cnt;
    RK_S32 real_bps;
    RK_S32 prev_aq_prop_offset;
    RK_S32 quality;

} MppRateControlV2;

/*
 * Data structure from encoder to hal
 * type         - frame encode type
 * bit_target   - frame target size
 * bit_min      - frame minimum size
 * bit_max      - frame maximum size
 */
typedef struct RcSyntaxV2_t {
    MppEncGopMode    gop_mode;
    ENC_FRAME_TYPE   type;
    RK_S32           bit_target;
    RK_S32           bit_max;
    RK_S32           bit_min;
    RK_S32           aq_prop_offset;

    /* head node of rc parameter list */
    struct list_head *rc_head;
} RcSyntaxV2;

/*
 * Data structure from hal to encoder
 * type         - frame encode type
 * bits         - frame actual byte size
 */
typedef struct HalRcResultV2_s {
    ENC_FRAME_TYPE  type;
    RK_S32          time;
    RK_S32          bits;
} RcHalResultV2;

typedef struct RecordNodeV2_t {
    struct list_head list;
    ENC_FRAME_TYPE   frm_type;
    /* @frm_cnt starts from ONE */
    RK_U32           frm_cnt;
    RK_U32           bps;
    RK_U32           fps;
    RK_S32           gop;
    RK_S32           bits_per_pic;
    RK_S32           bits_per_intra;
    RK_S32           bits_per_inter;
    RK_U32           tgt_bits;
    RK_U32           bit_min;
    RK_U32           bit_max;
    RK_U32           real_bits;
    RK_S32           acc_intra_bits_in_fps;
    RK_S32           acc_inter_bits_in_fps;
    RK_S32           last_fps_bits;
    float            last_intra_percent;

    /* hardware result */
    RK_S32           qp_sum;
    RK_S64           sse_sum;
    RK_S32           set_qp;
    RK_S32           qp_min;
    RK_S32           qp_max;
    RK_S32           real_qp;
    MppLinRegV2      lin_reg;
    RK_S32           wlen;
} RecordNodeV2;

#ifdef __cplusplus
extern "C" {
#endif

MPP_RET mpp_data_init_v2(MppDataV2 **p, RK_S32 len, RK_S32 value);
void mpp_data_deinit_v2(MppDataV2 *p);
void mpp_data_reset_v2(MppDataV2 *p, RK_S32 val);
void mpp_data_update_v2(MppDataV2 *p, RK_S32 val);
RK_S32 mpp_data_sum_v2(MppDataV2 *p);
RK_S32 mpp_data_mean_v2(MppDataV2 *p);
RK_S32 mpp_data_get_pre_val_v2(MppDataV2 *p, RK_S32 idx);
RK_S32 mpp_data_sum_with_ratio_v2(MppDataV2 *p, RK_S32 len, RK_S32 num, RK_S32 denorm);

void mpp_pid_reset_v2(MppPIDCtxV2 *p);
void mpp_pid_set_param_v2(MppPIDCtxV2 *p, RK_S32 coef_p, RK_S32 coef_i, RK_S32 coef_d, RK_S32 div, RK_S32 len);
void mpp_pid_update_v2(MppPIDCtxV2 *p, RK_S32 val);
RK_S32 mpp_pid_calc_v2(MppPIDCtxV2 *ctx);

MPP_RET mpp_rc_init_v2(MppRateControlV2 **ctx);
MPP_RET mpp_rc_deinit_v2(MppRateControlV2 *ctx);

/*
 * Translate MppEncRcCfg struct to internal bitrate setting
 * Called in mpp_control function.
 * If parameter changed mark flag and let encoder recalculate bit allocation.
 */
MPP_RET mpp_rc_set_user_cfg_v2(MppRateControlV2 *ctx, MppEncCfgSet *cfg, RK_S32 force_idr);

/*
 * When one frame is encoded hal will call this function to update paramter
 * from hardware. Hardware will update bits / qp_sum / mad or sse data
 *
 * Then rate control will update the linear regression model
 */
MPP_RET mpp_rc_update_hw_result_v2(MppRateControlV2 *ctx, RcHalResultV2 *result);

/*
 * Use bps/fps config generate bit allocation setting
 * Called in controller loop when parameter changed or get a encoder result.
 * This function will calculation next frames target bits according to current
 * bit rate status.
 *
 * bits[0] - target
 * bits[1] - min
 * bits[1] - max
 */
MPP_RET mpp_rc_bits_allocation_v2(MppRateControlV2 *ctx, RcSyntaxV2 *rc_syn);

MPP_RET mpp_rc_record_param_v2(struct list_head *head, MppRateControlV2 *ctx,
                               RcSyntaxV2 *rc_syn);

MPP_RET mpp_rc_calc_real_bps_v2(struct list_head *head, MppRateControlV2 *ctx,
                                RK_S32 cur_bits);

MPP_RET mpp_rc_param_ops_v2(struct list_head *head, RK_U32 frm_cnt,
                            RC_PARAM_OPS ops, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __RC_BASE_H__ */
