/**
 * vecs.h
 */

#if !defined (_GEOMETRY_H)
# define _GEOMETRY_H

# include <types.h>

# define	__align(_x)	__attribute__((aligned(_x)))

typedef struct _vec2	__align(8)	Vec2;
typedef struct _vec3				Vec3;
typedef struct _vec4	__align(16) Vec4;
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

struct _vertex
{
	Vec2	pos;
	Vec3	color;
};

#endif // _VECS_H
