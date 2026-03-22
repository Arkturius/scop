/**
 * types.h
 */

#ifndef _TYPES_H
# define _TYPES_H

# include <stdbool.h>
# include <stdint.h>
# include <string.h>
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

# define    shift_args(ac, av)  (ac--, *av++)
# define    unused(_x)          (void)(_x)
# define	alignas(_x)			__attribute__((aligned(_x)))

# define    info(_s, ...)                                              \
    dprintf(2, "[INFO]   | %s:%s:%d "_s"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);

# define    error(_s, ...)                                            \
    dprintf(2, "[ERROR]  | %s:%s:%d "_s"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);

# define    warning(_s, ...)                                          \
    dprintf(2, "[WARNING]| %s:%s:%d "_s"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);

# define    _concat(_a, _b)			_a##_b
# define    concat(_a, _b)			_concat(_a, _b)

# define    stringify(_x)			#_x

# define	array_len(_arr)			(sizeof(_arr)/sizeof(_arr[0]))

# define	min(_a, _b)				((_a) < (_b) ? (_a) : (_b))
# define	max(_a, _b)				((_a) > (_b) ? (_a) : (_b))

# define	clamp(_v, _min, _max)	min(max(_v, _min), _max)

# define	struct_null(_t)			((_t *){0})
# define	struct_offset(_t, _d)	((u64)&(struct_null(_t)->_d))

# define ARR_IMPLEMENTATION
# include <arr.h>

typedef char	*String;

#endif // _TYPES_H
