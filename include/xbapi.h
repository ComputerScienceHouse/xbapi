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
	XBAPI_ERR_BUFBIG
} xbapi_err_e;

typedef struct {
	xbapi_err_e code;
	union {
		int sys_errno;
	};
} xbapi_rc_t;

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
