/**
 * arr.h | Dynamic arrays
 */

#ifndef _ARR_H
# define _ARR_H

# include <stdint.h>
# include <stdlib.h>

# define	arr_decl(_type, _name)											\
																			\
	typedef struct _name##_arr												\
	{																		\
		uint32_t	capacity;												\
		uint32_t	count;													\
		_type	   *items;													\
	} _name

# define	_arr(_t, _c, _i)												\
	(_t) { .count = _c, .capacity = _c, items = _i }

# define	arr_count(_arr)		((_arr).count)
# define	arr_first(_arr)		((_arr).items)
# define	arr_size(_arr)		((_arr).capacity)
# define	arr_last(_arr)		((_arr).items + arr_count(_arr) - 1)
# define	arr_index(_arr, _e) ((_e) - (_arr.items))

# define	ARR_MIN_SIZE	8

# ifndef ARR_MAX_SIZE
#  define	ARR_MAX_SIZE	(1 << 28)
# endif

# define	arr_foreach(_type, _it, _arr)									\
																			\
	for (																	\
		_type *_it = arr_first(_arr);										\
		_it && _it <= arr_last(_arr);										\
		++_it																\
	)

# define	arr_foreach_rev(_type, _it, _arr)								\
																			\
	for (																	\
		_type *_it = arr_last(_arr);										\
		arr_first(_arr) && _it >= arr_first(_arr);							\
		--_it																\
	)

# define	arr_realloc(_arr, _n)											\
																			\
	do {																	\
		if ((arr_size(_arr) << 1) > ARR_MAX_SIZE)							\
			abort();														\
		uint32_t new_capacity = 0;											\
		if (arr_size(_arr) == 0)											\
			new_capacity = _n ? _n : ARR_MIN_SIZE;							\
		else if (arr_count(_arr) + 1 == arr_size(_arr))						\
			new_capacity = (_arr.capacity << 1);							\
		if (new_capacity)													\
		{																	\
			_arr.items = realloc(											\
				arr_first(_arr),											\
				new_capacity * sizeof *arr_first(_arr)						\
			);																\
			arr_size(_arr) = new_capacity;									\
		}																	\
	} while (0)

# define	arr_reserve(_arr, _n)											\
																			\
	do {																	\
		arr_realloc(_arr, _n);												\
		arr_count(_arr) = 0;												\
	} while (0)

# define	arr_swaplast(_arr, _i)											\
																			\
	do {																	\
		uint32_t _elem_size = sizeof *arr_first(_arr);						\
		memcpy(&_arr.items[_i], arr_last(_arr), _elem_size);				\
	} while (0)

# define	arr_destroy(_arr)												\
																			\
	do {																	\
		free(arr_first(_arr));												\
		arr_first(_arr) = NULL;												\
		arr_count(_arr) = 0;												\
		arr_size(_arr) = 0;													\
	} while (0)

# define	arr_append(_arr, _e)											\
																			\
	do {																	\
		arr_realloc(_arr, 0);												\
		_arr.items[arr_count(_arr)++] = _e;									\
	} while (0)

# define	arr_pop(_arr, _n)												\
																			\
	do {																	\
		if (_n >= _arr.count)												\
			arr_count(_arr) = 0;											\
		else																\
			arr_count(_arr) -= _n;											\
	} while (0)

# define	arr_delete(_arr, _i)											\
																			\
	do {																	\
		if (_i >= arr_count(_arr))											\
			break ;															\
		arr_swaplast(_arr, _i);												\
		arr_count(_arr)--;													\
	} while (0)

# define	arr_map_custom(_type, _it, _arr, _f, _args)						\
																			\
	arr_foreach(_type, _it, _arr)											\
	{																		\
		_f _args;															\
	}

# define	arr_map(_type, _arr, _f)										\
																			\
	arr_map_custom(_type, _tmp, _arr, _f, (*_tmp))

#endif // _ARR_H
