/**
 * types.h
 */

#ifndef _TYPES_H
# define _TYPES_H

# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>

typedef uint64_t	u64;
typedef uint32_t	u32;
typedef uint16_t	u16;
typedef uint8_t		u8;

typedef	uintptr_t	uptr;

typedef int64_t		i64;
typedef int32_t		i32;
typedef int16_t		i16;
typedef int8_t		i8;

typedef	intptr_t	iptr;

typedef float		f32;
typedef double		f64;

# define    SHIFT_ARGS(ac, av)  (ac--, *av++)
# define    UNUSED(_x)          (void)(_x)

# define    info(_s, ...)                                              \
    dprintf(2, "[INFO]   | "_s"\n", ##__VA_ARGS__);

# define    error(_s, ...)                                            \
    dprintf(2, "[ERROR]  | "_s"\n", ##__VA_ARGS__);

# define    warning(_s, ...)                                          \
    dprintf(2, "[WARNING]| "_s"\n", ##__VA_ARGS__);

# define    _CONCAT(_a, _b) _a##_b
# define    CONCAT(_a, _b)  _CONCAT(_a, _b)

# define    STRINGIFY(_x)   #_x

# define	ARRAY_LEN(_arr)		(sizeof(_arr)/sizeof(_arr[0]))

# define	MIN(_a, _b)		((_a) < (_b) ? (_a) : (_b))
# define	MAX(_a, _b)		((_a) > (_b) ? (_a) : (_b))

# define	CLAMP(_v, _min, _max)	MIN(MAX(_v, _min), _max)

# define	struct_null(_t)			((_t *){0})
# define	struct_offset(_t, _d)	((u64)&(struct_null(_t)->_d))

# define VEC_IMPLEMENTATION
# include "vec.h"

typedef char        *String;

#endif // _TYPES_H
