#include "camera.h"

void camera_local_axes(const camera* camera, vec3 x, vec3 y, vec3 z)
{
    glm_vec3_copy((float*)camera->up, y);
    glm_vec3_sub((float*)camera->target, (float*)camera->position, z);
    glm_vec3_cross(z, y, x);
}

void camera_local_axes_normalized(const camera* camera, vec3 x, vec3 y, vec3 z)
{
    glm_vec3_normalize_to((float*)camera->up, y);
    glm_vec3_sub((float*)camera->target, (float*)camera->position, z);
    glm_vec3_cross(z, y, x);

    glm_vec3_normalize(x);
    glm_vec3_normalize(z);
}

void camera_yaw(camera* camera, const float angle)
{
    vec3 direction = { 0 };

    glm_vec3_sub(camera->target, camera->position, direction);

    glm_vec3_rotate(direction, angle, camera->up);

    const bool rotate_target = CAMERA_THIRD_PERSON == camera->mode;

    if (rotate_target)
    {
        glm_vec3_sub(camera->target, direction, camera->position);
    }
    else
    {
        glm_vec3_add(camera->position, direction, camera->target);
    }
}

void camera_pitch(camera* camera, float angle)
{
    vec3 x = { 0 };
    vec3 y = { 0 };
    vec3 z = { 0 };

    camera_local_axes(camera, x, y, z);

    vec3 direction = { 0 };

    glm_vec3_sub(camera->target, camera->position, direction);

    const bool clamp = (CAMERA_FREE == camera->mode) || (CAMERA_FIRST_PERSON == camera->mode) || (CAMERA_THIRD_PERSON == camera->mode);
    if (clamp)
    {
        const float max_angle_up = glm_vec3_angle(camera->up, direction) - (float)1e-6;
        angle = glm_min(angle, max_angle_up);

        vec3 down = { 0 };
        glm_vec3_negate_to(camera->up, down);

        const float max_angle_down = -(glm_vec3_angle(down, direction) - (float)1e-6);
        angle = glm_max(angle, max_angle_down);
    }

    glm_vec3_rotate(direction, angle, x);

    const bool rotate_target = CAMERA_THIRD_PERSON == camera->mode;

    if (rotate_target)
    {
        glm_vec3_sub(camera->target, direction, camera->position);
    }
    else
    {
        glm_vec3_add(camera->position, direction, camera->target);
    }
}

void camera_roll(camera* camera, const float angle)
{
    vec3 z = { 0 };
    glm_vec3_sub(camera->target, camera->position, z);

    glm_vec3_rotate(camera->up, angle, z);
}

void camera_update(camera* camera, vec2 mouse_delta)
{
    camera_yaw(camera, mouse_delta[0]);
    camera_pitch(camera, mouse_delta[1]);
}

void camera_get_transform(const camera* camera, mat4 transform)
{
    mat4 projection = { 0 };
    glm_perspective(
        camera->fov,
        camera->aspect,
        camera->near,
        camera->far,
        projection
    );

    mat4 view = { 0 };
    glm_lookat(
        (float*)camera->position,
        (float*)camera->target,
        (float*)camera->up,
        view
    );

    glm_mul(projection, view, transform);
}
