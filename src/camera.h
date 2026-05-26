#ifndef CAMERA_H_
#define CAMERA_H_

#include <cglm/cglm.h>

typedef enum CameraMode {
    CAMERA_FIRST_PERSON,
    CAMERA_THIRD_PERSON,
    CAMERA_FREE,
} CameraMode;

typedef enum CameraType {
    CAMERA_PERSPECTIVE,
    CAMERA_ORTHOGONAL,
} CameraType;

typedef struct camera {
    vec3 position;
    vec3 target;
    vec3 up;
    float fov;
    float aspect;
    float near;
    float far;
    CameraMode mode;
    CameraType type;
} camera;

void camera_local_axes(const camera* camera, vec3 x, vec3 y, vec3 z);
void camera_local_axes_normalized(const camera* camera, vec3 x, vec3 y, vec3 z);
void camera_yaw(camera* camera, float angle);
void camera_pitch(camera* camera, float angle);
void camera_roll(camera* camera, float angle);
void camera_update(camera* camera, vec2 mouse_delta);
void camera_get_projection(const camera* camera, mat4 projection);
void camera_get_view(const camera* camera, mat4 view);
void camera_get_transform(const camera* camera, mat4 transform);

#endif // CAMERA_H_
