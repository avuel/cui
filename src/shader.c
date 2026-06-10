#include "shader.h"

// cui includes
#include "log.h"

static void shader_program_meta_data_log(const SDL_ShaderCross_GraphicsShaderMetadata* shader_program_meta_data)
{
    log_dbg("sampler count: %u", shader_program_meta_data->resource_info.num_samplers);
    log_dbg("storage texture count: %u", shader_program_meta_data->resource_info.num_storage_textures);
    log_dbg("storage buffers count: %u", shader_program_meta_data->resource_info.num_storage_buffers);
    log_dbg("uniform buffers count: %u", shader_program_meta_data->resource_info.num_uniform_buffers);

    log_dbg("input count: %u", shader_program_meta_data->num_inputs);
    for (Uint32 i = 0; i < shader_program_meta_data->num_inputs; ++i)
    {
        const SDL_ShaderCross_IOVarMetadata* input = &shader_program_meta_data->inputs[i];
        log_dbg("input %u: name=%s location=%u vector_type=%d vector_size=%d", i, input->name, input->location, input->vector_type, input->vector_size);
    }

    log_dbg("output count: %u", shader_program_meta_data->num_outputs);
    for (Uint32 i = 0; i < shader_program_meta_data->num_outputs; ++i)
    {
        const SDL_ShaderCross_IOVarMetadata* output = &shader_program_meta_data->outputs[i];
        log_dbg("output %u: name=%s location=%u vector_type=%d vector_size=%d", i, output->name, output->location, output->vector_type, output->vector_size);
    }
}

char* shader_program_load(unsigned char shader_source[], const size_t shader_source_length)
{
    char* shader_program = SDL_malloc(shader_source_length + 1);
    if (NULL == shader_program)
    {
        log_err("SDL_malloc failed: %s", SDL_GetError());
        return NULL;
    }

    memcpy(shader_program, shader_source, shader_source_length);
    shader_program[shader_source_length] = '\0';

    return shader_program;
}

SDL_GPUShader* shader_create_hlsl(SDL_GPUDevice* device, const SDL_ShaderCross_HLSL_Info* shader_program_info)
{
    size_t shader_program_code_size = 0;
    void* shader_program_code = SDL_ShaderCross_CompileSPIRVFromHLSL(shader_program_info, &shader_program_code_size);

    if (NULL == shader_program_code)
    {
        log_err("shader program compilation failed: %s", SDL_GetError());
        return NULL;
    }

    const SDL_ShaderCross_SPIRV_Info shader_program_info_spirv = {
        .bytecode = shader_program_code,
        .bytecode_size = shader_program_code_size,
        .entrypoint = shader_program_info->entrypoint,
        .shader_stage = shader_program_info->shader_stage,
        .props = shader_program_info->props,
    };

    const bool graphics_shader = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE != shader_program_info->shader_stage;

    SDL_GPUShader* shader = NULL;

    if (graphics_shader)
    {
        SDL_ShaderCross_GraphicsShaderMetadata* shader_program_meta_data = SDL_ShaderCross_ReflectGraphicsSPIRV(shader_program_code, shader_program_code_size, shader_program_info->props);

        if (NULL == shader_program_meta_data)
        {
            log_err("shader program meta data reflection failed: %s", SDL_GetError());
            return NULL;
        }

        shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &shader_program_info_spirv, &shader_program_meta_data->resource_info, shader_program_info->props);
        SDL_free(shader_program_meta_data);

        if (NULL == shader)
        {
            log_err("shader compilation failed: %s", SDL_GetError());
            goto shader_create_hlsl_failure;
        }
    }
    else
    {
        log_err("shader stage is not graphics");
        goto shader_create_hlsl_failure;
    }

    // if made it here, return normally
    goto shader_create_hlsl_return;

// label to exit early
shader_create_hlsl_failure:
    shader = NULL;

// label for return (handles cleanup)
shader_create_hlsl_return:
    SDL_free(shader_program_code);
    return shader;
}

SDL_GPUComputePipeline* pipeline_create_hlsl(SDL_GPUDevice* device, const SDL_ShaderCross_HLSL_Info* shader_program_info)
{
    size_t shader_program_code_size = 0;
    void* shader_program_code = SDL_ShaderCross_CompileSPIRVFromHLSL(shader_program_info, &shader_program_code_size);

    if (NULL == shader_program_code)
    {
        log_err("shader program compilation failed: %s", SDL_GetError());
        return NULL;
    }

    const SDL_ShaderCross_SPIRV_Info shader_program_info_spirv = {
        .bytecode = shader_program_code,
        .bytecode_size = shader_program_code_size,
        .entrypoint = shader_program_info->entrypoint,
        .shader_stage = shader_program_info->shader_stage,
        .props = shader_program_info->props,
    };

    const bool compute_shader = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE == shader_program_info->shader_stage;

    SDL_GPUComputePipeline* pipeline = NULL;

    if (compute_shader)
    {
        SDL_ShaderCross_ComputePipelineMetadata* shader_program_meta_data = SDL_ShaderCross_ReflectComputeSPIRV(shader_program_code, shader_program_code_size, shader_program_info->props);

        if (NULL == shader_program_meta_data)
        {
            log_err("shader program meta data reflection failed: %s", SDL_GetError());
            goto pipeline_create_hlsl_failure;
        }

        pipeline = SDL_ShaderCross_CompileComputePipelineFromSPIRV(device, &shader_program_info_spirv, shader_program_meta_data, shader_program_info->props);
        SDL_free(shader_program_meta_data);

        if (NULL == pipeline)
        {
            log_err("pipeline compilation failed: %s", SDL_GetError());
            goto pipeline_create_hlsl_failure;
        }
    }

    // if made it here, return normally
    goto pipeline_create_hlsl_return;

// label to exit early
pipeline_create_hlsl_failure:
    pipeline = NULL;

// label for return (handles cleanup)
pipeline_create_hlsl_return:
    SDL_free(shader_program_code);
    return pipeline;
}