/* nuklear - public domain */

/* =============================================================== *
 *                                                                 *
 *                          CONFIG                                 *
 *                                                                 *
 * =============================================================== */

#ifndef GPU_H_
#define GPU_H_

#include <SDL3/SDL.h>

// optional: sdl3_gpu does not need any of these defines
// (but some examples might need them, so be careful)
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO

#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

// mandatory: sdl3_gpu depends on those defines
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT

// We can re-use the types provided by SDL which are extremely portable,
// so there is no need for Nuklear to detect those on its own
#ifndef NK_INCLUDE_FIXED_TYPES
    #define NK_INT8              Sint8
    #define NK_UINT8             Uint8
    #define NK_INT16             Sint16
    #define NK_UINT16            Uint16
    #define NK_INT32             Sint32
    #define NK_UINT32            Uint32
    // SDL guarantees 'uintptr_t' typedef
    #define NK_SIZE_TYPE         uintptr_t
    #define NK_POINTER_TYPE      uintptr_t
#endif // NK_INCLUDE_FIXED_TYPES

// We can reuse the `bool` symbol because SDL3 guarantees its existence
#ifndef NK_INCLUDE_STANDARD_BOOL
    #define NK_BOOL               bool
#endif // NK_INCLUDE_STANDARD_BOOL

// We can re-use various portable libc functions provided by SDL
#define NK_ASSERT(condition)      SDL_assert(condition)
#define NK_STATIC_ASSERT(exp)     SDL_COMPILE_TIME_ASSERT(, exp)
#define NK_MEMSET(dst, c, len)    SDL_memset(dst, c, len)
#define NK_MEMCPY(dst, src, len)  SDL_memcpy(dst, src, len)
#define NK_VSNPRINTF(s, n, f, a)  SDL_vsnprintf(s, n, f, a)
#define NK_STRTOD(str, endptr)    SDL_strtod(str, endptr)

/* SDL3 does not provide "dtoa" (only integer versions)
 * but we can emulate it with SDL_snprintf */
char* nk_sdl_dtoa(char* str, double d);
#define NK_DTOA(str, d) nk_sdl_dtoa(str, d)

// SDL can also provide us with math functions, but beware that Nuklear's own
// implementation can be slightly faster at the cost of some precision
#define NK_INV_SQRT(f)            (1.0f / SDL_sqrtf(f))
#define NK_SIN(f)                 SDL_sinf(f)
#define NK_COS(f)                 SDL_cosf(f)

// HACK: Nuklear pulls two stb libraries in order to use font baking
// those libraries pull in some libc headers internally, creating a linkage dependency,
// so you’ll most likely want to use SDL symbols instead
#define STBTT_ifloor(x)       ((int)SDL_floor(x))
#define STBTT_iceil(x)        ((int)SDL_ceil(x))
#define STBTT_sqrt(x)         SDL_sqrt(x)
#define STBTT_pow(x,y)        SDL_pow(x,y)
#define STBTT_fmod(x,y)       SDL_fmod(x,y)
#define STBTT_cos(x)          SDL_cosf(x)
#define STBTT_acos(x)         SDL_acos(x)
#define STBTT_fabs(x)         SDL_fabs(x)
#define STBTT_assert(x)       SDL_assert(x)
#define STBTT_strlen(x)       SDL_strlen(x)
#define STBTT_memcpy          SDL_memcpy
#define STBTT_memset          SDL_memset
#define stbtt_uint8           Uint8
#define stbtt_int8            Sint8
#define stbtt_uint16          Uint16
#define stbtt_int16           Sint16
#define stbtt_uint32          Uint32
#define stbtt_int32           Sint32
#define STBRP_SORT            SDL_qsort
#define STBRP_ASSERT          SDL_assert
// There is no need to define STBTT_malloc/STBTT_free macros
// Nuklear will define those to user-provided nk_allocator

// third party includes
#include <nuklear.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#if SDL_MAJOR_VERSION < 3
#error "nk_sdl3_gpu requires at least SDL 3.0.0"
#endif // SDL_MAJOR_VERSION < 3
#ifndef NK_INCLUDE_COMMAND_USERDATA
#error "nk_sdl3_gpu requires the NK_INCLUDE_COMMAND_USERDATA define"
#endif // NK_INCLUDE_COMMAND_USERDATA
#ifndef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#error "nk_sdl3_gpu requires the NK_INCLUDE_VERTEX_BUFFER_OUTPUT define"
#endif // NK_INCLUDE_VERTEX_BUFFER_OUTPUT

// We have to redefine it because demos do not include any headers
// This is the same default value as the one from "src/nuklear_internal.h"
#ifndef NK_BUFFER_DEFAULT_INITIAL_SIZE
#define NK_BUFFER_DEFAULT_INITIAL_SIZE (4*1024)
#endif // NK_BUFFER_DEFAULT_INITIAL_SIZE

NK_API struct nk_context*    nk_sdl_init(SDL_Window* window, SDL_GPUDevice* gpu, struct nk_allocator allocator, int max_vertex_buffer, int max_index_buffer);
NK_API void                  nk_sdl_shutdown(struct nk_context* ctx);
NK_API struct nk_allocator   nk_sdl_allocator(void);
NK_API SDL_GPUShaderFormat   nk_sdl_get_shader_formats(void);
NK_API int                   nk_sdl_handle_event(struct nk_context* ctx, SDL_Event* evt);
NK_API nk_bool               nk_sdl_render_needed(struct nk_context* ctx);
NK_API void                  nk_sdl_render(struct nk_context* ctx, enum nk_anti_aliasing AA, struct nk_colorf color);
NK_API void                  nk_sdl_update_text_input(struct nk_context* ctx);
NK_API nk_handle             nk_sdl_get_userdata(const struct nk_context* ctx);
NK_API void                  nk_sdl_set_userdata(struct nk_context* ctx, nk_handle userdata);
#ifdef NK_INCLUDE_FONT_BAKING
NK_API struct nk_font_atlas* nk_sdl_font_stash_begin(struct nk_context* ctx);
NK_API void                  nk_sdl_font_stash_end(struct nk_context* ctx);
#endif // NK_INCLUDE_FONT_BAKING

#endif // GPU_H_
