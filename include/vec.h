/**
 * vec.h | Dynamic arrays
 */

#ifndef _VEC_H
# define _VEC_H

# define	vec_decl(_type)                                             \
                                                                        \
	typedef struct _type##_vec                                          \
    {                                                                   \
        uint32_t    capacity;                                           \
        uint32_t    count;                                              \
        _type       *items;                                             \
    } _type##s

# define	_vec(_t, _c, _i)											\
	(_t) { .count = _c, .capacity = _c, items = _i }

# define    vec_count(_vec)     ((_vec).count)
# define    vec_first(_vec)     ((_vec).items)
# define    vec_last(_vec)      ((_vec).items + vec_count(_vec) - 1)
# define    vec_index(_vec, _e) ((_e) - (_vec.items))

# define	VEC_MIN_SIZE	8
# define	VEC_MAX_SIZE	65536

#endif // _VEC_H

#if defined(VEC_IMPLEMENTATION)

# include <stdlib.h>
# include <stdint.h>

# define	vec_foreach(_type, _it, _vec)                               \
                                                                        \
	for (                                                               \
        _type *_it = _vec.items;                                        \
        _it && _it <= vec_last(_vec);                                   \
        ++_it                                                           \
    )

# define    vec_foreach_rev(_type, _it, _vec)                           \
                                                                        \
    for (                                                               \
        _type *_it = vec_last(_vec);                                    \
        _it && _it >= vec_first(_vec);                                  \
        --_it                                                           \
    )

# define	vec_realloc(_vec)                                           \
                                                                        \
	do {                                                                \
		if ((_vec.capacity << 1) > VEC_MAX_SIZE)                        \
			abort();                                                    \
        uint32_t new_capacity = 0;                                      \
		if (_vec.capacity == 0)                                         \
            new_capacity = VEC_MIN_SIZE;                                \
		else if ( _vec.count + 1 == _vec.capacity)                      \
            new_capacity = (_vec.capacity << 1);                        \
        if (new_capacity)                                               \
        {                                                               \
            _vec.items = realloc(                                       \
                _vec.items,                                             \
                new_capacity * sizeof(_vec.items[0])                    \
            );                                                          \
            _vec.capacity = new_capacity;                               \
        }                                                               \
	} while (0)

# define    vec_swaplast(_vec, _i)                                      \
                                                                        \
    do {                                                                \
        uint32_t _elem_size = sizeof *_vec.items;                       \
        memcpy(&_vec.items[_i], vec_last(_vec), _elem_size);            \
    } while (0)

# define	vec_destroy(_vec)                                           \
                                                                        \
	do {                                                                \
		free(_vec.items);                                               \
	} while (0)

# define	vec_append(_vec, _e)                                        \
                                                                        \
	do {                                                                \
		vec_realloc(_vec);                                              \
		_vec.items[vec_count(_vec)++] = _e;                             \
	} while (0)

# define	vec_pop(_vec, _n)                                           \
                                                                        \
	do {                                                                \
		if (_n >= _vec.count)                                           \
			vec_count(_vec) = 0;                                        \
		else                                                            \
			vec_count(_vec) -= _n;                                      \
	} while (0)

# define    vec_delete(_vec, _i)                                        \
                                                                        \
    do {                                                                \
        if (_i >= vec_count(_vec))                                      \
            break ;                                                     \
        vec_swaplast(_vec, _i);                                         \
        vec_count(_vec)--;                                              \
    } while (0)

# define    vec_map(_type, _vec, _f)                                    \
                                                                        \
    vec_foreach(_type, _tmp, _vec)                                      \
        { _f(*_tmp); }

#endif // VEC_IMPLEMENTATION
