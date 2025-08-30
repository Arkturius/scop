/**
 * singleton.h
 */

#if !defined (_SINGLETON_H)
# define _SINGLETON_H

# if !defined (SINGLETON_HOOK_MALLOC)
#  define	SINGLETON_HOOK_MALLOC	malloc
# endif
# if !defined (SINGLETON_HOOK_FREE)
#  define	SINGLETON_HOOK_FREE		free
# endif

# if !defined (SINGLETON_HEAP)
#  define	singleton(_type, ...)											\
																			\
	_type	*SINGLETON_GET_INSTANCE(_type, ##__VA_ARGS__)(void)				\
	{																		\
		static _type	instance = {0};										\
																			\
		return (&instance);													\
	}
# else
#  define	singleton(_type, ...)											\
																			\
	_type																	\
	*SINGLETON_GET_INSTANCE(_type, ##__VA_ARGS__)(void)						\
	{																		\
		static _type	*instance;											\
																			\
		if (!instance)														\
			instance = SINGLETON_HOOK_MALLOC(sizeof(_type));				\
		if (instance)														\
			memset(instance, 0, sizeof(_type))								\
		return (instance);													\
	}																		\
																			\
	__attribute__((destructor)) void										\
	SINGLETON_DEL_INSTANCE(_type, ##__VA_ARGS__)(void)						\
	{																		\
		_type	*instance;													\
																			\
		instance = SINGLETON_GET_INSTANCE(_type. ##__VA_ARGS__)();			\
		SINGLETON_HOOK_FREE(instance);										\
	}
#endif

# define	singleton_decl(_type, ...)										\
	_type	*SINGLETON_GET_INSTANCE(_type, ##__VA_ARGS__)(void)

# define	SINGLETON_GET_INSTANCE(...)										\
	PICK_3(__VA_ARGS__, GET_INSTANCE_NAME, GET_INSTANCE_TYPE)(__VA_ARGS__)

# define	SINGLETON_DEL_INSTANCE(...)										\
	PICK_3(__VA_ARGS__, DEL_INSTANCE_NAME, DEL_INSTANCE_TYPE)(__VA_ARGS__)

# define	GET_INSTANCE_NAME(_type,_name)	_name##_getinstance
# define	GET_INSTANCE_TYPE(_type)		_type##_getinstance

# define	DEL_INSTANCE_NAME(_type,_name)	_name##_delinstance
# define	DEL_INSTANCE_TYPE(_type)		_type##_delinstance

# define	PICK_3(_1, _2, _3, ...)	_3

#endif // _SINGLETON_H
