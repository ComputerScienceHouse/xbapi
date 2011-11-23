#ifdef ___XBAPI_H__
#error Double include of private header _xbapi.h
#else /* ___XBAPI_H__ */
#define ___XBAPI_H__

xbapi_rc_t xbapi_escape( uint8_t **buf );

#define XBAPI_FRAME_DELIM   0x7E
#define XBAPI_ESCAPE        0x7D
#define XBAPI_XON           0x11
#define XBAPI_XOFF          0x13

#endif /* ___XBAPI_H__ */
