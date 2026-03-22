/**
 * job.h
 */

#if !defined (_JUSTOBJ_H)
# define _JUSTOBJ_H

# include <unistd.h>
# include <stdbool.h>
# include <stdint.h>
# include <ctype.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/mman.h>

# include <arr.h>

typedef char	*String;

typedef struct	_job_file
{
	String		content;
	uint32_t	size;
}	JOBfile;

# define	JOB_FILE(_c, _s)	(JOBfile){ .content = _c, .size = _s }	
# define	JOB_NULL_FILE		JOB_FILE(NULL, 0)

typedef struct	_job_vec4
{
	float	x;
	float	y;
	float	z;
	float	w;
}	JOBvec4;

typedef struct	_job_ivec4
{
	uint32_t	x;
	uint32_t	y;
	uint32_t	z;
	uint32_t	w;
}	JOBivec4;

typedef struct	_job_face 
{
	JOBivec4	v;
	JOBivec4	vt;
	JOBivec4	vn;
}	JOBface;

typedef JOBvec4	JOBv;
typedef JOBvec4	JOBvn;
typedef JOBvec4	JOBvt;
typedef JOBface	JOBf;

arr_decl(JOBv,	JOBv_a);
arr_decl(JOBvn,	JOBvn_a);
arr_decl(JOBvt,	JOBvt_a);
arr_decl(JOBf,	JOBf_a);

typedef struct	_job_data
{
	JOBv_a	v;
	JOBvn_a	vn;
	JOBvt_a	vt;
	JOBf_a	f;
}	JOBdata;

arr_decl(JOBdata, JOBa_data);

JOBfile
job_open_model(const String filename);

void
job_close_model(JOBfile *file);

bool
job_parse_model(JOBfile *file, JOBdata *data);

#endif // _JUSTOBJ_H

#if defined(JUSTOBJ_IMPLEMENTATION)

static inline bool
_job_isspace(char c)
{
	return (c != '\n' && isspace(c));
}

static inline bool
_job_skip_whitespaces(String cursor, String *next)
{
	if (!cursor)
		return (false);
	while (*cursor && _job_isspace(*cursor))
		cursor++;
	if (next)
		*next = cursor;
	return (*cursor != 0);
}

static inline bool
_job_parse_maybe(char c, String cursor, String *next)
{
	bool	ok;

	ok = *cursor == c;
	if (ok)
		cursor++;
	if (next)
		*next = cursor;
	return (ok);
}

static inline void
_job_discard_line(String cursor, String *next)
{
	while (*cursor && *cursor != '\n')
		cursor++;
	if (next)
	{
		*next = cursor;
		if (*cursor)
			*next += 1;
	}
}

static inline String
_job_parse_word(String cursor, String *next)
{
	String	tmp;

	_job_skip_whitespaces(cursor, &cursor);
	tmp = cursor;
	while (*cursor && !isspace(*cursor))
		cursor++;
	if (next)
		*next = cursor;
	return (tmp);
}

static inline int
_job_parse_int(String str, String *end)
{
	if (!str)
	{
		if (end)
			*end = str;
		return (0);
	}

	register const char *p = str;
	register long result = 0;
	register int sign = 0;

	while (_job_isspace(*p))
		p++;
	sign = *p == '-';
	if (*p == '+' || *p == '-')
		p++;
	while (isdigit(*p))
	{
		result = result * 10 + (*p - '0');
		p++;
	}
	if (end)
		*end = (String)p;
	if (sign)
		return (-result);
	return (result);
}

static inline float
_job_parse_float(String str, String *end)
{
    String	s = str;
    float	result = 0.0f;
    float	sign = 1.0f;
    bool	found_digit = false;
    
	result = _job_parse_int(str, &str);
	_job_skip_whitespaces(s, &s);
	if (*s == '-')
	{
		result = -result;
		sign = -1.0f;
	}
	else if (result == 0 && str == s)
	{
		if (end)
			*end = str;
		return (0.0f);
	}
	if (*str == '.')
	{
		str++;
		float decimal = 0.1f;
		while (isdigit(*str))
		{
			found_digit = true;
			result += decimal * (*str - '0');
			decimal *= 0.1f;
			str++;
		}
	}
    if ((*str & ~32) == 'E' && found_digit)
	{
        str++;
		String	save = str;
		int		exponent = _job_parse_int(str, &str);

		if (exponent == 0 && str == save)
			str = save--;
		else
		{
			if (exponent > 0)
			{
                for (int i = 0; i < exponent; i++)
                    result *= 10.0f;
            }
			else if (exponent < 0) 
			{
                for (int i = 0; i < -exponent; i++)
                    result *= 0.1f;
            }
        }
    }
	if (end)
		*end = str;
    if (!found_digit)
		return (0.0f);
    return (result * sign);
}

static bool
_job_parse_vn(JOBvn *vn, String cursor, String *next)
{
	vn->x = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	vn->y = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	vn->z = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	_job_discard_line(cursor, &cursor);
	if (next)
		*next = cursor;
	return (true);
}

