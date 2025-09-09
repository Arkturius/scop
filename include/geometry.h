/**
 * vecs.h
 */

#if !defined (_GEOMETRY_H)
# define _GEOMETRY_H

# include <types.h>

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

struct _mat4x4
{
	f32	x1, x2, x3, x4;
	f32	y1, y2, y3, y4;
	f32	z1, z2, z3, z4;
	f32	w1, w2, w3, w4;
};

struct _vertex
{
	Vec2	pos;
	Vec3	color;
};

#endif // _VECS_H
