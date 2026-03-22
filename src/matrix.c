/**
 * matrix.c
 */

#include <geometry.h>

#if 1
Mat4
mat4_identity(void)
{
    Mat4 out = {0};

    M4(out,0,0) = 1.0f;
    M4(out,1,1) = 1.0f;
    M4(out,2,2) = 1.0f;
    M4(out,3,3) = 1.0f;

	return (out);
}

Mat4
mat4_perspective(float fovy, float aspect, float zNear, float zFar)
{
    float tanHalfFovy = tanf(fovy * 0.5f);

    Mat4 out = {0};

    M4(out, 0, 0) = 1.0f / (aspect * tanHalfFovy);
    M4(out, 1, 1) = 1.0f / (tanHalfFovy);
    M4(out, 2, 2) = zFar / (zNear - zFar);
    M4(out, 2, 3) = -1.0f;
    M4(out, 3, 2) = -(zFar * zNear) / (zFar - zNear);

    return out;
}

static inline Vec3
vec3_sub(Vec3 a, Vec3 b)
{
	return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline Vec3
vec3_cross(Vec3 a, Vec3 b)
{ 
    return (Vec3)
	{
		.x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
	}; 
}

static inline float
vec3_dot(Vec3 a, Vec3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

static inline Vec3
vec3_norm(Vec3 v)
{ 
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	if (len <= 1e-4)
		return (Vec3) { 0.0, 0.0, 0.0 };
    return (Vec3) { v.x / len, v.y / len, v.z / len }; 
}

Mat4
mat4_lookat(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = vec3_norm(vec3_sub(center, eye));
    Vec3 s = vec3_norm(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 out = mat4_identity();

    M4(out,0,0) =  s.x; 
	M4(out,1,0) =  s.y; 
	M4(out,2,0) =  s.z;
    M4(out,0,1) =  u.x;
	M4(out,1,1) =  u.y;
	M4(out,2,1) =  u.z;
    M4(out,0,2) = -f.x;
	M4(out,1,2) = -f.y;
	M4(out,2,2) = -f.z;
    M4(out,3,0) = -vec3_dot(s, eye);
    M4(out,3,1) = -vec3_dot(u, eye);
    M4(out,3,2) =  vec3_dot(f, eye);

    return out;
}

Mat4
mat4_rotate(float angleRad, Vec3 axis)
{
    axis = vec3_norm(axis);
    float c = cosf(angleRad);
    float s = sinf(angleRad);
    float ic = 1.0f - c;
	float x = axis.x, y = axis.y, z = axis.z;

    Mat4 out = mat4_identity();

    M4(out, 0, 0) = c + x * x * ic;
    M4(out, 0, 1) = y * x * ic + z * s;
    M4(out, 0, 2) = z * x * ic - y * s;

    M4(out, 1, 0) = x * y * ic - z * s;
    M4(out, 1, 1) = c + y * y * ic;
    M4(out, 1, 2) = z * y * ic + x * s;

    M4(out, 2, 0) = x * z * ic + y * s;
    M4(out, 2, 1) = y * z * ic - x * s;
    M4(out, 2, 2) = c + z * z * ic;

    return out;
}
#endif
