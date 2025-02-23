#ifndef _BLACKSAIL_UTILS_H
#define _BLACKSAIL_UTILS_H

/*
 * _bytes : Input byte array
 * _str   : Output string
 * _len   : Byte array length
 * 
 * NOTE: _str should be at least (_len * 3 + 1) bytes big.
 */
#define BYTES_TO_URL(_bytes, _str, _len)								\
	do {																\
		const char *_hex = "0123456789ABCDEF"; 							\
		int _pos = 0;													\
		for (int i = 0; i < (_len); i++) {								\
			if (('a' <= (_bytes)[i] && (_bytes)[i] <= 'z') ||			\
				('A' <= (_bytes)[i] && (_bytes)[i] <= 'Z') ||			\
				('0' <= (_bytes)[i] && (_bytes)[i] <= '9')) {			\
				(_str)[_pos++] = (_bytes)[i];							\
			} else {													\
				(_str)[_pos++] = '%';									\
				(_str)[_pos++] = _hex[(_bytes)[i] >> 4];				\
				(_str)[_pos++] = _hex[(_bytes)[i] & 15];				\
			}															\
		}																\
		(_str)[_pos] = '\0';											\
	} while (0);

/*
 * Converts milliseconds to microseconds.
 */
#define MS_TO_US(x) ((x) * 1000)

#endif /* _BLACKSAIL_UTILS_H */