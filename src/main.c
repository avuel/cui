#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <nuklear_sdl_renderer.h>

#include "camera.h"

#define WINDOW_TITLE  "cui"
#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define COLOR_RED    "\033[0;31m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_WHITE  "\033[38;5;252m"
#define COLOR_YELLOW "\033[38;5;220m"
#define COLOR_ORANGE "\033[38;5;208m"
#define COLOR_RESET  "\033[0m"

#define log_(lvl, typ, col, fmt, ...)                                                                                                                                                             \
do {                                                                                                                                                                                              \
    SDL_Time ns__ = { 0 };                                                                                                                                                                        \
    if (SDL_GetCurrentTime(&ns__))                                                                                                                                                                \
    {                                                                                                                                                                                             \
        SDL_DateTime dt__ = { 0 };                                                                                                                                                                \
        SDL_TimeToDateTime(ns__, &dt__, true);                                                                                                                                                    \
        const int ms__ = SDL_NS_TO_MS(dt__.nanosecond);                                                                                                                                           \
                                                                                                                                                                                                  \
        const char time__[] = {                                                                                                                                                                   \
            (char)('0' +  dt__.month / 10),                                                                                                                                                       \
            (char)('0' +  dt__.month % 10),                                                                                                                                                       \
            '-',                                                                                                                                                                                  \
            (char)('0' +  dt__.day / 10),                                                                                                                                                         \
            (char)('0' +  dt__.day % 10),                                                                                                                                                         \
            '-',                                                                                                                                                                                  \
            (char)('0' + dt__.year / 1000),                                                                                                                                                       \
            (char)('0' + dt__.year % 1000 / 100),                                                                                                                                                 \
            (char)('0' + dt__.year %  100 / 10),                                                                                                                                                  \
            (char)('0' + dt__.year %   10),                                                                                                                                                       \
            ' ',                                                                                                                                                                                  \
            (char)('0' +  dt__.hour / 10),                                                                                                                                                        \
            (char)('0' +  dt__.hour % 10),                                                                                                                                                        \
            ':',                                                                                                                                                                                  \
            (char)('0' +  dt__.minute / 10),                                                                                                                                                      \
            (char)('0' +  dt__.minute % 10),                                                                                                                                                      \
            ':',                                                                                                                                                                                  \
            (char)('0' +  dt__.second / 10),                                                                                                                                                      \
            (char)('0' +  dt__.second % 10),                                                                                                                                                      \
            '.',                                                                                                                                                                                  \
            (char)('0' + ms__ / 100),                                                                                                                                                             \
            (char)('0' + ms__ % 100 / 10),                                                                                                                                                        \
            (char)('0' + ms__ % 10),                                                                                                                                                              \
            '\0',                                                                                                                                                                                 \
        };                                                                                                                                                                                        \
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, lvl, col "[%s] " typ " [%" PRIu64 "] %s:%d: " fmt COLOR_RESET, time__, SDL_GetCurrentThreadID(), __FILE__, __LINE__, ##__VA_ARGS__);              \
    }                                                                                                                                                                                             \
    else                                                                                                                                                                                          \
    {                                                                                                                                                                                             \
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, lvl, col "[-----INVALID--TIME-----] " typ " [%" PRIu64 "] %s:%d: " fmt COLOR_RESET, SDL_GetCurrentThreadID(), __FILE__, __LINE__, ##__VA_ARGS__); \
    }                                                                                                                                                                                             \
} while (0)

#define log_dbg(fmt, ...) log_(SDL_LOG_PRIORITY_DEBUG,    " DEBUG ", COLOR_CYAN,   fmt, ##__VA_ARGS__)
#define log_msg(fmt, ...) log_(SDL_LOG_PRIORITY_INFO,     "MESSAGE", COLOR_WHITE,  fmt, ##__VA_ARGS__)
#define log_wrn(fmt, ...) log_(SDL_LOG_PRIORITY_WARN,     "WARNING", COLOR_YELLOW, fmt, ##__VA_ARGS__)
#define log_err(fmt, ...) log_(SDL_LOG_PRIORITY_ERROR,    " ERROR ", COLOR_ORANGE, fmt, ##__VA_ARGS__)
#define log_fat(fmt, ...) log_(SDL_LOG_PRIORITY_CRITICAL, " FATAL ", COLOR_RED,    fmt, ##__VA_ARGS__)

