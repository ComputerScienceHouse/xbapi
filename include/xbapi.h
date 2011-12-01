#ifndef __XBAPI_H__
#define __XBAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define export __attribute__((visibility("default")))

typedef enum {
	XBAPI_ERR_NOERR,
	XBAPI_ERR_SYS,
	XBAPI_ERR_OVERFLOW,
	XBAPI_ERR_BADPACKET,
	XBAPI_ERR_BUFBIG,
	XBAPI_ERR_INCWRITE
} xbapi_err_e;

typedef enum {
	// Addressing
	XBAPI_AT_DH,
	XBAPI_AT_DL,
	XBAPI_AT_MY,
	XBAPI_AT_MP,
	XBAPI_AT_NC,
	XBAPI_AT_SH,
	XBAPI_AT_SL,
	XBAPI_AT_NI,
	XBAPI_AT_SE,
	XBAPI_AT_DE,
	XBAPI_AT_CI,
	XBAPI_AT_NP,
	XBAPI_AT_DD,
	// Networking
	XBAPI_AT_CH,
	XBAPI_AT_ID,
	XBAPI_AT_OP,
	XBAPI_AT_NH,
	XBAPI_AT_BH,
	XBAPI_AT_OI,
	XBAPI_AT_NT,
	XBAPI_AT_NO,
	XBAPI_AT_SC,
	XBAPI_AT_SD,
	XBAPI_AT_ZS,
	XBAPI_AT_NJ,
	XBAPI_AT_JV,
	XBAPI_AT_NW,
	XBAPI_AT_JN,
	XBAPI_AT_AR,
	// Security
	XBAPI_AT_EE,
	XBAPI_AT_EO,
	XBAPI_AT_NK,
	XBAPI_AT_KY,
	// RF Interfacing
	XBAPI_AT_PL,
	XBAPI_AT_PM,
	XBAPI_AT_DB,
	XBAPI_AT_PP,
	// Serial Interfacing
	XBAPI_AT_AP,
	XBAPI_AT_AO,
	XBAPI_AT_BD,
	XBAPI_AT_NB,
	XBAPI_AT_SB,
	XBAPI_AT_RO,
	XBAPI_AT_D7,
	XBAPI_AT_D6,
	// IO
	XBAPI_AT_IR,
	XBAPI_AT_IC,
	XBAPI_AT_P0,
	XBAPI_AT_P1,
	XBAPI_AT_P2,
	XBAPI_AT_P3,
	XBAPI_AT_D0,
	XBAPI_AT_D1,
	XBAPI_AT_D2,
	XBAPI_AT_D3,
	XBAPI_AT_D4,
	XBAPI_AT_D5,
	XBAPI_AT_D8,
	XBAPI_AT_LT,
	XBAPI_AT_PR,
	XBAPI_AT_RP,
	XBAPI_AT_SV,  // %V
	XBAPI_AT_VS,  // V+
	XBAPI_AT_TP,
	// Diagnostics
	XBAPI_AT_VR,
	XBAPI_AT_HV,
	XBAPI_AT_AI,
	// AT Command Options
	XBAPI_AT_CT,
	XBAPI_AT_CN,
	XBAPI_AT_GT,
	XBAPI_AT_CC,
	// Sleep
	XBAPI_AT_SM,
	XBAPI_AT_SN,
	XBAPI_AT_SP,
	XBAPI_AT_ST,
	XBAPI_AT_SO,
	XBAPI_AT_WH,
	XBAPI_AT_PO,
	// Execution
	XBAPI_AT_AC,
	XBAPI_AT_WR,
	XBAPI_AT_RE,
	XBAPI_AT_FR,
	XBAPI_AT_NR,
	XBAPI_AT_SI,
	XBAPI_AT_CB,
	XBAPI_AT_ND,
	XBAPI_AT_DN,
	XBAPI_AT_IS,
	XBAPI_AT_1S
} xbapi_at_e;

typedef union {
	uint64_t u128[2];
	uint32_t u32;
	uint16_t u16;
	uint8_t u8;
	uint8_t *text;
} xbapi_at_arg_u;

typedef struct {
	xbapi_err_e code;
	union {
		int sys_errno;
	};
} xbapi_rc_t;

typedef enum {
	XBAPI_OP_STATUS_OK,
	XBAPI_OP_STATUS_ERROR,
	XBAPI_OP_STATUS_INVALID_CMD,
	XBAPI_OP_STATUS_INVALID_PARAM,
	XBAPI_OP_STATUS_TX_FAILURE,
	XBAPI_OP_STATUS_PENDING
} xbapi_op_status_e;

typedef struct {
	uint8_t frame_id;
	xbapi_op_status_e status;
	uint8_t *data;
} xbapi_op_t;

export xbapi_err_e xbapi_errno( xbapi_rc_t err );
export int xbapi_sys_errno( xbapi_rc_t err );

export xbapi_rc_t
	xbapi_rc( xbapi_err_e err ),
	xbapi_rc_sys();

export const char *xbapi_strerror( xbapi_rc_t err );

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* __XBAPI_H__ */
