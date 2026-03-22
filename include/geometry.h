/**
 * vecs.h
 */

#if !defined (_GEOMETRY_H)
# define _GEOMETRY_H

# include <types.h>
# include <math.h>

# define	__align(_x)	__attribute__((aligned(_x)))

typedef float	__attribute__((vector_size(4)))	fVec;

typedef struct _vec2	__align(8)	Vec2;
typedef struct _vec3				Vec3;
typedef struct _vec4	__align(16) Vec4;
typedef struct _mat4x4	__align(32)	Mat4x4;
typedef struct _vec2	__align(8)	Point2D;
typedef struct _vec4	__align(16)	Point3D;
typedef struct _vec3				ColorF;
typedef struct _vertex				Vertex;

# define	vec4(_x, _y, _z, _w)	(Vec4){.x = _x, .y = _y, .z = _z, .w = _w}
# define	vec3(_x, _y, _z)		(Vec3){.x = _x, .y = _y, .z = _z}
# define	vec2(_x, _y)			(Vec2){.x = _x, .y = _y}

struct _vec2
{
	f32	x;
	f32	y;
};

struct _vec3
{
	f32 x;
	f32	y;
	f32	z;
};

struct _vec4
{
	f32	x;
	f32	y;
	f32	z;
	f32	w;
};

typedef struct
{
    float m[16];
}	Mat4;

# define	M4(_m, _c, _r) (_m).m[(_c) * 4 + (_r)]

struct _vertex
{
	Vec3	pos;
	Vec3	color;
	Vec2	tex;
};

Mat4
mat4_identity(void);

Mat4
mat4_rotate(float angleRad, Vec3 axis);

Mat4
mat4_lookat(Vec3 eye, Vec3 center, Vec3 up);

Mat4
mat4_perspective(float fovy, float aspect, float zNear, float zFar);

#endif // _VECS_H