static bool
_job_parse_vt(JOBvt *vt, String cursor, String *next)
{
	vt->x = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	vt->y = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	vt->z = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	_job_discard_line(cursor, &cursor);
	if (next)
		*next = cursor;
	return (true);
}

static bool
_job_parse_v(JOBv *v, String cursor, String *next)
{
	v->x = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	v->y = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	v->z = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	const String save = cursor;

	v->w = _job_parse_float(cursor, &cursor);
	if (!*cursor)
		return (false);

	if (v->w == 0.0f && cursor == save)
		v->w = 1.0f;

	_job_discard_line(cursor, &cursor);
	if (next)
		*next = cursor;
	return (true);
}

static bool
_job_parse_vertex(JOBdata *data, String cursor, String *next)
{
	String	id = cursor + 1;

	switch (*id)
	{
		case 'n':
		{
			JOBvn	vn;
			if (!_job_parse_vn(&vn, id + 1, &cursor)) return (false);
			arr_append(data->vn, vn);
			break ;
		}
		case 't':
		{
			JOBvt	vt;
			if (!_job_parse_vt(&vt, id + 1, &cursor)) return (false);
			arr_append(data->vt, vt);
			break ;
		}
		case ' ':
		case '\t':
		{
			JOBv	v;
			if (!_job_parse_v(&v, id + 1, &cursor)) return (false);
			arr_append(data->v, v);
			break ;
		}
		default:
			return (false);
	}
	if (next)
		*next = cursor;
	return (true);
}

static bool
_job_parse_face_data(JOBivec4 *vd, String cursor, String *next)
{
	bool	texture;
	bool	normal;

	vd->x = _job_parse_int(cursor, &cursor);
	if (!*cursor)
		return (false);
	texture = _job_parse_maybe('/', cursor, &cursor);
	if (!*cursor)
		return (false);
	
	if (texture)
	{
		normal = _job_parse_maybe('/', cursor, &cursor);
		if (!*cursor)
			return (false);

		if (!normal)
		{
			vd->y = _job_parse_int(cursor, &cursor);
			if (!*cursor)
				return (false);
		}
		vd->z = _job_parse_int(cursor, &cursor);
		if (!*cursor)
			return (false);
	}
	if (next)
		*next = cursor;
	return (true);
}

static bool
_job_parse_face(JOBdata *data, String cursor, String *next)
{
	JOBface		f;
	JOBivec4	tmp;

	cursor += 2;

	tmp = (JOBivec4){0};
	if (!_job_parse_face_data(&tmp, cursor, &cursor)) return (false);
	f.v.x = tmp.x;
	f.vt.x = tmp.y;
	f.vn.x = tmp.z;

	tmp = (JOBivec4){0};
	if (!_job_parse_face_data(&tmp, cursor, &cursor)) return (false);
	f.v.y = tmp.x;
	f.vt.y = tmp.y;
	f.vn.y = tmp.z;

	tmp = (JOBivec4){0};
	if (!_job_parse_face_data(&tmp, cursor, &cursor)) return (false);
	f.v.z = tmp.x;
	f.vt.z = tmp.y;
	f.vn.z = tmp.z;

	arr_append(data->f, f);
	_job_discard_line(cursor, &cursor);
	if (next)
		*next = cursor;
	return (true);
}

bool
job_parse_model(JOBfile *file, JOBdata *data)
{
	String		cursor	= file->content;

	arr_reserve(data->v, 128);
	arr_reserve(data->vt, 128);
	arr_reserve(data->vn, 128);
	arr_reserve(data->f, 128);
	do 
	{
		switch (*cursor)
		{
			case '\n':
				cursor++;
				break ;
			case 'o':
			case '#':
			case 'm':
				_job_discard_line(cursor, &cursor);
				break ;
			case 'v':
				if (!_job_parse_vertex(data, cursor, &cursor))
					return (false);
				break ;
			case 'f':
				if (!_job_parse_face(data, cursor, &cursor))
					return (false);
				break ;
			case 0:
				cursor = file->content + file->size;
				break ;
			default:
				_job_discard_line(cursor, &cursor);
				break ;
		}
	}
	while (cursor < file->content + file->size);
	return (true);
}

void
*job_data_slice(JOBdata *data, const char *fmt)
{
	return (NULL);
}

JOBfile
job_open_model(const String filename)
{
	int	fd = open(filename, O_RDONLY);

	if (fd == -1)
		return (JOB_NULL_FILE);

	struct stat	st = {0};

	if (fstat(fd, &st) == -1)
	{
		close(fd);
		return (JOB_NULL_FILE);
	}

	void	*ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	madvise(ptr, st.st_size, MADV_WILLNEED);

	close(fd);
	if (ptr == MAP_FAILED)
		return (JOB_NULL_FILE);

	return (JOB_FILE(ptr, st.st_size));
}

void
job_close_model(JOBfile *file)
{
	if (file->content)
		munmap(file->content, file->size);
}

#endif // JUSTOBJ_IMPLEMENTATION
