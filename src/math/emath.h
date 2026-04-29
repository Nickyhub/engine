#pragma once
#include "math_types.h"
#include "memory/ememory.h"

f32 esin(f32 radians);
f32 ecos(f32 radians);
f32 etan(f32 radians);
f32 esqrt(f32 value);

EINLINE f32 deg_to_rad(f32 deg)
{
    return deg * (PI / 180.0f);
}

EINLINE f32 rad_to_deg(f32 rad)
{
    return rad * (180.0f / PI);
}

EINLINE vec2 vec2_zero()
{
    vec2 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    return zero;
}

EINLINE vec2 vec2_one()
{
    vec2 one;
    one.x = 1.0f;
    one.y = 1.0f;
    return one;
}

EINLINE f32 vec2_dot_product(vec2 first, vec2 second)
{
    f32 x = first.x * second.x;
    f32 y = first.y * second.y;
    return x + y;
}

EINLINE vec2 vec2_add(vec2 first, vec2 second)
{
    vec2 result;
    result.x = first.x + second.x;
    result.y = first.y + second.y;
    return result;
}

EINLINE vec2 vec2_substract(vec2 first, vec2 second)
{
    vec2 result;
    result.x = first.x - second.x;
    result.y = first.y - second.y;
    return result;
}

EINLINE vec2 vec2_invert(vec2 vector)
{
    vec2 result;
    result.x = vector.x * -1;
    result.y = vector.y * -1;
    return result;
}

EINLINE f32 vec2_length(vec2 data)
{
    return esqrt(data.x * data.x + data.y * data.y);
}

EINLINE vec2 vec2_normalized(vec2 data)
{
    vec2 result;
    f32 length = vec2_length(data);
    result.x = data.x / length;
    result.y = data.y / length;
    return result;
}

EINLINE void vec2_normalize(vec2 *data)
{
    f32 length = vec2_length(*data);
    data->x /= length;
    data->y /= length;
}

EINLINE vec3 vec3_zero()
{
    vec3 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    return zero;
}

EINLINE vec3 vec3_one()
{
    vec3 one;
    one.x = 1.0f;
    one.y = 1.0f;
    one.z = 1.0f;
    return one;
}

EINLINE f32 vec3_dot(vec3 first, vec3 second)
{
    f32 x = first.x * second.x;
    f32 y = first.y * second.y;
    f32 z = first.z * second.z;
    return x + y + z;
}

EINLINE vec3 vec3_cross(vec3 first, vec3 second)
{
    vec3 result = vec3_zero();
    result.x = (first.y * second.z) - (first.z * second.y);
    result.y = (first.z * second.x) - (first.x * second.z);
    result.z = (first.x * second.y) - (first.y * second.x);
    return result;
}

EINLINE vec3 vec3_add(vec3 first, vec3 second)
{
    vec3 result;
    result.x = first.x + second.x;
    result.y = first.y + second.y;
    result.z = first.z + second.z;
    return result;
}

EINLINE vec3 vec3_substract(vec3 first, vec3 second)
{
    vec3 result;
    result.x = first.x - second.x;
    result.y = first.y - second.y;
    result.z = first.z - second.z;
    return result;
}

EINLINE vec3 vec3_invert(vec3 vector)
{
    vec3 result;
    result.x = vector.x * -1;
    result.y = vector.y * -1;
    result.z = vector.z * -1;
    return result;
}

EINLINE f32 vec3_length(vec3 data)
{
    return esqrt(data.x * data.x + data.y * data.y + data.z * data.z);
}

EINLINE vec3 vec3_normalized(vec3 v)
{
    vec3 result;
    f32 length = vec3_length(v);
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
    return result;
}