struct nk_sdl_app {
    SDL_GPUDevice* device;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUBuffer* buffer_vertex;
    SDL_GPUBuffer* buffer_index;
    struct nk_context* ctx;
    struct nk_colorf bg;
    enum nk_anti_aliasing AA;

    SDL_Mutex* mouse_mutex;
    vec2 mouse_motion;

    camera camera;
};

typedef struct position {
    float x, y, z;
} position;

typedef struct color {
    Uint8 r, g, b, a;
} color;

typedef struct vertex_position_color {
    position position;
    color color;
} vertex_position_color;

// #define NUKLEAR

SDL_AppResult SDL_AppInit(void** appstate, const int argc, char* argv[])
{
    NK_UNUSED(argc);
    NK_UNUSED(argv);

    // set log priority
    SDL_SetLogPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);

    // remove sdl log prefixes
    for (int i = SDL_LOG_PRIORITY_INVALID + 1; i < SDL_LOG_PRIORITY_COUNT; ++i)
    {
        if (!SDL_SetLogPriorityPrefix(i, ""))
        {
            log_fat("failed to strip prefix: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        log_fat("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    struct nk_sdl_app* app = SDL_malloc(sizeof(*app));

    if (NULL == app)
    {
        log_fat("SDL_malloc failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#ifdef _WIN32
    const SDL_GPUShaderFormat gpu_format = SDL_GPU_SHADERFORMAT_DXIL;
    const char* const gpu_driver = "direct3d12";
#else
    const SDL_GPUShaderFormat gpu_format = SDL_GPU_SHADERFORMAT_SPIRV;
    const char* const driver = "vulkan";
#endif
    app->device = SDL_CreateGPUDevice(gpu_format, true, gpu_driver);
    if (NULL == app->device)
    {
        log_fat("create gpu device failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    app->window = SDL_CreateWindow("cui", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIDDEN);
    if (NULL == app->window)
    {
        log_fat("create window failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *appstate = app;

    if (!SDL_ClaimWindowForGPUDevice(app->device, app->window))
    {
        log_fat("claim window for gpu device failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // read vertex shader code
    size_t shader_vertex_code_size = 0;
    void* shader_vertex_code = SDL_LoadFile("shaders/vertex.dxil", &shader_vertex_code_size);
    if (NULL == shader_vertex_code)
    {
        log_fat("vertex shader code load failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    log_dbg("vertex shader size: %zu", shader_vertex_code_size);

    // upload vertex shader program
    const SDL_GPUShaderCreateInfo shader_vertex_create_info = {
        .code_size = shader_vertex_code_size,
        .code = shader_vertex_code,
        .entrypoint = "vs_main",
        .format = gpu_format,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1,
        .props = 0,
    };

    SDL_GPUShader* gpu_shader_vertex = SDL_CreateGPUShader(app->device, &shader_vertex_create_info);
    SDL_free(shader_vertex_code);
    if (NULL == gpu_shader_vertex)
    {
        log_fat("create gpu shader vertex failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // read pixel shader code
    size_t shader_pixel_code_size = 0;
    void* shader_pixel_code = SDL_LoadFile("shaders/pixel.dxil", &shader_pixel_code_size);
    if (NULL == shader_pixel_code)
    {
        log_fat("vertex shader code load failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    log_dbg("pixel shader size: %zu", shader_pixel_code_size);

    // upload pixel shader program
    const SDL_GPUShaderCreateInfo shader_pixel_create_info = {
        .code_size = shader_pixel_code_size,
        .code = shader_pixel_code,
        .entrypoint = "ps_main",
        .format = gpu_format,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0,
        .props = 0,
    };

    SDL_GPUShader* gpu_shader_pixel = SDL_CreateGPUShader(app->device, &shader_pixel_create_info);
    SDL_free(shader_pixel_code);

    if (NULL == gpu_shader_pixel)
    {
        log_fat("create gpu shader pixel failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
        .vertex_shader   = gpu_shader_vertex,
        .fragment_shader = gpu_shader_pixel,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (const SDL_GPUVertexBufferDescription[]) {
                { .slot = 0, .pitch = sizeof(vertex_position_color), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0 },
            },
            .num_vertex_attributes = 2,
            .vertex_attributes = (const SDL_GPUVertexAttribute[]) {
                    { .location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,      .offset = 0,                },
                    { .location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, .offset = sizeof(position), },
            },
        },
        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
            .depth_bias_constant_factor = 0.0f,
            .depth_bias_clamp = 0.0f,
            .depth_bias_slope_factor = 0.0f,
            .enable_depth_bias = false,
            .enable_depth_clip = false,
        },
        .multisample_state = {
            .sample_count = 0,
            .sample_mask  = 0,
            .enable_mask  = false,
            .enable_alpha_to_coverage = false,
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_INVALID,
            .back_stencil_state = {
                .fail_op       = SDL_GPU_STENCILOP_INVALID,
                .pass_op       = SDL_GPU_STENCILOP_INVALID,
                .depth_fail_op = SDL_GPU_STENCILOP_INVALID,
                .compare_op    = SDL_GPU_COMPAREOP_INVALID,
            },
            .front_stencil_state = {
                .fail_op       = SDL_GPU_STENCILOP_INVALID,
                .pass_op       = SDL_GPU_STENCILOP_INVALID,
                .depth_fail_op = SDL_GPU_STENCILOP_INVALID,
                .compare_op    = SDL_GPU_COMPAREOP_INVALID,
            },
            .compare_mask = 0,
            .write_mask   = 0,
            .enable_depth_test   = false,
            .enable_depth_write  = false,
            .enable_stencil_test = false,
        },
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (const SDL_GPUColorTargetDescription[]) {
                { .format = SDL_GetGPUSwapchainTextureFormat(app->device, app->window), },
            },
            .has_depth_stencil_target = false,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_INVALID,
        },
    };

    // create graphics pipeline
    app->pipeline = SDL_CreateGPUGraphicsPipeline(app->device, &graphics_pipeline_create_info);

    // free shaders after creating pipeline
    SDL_ReleaseGPUShader(app->device, gpu_shader_vertex);
    SDL_ReleaseGPUShader(app->device, gpu_shader_pixel);

    if (NULL == app->pipeline)
    {
        log_fat("create gpu graphics pipeline failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // create buffers for cube and upload data
    {
        // create vertex buffer
        const SDL_GPUBufferCreateInfo buffer_vertex_create_info = {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size  = 24 * sizeof(vertex_position_color),
            .props = 0,
        };
        app->buffer_vertex = SDL_CreateGPUBuffer(app->device, &buffer_vertex_create_info);
        if (NULL == app->buffer_vertex)
        {
            log_fat("create vertex buffer failed %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // create index buffer
        const SDL_GPUBufferCreateInfo buffer_index_create_info = {
            .usage = SDL_GPU_BUFFERUSAGE_INDEX,
            .size  = 36 * sizeof(Uint16),
            .props = 0,
        };
        app->buffer_index = SDL_CreateGPUBuffer(app->device, &buffer_index_create_info);
        if (NULL == app->buffer_index)
        {
            log_fat("create index buffer failed %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // create gpu transfer buffer
        const SDL_GPUTransferBufferCreateInfo gpu_transfer_buffer_create_info = {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size  = buffer_vertex_create_info.size + buffer_index_create_info.size,
            .props = 0
        };
        SDL_GPUTransferBuffer* gpu_transfer_buffer = SDL_CreateGPUTransferBuffer(app->device, &gpu_transfer_buffer_create_info);
        if (NULL == gpu_transfer_buffer)
        {
            log_fat("create gpu transfer buffer failed %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        vertex_position_color* gpu_transfer_buffer_data = SDL_MapGPUTransferBuffer(app->device, gpu_transfer_buffer, false);
        if (NULL == gpu_transfer_buffer_data)
        {
            log_fat("map gpu transfer buffer failed %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        vertex_position_color* vertex_data = gpu_transfer_buffer_data;
        Uint16 vertex_count = 0;

        // front
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b =   0, .a = 255} };

        // right
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r = 255, .g = 255, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r = 255, .g = 255, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g = 255, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r = 255, .g = 255, .b =   0, .a = 255} };

        // top
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r = 255, .g = 255, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r = 255, .g = 255, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g = 255, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g = 255, .b = 255, .a = 255} };

        // back
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b = 255, .a = 255} };

        // left
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y =  2.0f, .z = -2.0f, }, .color = { .r = 255, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r = 255, .g =   0, .b = 255, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r = 255, .g =   0, .b = 255, .a = 255} };

        // bottom
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r =   0, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z =  2.0f, }, .color = { .r =   0, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x =  2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b =   0, .a = 255} };
        vertex_data[vertex_count++]  = (vertex_position_color) { .position = { .x = -2.0f, .y = -2.0f, .z = -2.0f, }, .color = { .r =   0, .g =   0, .b =   0, .a = 255} };


        Uint16* index_data = (Uint16*)(&vertex_data[vertex_count]);
        Uint16 indices[36] = {
            // front
             0,  1,  2,
             3,  2,  1,

            // right
             4,  5,  6,
             7,  6,  5,

            // top
             8,  9, 10,
            11, 10,  9,

            // back
            12, 13, 14,
            15, 14, 13,

            // left
            16, 17, 18,
            19, 18, 17,

            // bottom
            20, 21, 22,
            23, 22, 21,
        };
        SDL_memcpy(index_data, indices, sizeof(indices));

        SDL_UnmapGPUTransferBuffer(app->device, gpu_transfer_buffer);

        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app->device);
        if (NULL == command_buffer)
        {
            log_fat("acquire command buffer failed %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
        {
            // copy vertex data
            const SDL_GPUTransferBufferLocation gpu_transfer_buffer_location_vertex = {
                .transfer_buffer = gpu_transfer_buffer,
                .offset = 0,
            };
            const SDL_GPUBufferRegion gpu_transfer_buffer_region_vertex = {
                .buffer = app->buffer_vertex,
                .offset = 0,
                .size   = buffer_vertex_create_info.size,
            };
            SDL_UploadToGPUBuffer(copy_pass, &gpu_transfer_buffer_location_vertex, &gpu_transfer_buffer_region_vertex, false);

            // copy index data
            const SDL_GPUTransferBufferLocation gpu_transfer_buffer_location_index = {
                .transfer_buffer = gpu_transfer_buffer,
                .offset          = buffer_vertex_create_info.size,
            };
            const SDL_GPUBufferRegion gpu_transfer_buffer_region_index = {
                .buffer = app->buffer_index,
                .offset = 0,
                .size   = buffer_index_create_info.size,
            };
            SDL_UploadToGPUBuffer(copy_pass, &gpu_transfer_buffer_location_index, &gpu_transfer_buffer_region_index, false);
        }
        SDL_EndGPUCopyPass(copy_pass);

        // submit command buffer
        const bool success = SDL_SubmitGPUCommandBuffer(command_buffer);

        SDL_ReleaseGPUTransferBuffer(app->device, gpu_transfer_buffer);

        if (!success)
        {
            log_fat("submit command buffer failed: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    app->bg.r = 0.10f;
    app->bg.g = 0.18f;
    app->bg.b = 0.24f;
    app->bg.a = 1.0f;

#ifdef NUKLEAR
    float font_scale = 1.0f;
    {
        /* This scaling logic was kept simple for the demo purpose.
         * On some platforms, this might not be the exact scale
         * that you want to use. For more information, see:
         * https://wiki.libsdl.org/SDL3/README-highdpi */
        const float scale = SDL_GetWindowDisplayScale(app->window);
        SDL_SetRenderScale(app->renderer, scale, scale);
        font_scale = scale;
    }

    app->ctx = nk_sdl_init(app->window, app->renderer, nk_sdl_allocator());

    /* set up the font atlas and add desired font; note that font sizes are
     * multiplied by font_scale to produce better results at higher DPIs */
    struct nk_font* font = NULL;
    struct nk_font_atlas* atlas = nk_sdl_font_stash_begin(app->ctx);
    {
        const struct nk_font_config config = nk_font_config(0);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
    }
    nk_sdl_font_stash_end(app->ctx);

    /* this hack makes the font appear to be scaled down to the desired
     * size and is only necessary when font_scale > 1 */
    if (NULL == font)
    {
        log_fat("font is null");
        return SDL_APP_FAILURE;
    }

    font->handle.height /= font_scale;
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    nk_style_set_font(app->ctx, &font->handle);

    app->AA = NK_ANTI_ALIASING_ON;

    nk_input_begin(app->ctx);
#endif

    app->camera = (camera) {
        .position = { 0.0f, 0.0f, 10.0f },
        .target   = { 0.0f, 0.0f, 0.0f },
        .up       = { 0.0f, 1.0f, 0.0f },
        .fov      = 60.0f * SDL_PI_F / 180.0f,
        .aspect   = (float)((double)WINDOW_WIDTH / (double)WINDOW_HEIGHT),
        .near     = 20.0f,
        .far      = 60.0f,
        .mode     = CAMERA_THIRD_PERSON,
        .type     = CAMERA_PERSPECTIVE,
    };

    glm_vec2_zero(app->mouse_motion);
    app->mouse_mutex = SDL_CreateMutex();

    if (NULL == app->mouse_mutex)
    {
        log_fat("create mutex failed %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    *appstate = app;

    if (!SDL_ShowWindow(app->window))
    {
        log_fat("SDL_ShowWindow failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event)
{
    struct nk_sdl_app* app = appstate;

    static bool middle_down = false;
    static vec2 mouse_position = { 0 };

    switch (event->type)
    {
        case SDL_EVENT_QUIT:
        {
            return SDL_APP_SUCCESS;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            if (SDL_BUTTON_MIDDLE == event->button.button)
            {
                middle_down = true;
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if (SDL_BUTTON_MIDDLE == event->button.button)
            {
                middle_down = false;
            }
            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
            if (middle_down)
            {
                SDL_LockMutex(app->mouse_mutex);
                {
                    app->mouse_motion[0] = app->mouse_motion[0] - event->motion.xrel;
                    app->mouse_motion[1] = app->mouse_motion[1] - event->motion.yrel;
                }
                SDL_UnlockMutex(app->mouse_mutex);
            }
        }

        case SDL_EVENT_KEY_DOWN:
        {
            if (SDL_SCANCODE_ESCAPE == event->key.scancode)
            {
                SDL_Event event_quit = {
                    .quit = {
                        .type = SDL_EVENT_QUIT,
                        .timestamp = SDL_GetTicksNS(),
                    },
                };
                SDL_PushEvent(&event_quit);
            }
            break;
        }

        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        {
            /* You may wish to rescale the renderer and Nuklear during this event.
             * Without this the UI and Font could appear too small or too big.
             * This is not handled by the demo in order to keep it simple,
             * but you may wish to re-bake the Font whenever this happens. */
            log_wrn("Unhandled scale event! Nuklear may appear blurry");
            return SDL_APP_CONTINUE;
        }
        default:
        {
            break;
        }
    }


#ifdef NUKLEAR
    /* Remember to always rescale the event coordinates,
     * if your renderer uses custom scale. */
    SDL_ConvertEventToRenderCoordinates(app->renderer, event);
    nk_sdl_handle_event(app->ctx, event);
#endif

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    // {
    //     static size_t count = 0;
    //     ++count;
    //     log_dbg("update %zu", count);
    // }

    struct nk_sdl_app* app = appstate;

    vec2 mouse_delta = { 0 };
    {
        SDL_LockMutex(app->mouse_mutex);
        glm_vec2_scale(app->mouse_motion, 0.25f * ((float)M_PI / 180.0f), mouse_delta);
        glm_vec2_zero(app->mouse_motion);
        SDL_UnlockMutex(app->mouse_mutex);
    }

    static Uint64 ticks_last = 0;
    const Uint64 ticks_current = SDL_GetTicksNS();
    const double time_delta = (double)(ticks_current - ticks_last) / 1e9;
    ticks_last = ticks_current;

    log_dbg("dt: %.6f", time_delta);

    camera_update(&app->camera, mouse_delta);

    mat4 view_projection = { 0 };
    camera_get_transform(&app->camera, view_projection);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app->device);
    if (NULL == command_buffer)
    {
        log_fat("acquire gpu command buffer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapchain_texture = NULL;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app->window, &swapchain_texture, NULL, NULL))
    {
        log_fat("wait and acquire gpu swapchain texture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (NULL != swapchain_texture)
    {
        const SDL_GPUColorTargetInfo color_target_info = {
            .texture = swapchain_texture,
            .mip_level = 0,
            .layer_or_depth_plane = 0,
            .clear_color = { .r = app->bg.r, .g = app->bg.g, .b = app->bg.b, .a = app->bg.a },
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .resolve_texture = NULL,
            .resolve_mip_level = 0,
            .resolve_layer = 0,
            .cycle = false,
            .cycle_resolve_texture = false,
        };

        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);
        {
            // bind vertex buffer
            const SDL_GPUBufferBinding buffer_vertex_binding = {
                .buffer = app->buffer_vertex,
                .offset = 0,
            };
            SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_vertex_binding, 1);

            // bind index buffer
            const SDL_GPUBufferBinding buffer_index_binding = {
                .buffer = app->buffer_index,
                .offset = 0,
            };
            SDL_BindGPUIndexBuffer(render_pass, &buffer_index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

            // bind graphics pipeline
            SDL_BindGPUGraphicsPipeline(render_pass, app->pipeline);
            SDL_PushGPUVertexUniformData(command_buffer, 0, view_projection, sizeof(view_projection));

            // draw
            SDL_DrawGPUIndexedPrimitives(render_pass, 36, 1, 0, 0, 0);
        }
        SDL_EndGPURenderPass(render_pass);
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);

#ifdef NUKLEAR
    nk_input_end(app->ctx);

    float w = 0.0f;
    float h = 0.0f;
    {
        int width  = 0;
        int height = 0;
        if (!SDL_GetWindowSize(app->window, &width, &height))
        {
            log_fat("SDL_GetWindowSize failed: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
        w = (float)width;
        h = (float)height;
    }

    /* GUI */
    if (nk_begin(app->ctx, "cui", nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(app->ctx, 36.0f, 1);
        nk_button_label(app->ctx, "test");
    }
    nk_end(app->ctx);

    if (nk_sdl_render_needed(app->ctx))
    {
        static size_t count = 0;
        ++count;
        log_dbg("redraw count: %zu", count);

        SDL_SetRenderDrawColorFloat(app->renderer, app->bg.r, app->bg.g, app->bg.b, app->bg.a);
        SDL_RenderClear(app->renderer);

        nk_sdl_render(app->ctx, app->AA);
        nk_sdl_update_text_input(app->ctx);

        SDL_RenderPresent(app->renderer);
    }

    nk_input_begin(app->ctx);
#endif
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, const SDL_AppResult result)
{
    struct nk_sdl_app* app = appstate;

    if (NULL != app)
    {
#if 0
        nk_input_end(app->ctx);
        nk_sdl_shutdown(app->ctx);
#endif
        SDL_DestroyMutex(app->mouse_mutex);
        SDL_ReleaseGPUGraphicsPipeline(app->device, app->pipeline);
        SDL_ReleaseGPUBuffer(app->device, app->buffer_index);
        SDL_ReleaseGPUBuffer(app->device, app->buffer_vertex);
        SDL_ReleaseWindowFromGPUDevice(app->device, app->window);
        SDL_DestroyWindow(app->window);
        SDL_DestroyGPUDevice(app->device);
        SDL_free(app);
    }

    if (SDL_APP_SUCCESS == result)
    {
        log_msg("exit success");
    }

    if (SDL_APP_FAILURE == result)
    {
        log_fat("exit failure");
    }
}
