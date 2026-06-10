#ifndef SHADER_H_
#define SHADER_H_

// third party includes
#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

char* shader_program_load(unsigned char shader_source[], size_t shader_source_length);
SDL_GPUShader* shader_create_hlsl(SDL_GPUDevice* device, const SDL_ShaderCross_HLSL_Info* shader_program_info);
SDL_GPUComputePipeline* pipeline_create_hlsl(SDL_GPUDevice* device, const SDL_ShaderCross_HLSL_Info* shader_program_info);

#endif // SHADER_H_