EINLINE void vec3_normalize(vec3 *v)
{
    f32 length = vec3_length(*v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

EINLINE vec4 vec4_zero()
{
    vec4 zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    zero.w = 0.0f;
}

EINLINE vec4 vec4_one()
{
    vec4 one;
    one.x = 1.0f;
    one.y = 1.0f;
    one.z = 1.0f;
    one.w = 1.0f;
    return one;
}

EINLINE vec4 vec4_add(vec4 first, vec4 second)
{
    vec4 result;
    result.x = first.x + second.x;
    result.y = first.y + second.y;
    result.z = first.z + second.z;
    result.w = first.w + second.w;
    return result;
}

EINLINE vec4 vec4_substract(vec4 first, vec4 second)
{
    vec4 result;
    result.x = first.x - second.x;
    result.y = first.y - second.y;
    result.z = first.z - second.z;
    result.w = first.w - second.w;
    return result;
}

EINLINE vec4 vec4_invert(vec4 vector)
{
    vec4 result;
    result.x = vector.x * -1;
    result.y = vector.y * -1;
    result.z = vector.z * -1;
    result.w = vector.w * -1;
    return result;
}

EINLINE f32 vec4_length(vec4 v)
{
    return esqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

EINLINE vec4 vec4_normalized(vec4 v)
{
    vec4 result;
    f32 length = vec4_length(v);
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
    result.w = v.w / length;
    return result;
}

EINLINE void vec4_normalize(vec4 *v)
{
    f32 length = vec4_length(*v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
    v->w /= length;
}

EINLINE mat4 mat4_identity()
{
    mat4 matrix = {0};
    for (u16 i = 0; i < 4; i++)
    {
        matrix.data[i][i] = 1.0f;
    }
    return matrix;
}

EINLINE mat4 mat4_zero()
{
    mat4 zero;
    ezero_out(&zero, sizeof(mat4));
    return zero;
}

EINLINE mat4 mat4_multiply(mat4 first, mat4 second)
{
    mat4 out_matrix = mat4_identity();

    const f32 *m1_ptr = &first.data[0][0];
    const f32 *m2_ptr = &second.data[0][0];
    f32 *dst_ptr = &out_matrix.data[0][0];

    for (i32 i = 0; i < 4; ++i)
    {
        for (i32 j = 0; j < 4; ++j)
        {
            *dst_ptr =
                m1_ptr[0] * m2_ptr[0 + j] +
                m1_ptr[1] * m2_ptr[4 + j] +
                m1_ptr[2] * m2_ptr[8 + j] +
                m1_ptr[3] * m2_ptr[12 + j];
            dst_ptr++;
        }
        m1_ptr += 4;
    }
    return out_matrix;
}

EINLINE mat4 mat4_translation(vec3 translation)
{
    mat4 translate = mat4_identity();

    // [col][row]
    translate.data[3][0] = translation.x;
    translate.data[3][1] = translation.y;
    translate.data[3][2] = translation.z;

    return translate;
}

EINLINE mat4 mat4_rotate_x(mat4 matrix, f32 degrees)
{
    mat4 result = mat4_identity();
    f32 radians = deg_to_rad(degrees);

    // [col][row]
    result.data[1][1] = ecos(radians);
    result.data[2][1] = -esin(radians);
    result.data[1][2] = esin(radians);
    result.data[2][2] = ecos(radians);

    return result;
}

EINLINE mat4 mat4_rotate_y(mat4 matrix, f32 degrees)
{
    mat4 result = mat4_identity();
    f32 radians = deg_to_rad(degrees);

    // [col][row]
    result.data[0][0] = ecos(radians);
    result.data[2][0] = -esin(radians);
    result.data[0][2] = esin(radians);
    result.data[2][2] = ecos(radians);

    return result;
}

EINLINE mat4 mat4_rotate_z(mat4 matrix, f32 degrees)
{
    mat4 result = mat4_identity();
    f32 radians = deg_to_rad(degrees);

    // [col][row]
    result.data[0][0] = ecos(radians);
    result.data[1][0] = -esin(radians);
    result.data[0][2] = esin(radians);
    result.data[1][1] = ecos(radians);

    return result;
}

EINLINE mat4 mat4_rotate_xyz(mat4 matrix, f32 degrees)
{
    mat4 x = mat4_rotate_x(matrix, degrees);
    mat4 y = mat4_rotate_y(matrix, degrees);
    mat4 z = mat4_rotate_z(matrix, degrees);

    mat4 result = mat4_multiply(x, y);
    result = mat4_multiply(result, z);
    return result;
}

EINLINE mat4 mat4_scale(vec3 scale_factors)
{
    mat4 scale = mat4_identity();
    // [col][row]
    scale.data[0][0] = scale_factors.x;
    scale.data[1][1] = scale_factors.y;
    scale.data[2][2] = scale_factors.z;

    return scale;
}

EINLINE mat4 look_at(vec3 position, vec3 target_point, vec3 up)
{
    // dir is most likely the negative z-axis but a new name
    // was overkill so use dir in the matrix construction
    vec3 dir = vec3_normalized(vec3_invert(vec3_substract(target_point, position))); // look to down -Z so invert
    vec3 x_axis = vec3_normalized(vec3_cross(up, dir));
    vec3 y_axis = vec3_normalized(vec3_cross(x_axis, dir));

    // [col][row]

    mat4 view = mat4_identity();
    // First column
    view.data[0][0] = x_axis.x;
    view.data[0][1] = x_axis.y;
    view.data[0][2] = x_axis.z;

    // Second column
    view.data[1][0] = y_axis.x;
    view.data[1][1] = y_axis.y;
    view.data[1][2] = y_axis.z;

    // Third column
    view.data[2][0] = dir.x;
    view.data[2][1] = dir.y;
    view.data[2][2] = dir.z;

    // Fourth column (translation in camera space)
    view.data[3][0] = -vec3_dot(x_axis, position);
    view.data[3][1] = -vec3_dot(y_axis, position);
    view.data[3][2] = -vec3_dot(dir, position);

    return view;
}

EINLINE mat4 mat4_transpose(mat4 matrix)
{
    mat4 result = mat4_zero();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.data[i][j] = matrix.data[j][i];
        }
    }
    return result;
}

EINLINE mat4 mat4_inverse_view_matrix(mat4 view_matrix)
{
    return mat4_transpose(view_matrix);
}

EINLINE mat4 mat4_inverse(mat4 matrix)
{
    const f32 *m = &matrix.data[0][0];

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    mat4 out_matrix;
    f32 *o = &out_matrix.data[0][0];

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return out_matrix;
}

EINLINE mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far)
{
    mat4 mat4_perspective = mat4_zero();

    f32 tan_half_fov = etan(deg_to_rad(fov / 2.0f));
    f32 f = 1 / tan_half_fov;

    // [col][row]
    mat4_perspective.data[0][0] = f / aspect_ratio;
    mat4_perspective.data[1][1] = f;
    mat4_perspective.data[2][2] = far / (near - far);
    mat4_perspective.data[3][2] = (far * near) / (near - far); // col 3, row 2
    mat4_perspective.data[2][3] = -1.0f;                       // col 2, row 3
    return mat4_perspective;
}