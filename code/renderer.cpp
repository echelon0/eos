
struct VertexAttribute {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    u32 entity_ID;

    VertexAttribute operator = (VertexAttribute rhs) {
        this->position = rhs.position;
        this->normal = rhs.normal;
        this->texcoord = rhs.texcoord;
        this->entity_ID = rhs.entity_ID;
        return *this;
    }
};

struct Material {
    vec3 diffuse;
    vec3 specular;
    f32 exponent;
    f32 dissolve;
    u32 illum_model;
};

struct StaticModel {
    char str_name[128];
    Array<VertexAttribute> vertex_attributes;
    Array<Material> materials;
    Array<int> material_indices; //vertex_attribute indices with new materials
    Array<int> material_sizes; //how many vertices use this material
};

#define MAX_LIGHTS 64
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOTLIGHT 2

struct Light {
    int entity_ID;
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 cone_angle;
    u32 light_type;
    bool enabled;
};

struct Entity {
    unsigned int ID;
    StaticModel model;
    vec3 world_pos;
    quat orientation;
    vec3 velocity;
    float terminal_velocity;
    vec3 acceleration;
    bool selected; //solid wireframe on/off

    f32 current_speed;
    f32 walk_speed;
    f32 run_speed;
    f32 target_speed;
    quat target_orientation;
};

#define CAMERA_RIGHT 4
#define CAMERA_UP    5
#define CAMERA_DIR   6

struct Camera {
    vec3 position;
    vec3 direction;
    vec3 up;

    void rotate(vec3 *vector, f32 angle, int axis_of_rotation) {
        vec3 vec = *vector;
        switch(axis_of_rotation) {
            case X_AXIS: {
                vector->y = vec.y * (f32)cos(angle) + vec.z * -1*(f32)sin(angle);
                vector->z = vec.y * (f32)sin(angle) + vec.z * (f32)cos(angle);
            } break;
        
            case Y_AXIS: {
                vector->x = vec.x * (f32)cos(angle) + vec.z * (f32)sin(angle);
                vector->z = vec.x * -1*(f32)sin(angle) + vec.z * (f32)cos(angle);
            } break;
        
            case Z_AXIS: {
                vector->x = vec.x * (f32)cos(angle) + vec.y * -1*(f32)sin(angle);
                vector->y = vec.x * (f32)sin(angle) + vec.y * (f32)cos(angle);      
            } break;
        }
    }

    static void 
    rotate(vec3 *vector, f32 angle, vec3 *point, vec3 *line) { //rotates vector about "line" going through "point"
        vec3 vec = *vector;
        vector->x = (point->x*(line->y*line->y + line->z*line->z) - line->x*(point->y*line->y + point->z*line->z - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (f32)cos(angle)) + vec.x*(f32)cos(angle) + ((-point->z)*line->y + point->y*line->z - line->z*vec.y + line->y*vec.z) * (f32)sin(angle);
    
        vector->y = (point->y*(line->x*line->x + line->z*line->z) - line->y*(point->x*line->x + point->z*line->z - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (f32)cos(angle)) + vec.y*(f32)cos(angle) + (point->z*line->x - point->x*line->z + line->z*vec.x - line->x*vec.z) * (f32)sin(angle);
    
        vector->z = (point->z*(line->x*line->x + line->y*line->y) - line->z*(point->x*line->x + point->y*line->y - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (f32)cos(angle)) + vec.z*(f32)cos(angle) + ((-point->y)*line->x + point->x*line->y - line->y*vec.x + line->x*vec.y) * (f32)sin(angle);
    }
    
    void rotate(f32 angle, int axis_of_rotation) {
        vec3 right = normalize(cross(up, direction));
        
        if((axis_of_rotation == X_AXIS) || (axis_of_rotation == Y_AXIS) || (axis_of_rotation == Z_AXIS)) {
            rotate(&direction, angle, axis_of_rotation);
            rotate(&up, angle, axis_of_rotation);           
        } else {
            if(axis_of_rotation == CAMERA_RIGHT) {
                vec3 origin = vec3(0.0f, 0.0f, 0.0f);
                rotate(&direction, angle, &origin, &right);
                rotate(&up, angle, &origin, &right);
            } else if(axis_of_rotation == CAMERA_UP) {
                rotate(&direction, angle, &position, &up);
            } 
        }
        direction = normalize(direction);
        up = normalize(up);        
    }
};

struct ShaderConstants {
    mat44 model_matrix;
    mat44 view_matrix;
    mat44 projection_matrix;
    bool wire_frame_on;
    int entity_id;
    u32 padding1;
    u32 padding2;
};

struct MaterialConstants {
    vec4 diffuse;
    vec4 specular;
    f32 exponent;
    f32 dissolve;
    u32 illum_model;
    u32 iTime;
};

struct ShaderLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    f32 cone_angle;
    u32 light_type;
    bool enabled;
    u32 padding3;
    u32 padding4;
    vec3 padding5;
};

struct LightConstants {
    ShaderLight shader_lights[MAX_LIGHTS];
    vec4 eye_position;
};

struct D3D_RESOURCES {
    D3D_FEATURE_LEVEL feature_level;
    ID3D11Device *device;
    ID3D11DeviceContext *immediate_context;
    IDXGISwapChain1 *swap_chain;
    ID3D11Texture2D *back_buffer;
    ID3D11Texture2D *picking_buffer;
    ID3D11RenderTargetView *render_target_views[2];
    D3D11_VIEWPORT viewport;
    ID3D11VertexShader *vertex_shader;
    ID3D11GeometryShader *geometry_shader;    
    ID3D11PixelShader *pixel_shader;
    ID3D11Buffer *vertex_buffer;
    ID3D11RasterizerState *rasterizer_state;
    ID3D11Texture2D *depth_stencil_texture;
    ID3D11DepthStencilView *depth_stencil_view;
    D3D11_MAPPED_SUBRESOURCE picking_data;
    int vertex_count;
};

bool set_vertex_buffer(D3D_RESOURCES *directx, Array<Entity> &entities) {
    
    Array<VertexAttribute> all_vertices;
    for(int entity_index = 0; entity_index < entities.size; entity_index++) {
        for(int i = 0; i < entities[entity_index].model.vertex_attributes.size; i++) {
            all_vertices.push_back(entities[entity_index].model.vertex_attributes[i]);
        }
    }
    directx->vertex_count = all_vertices.size;
    
    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = sizeof(VertexAttribute) * directx->vertex_count;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    
    directx->device->CreateBuffer(&vertex_buffer_desc, 0, &directx->vertex_buffer);
    HRESULT map_hr = directx->immediate_context->Map(directx->vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if(FAILED(map_hr)) {
        LOG_ERROR("ERROR", "Cannot map vertex buffer");
        return false;
    }
    
    memcpy(mapped_subresource.pData, all_vertices.data, sizeof(VertexAttribute) * directx->vertex_count);
    directx->immediate_context->Unmap(directx->vertex_buffer, 0);

    return true;
}

bool draw_frame(D3D_RESOURCES *directx, Array<Entity> &entities, Light *lights, Camera camera) {

    FLOAT background_color[] = {0.788f, 0.867f, 1.0f, 1.0f};
    FLOAT picking_buffer_background[] = {-1.0f, -1.0f, -1.0f, -1.0f};
    directx->immediate_context->ClearRenderTargetView(directx->render_target_views[0], background_color);
    directx->immediate_context->ClearRenderTargetView(directx->render_target_views[1], picking_buffer_background);
    directx->immediate_context->ClearDepthStencilView(directx->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    UINT stride = sizeof(VertexAttribute);
    UINT offset = 0;
    directx->immediate_context->IASetVertexBuffers(0, 1, &directx->vertex_buffer, &stride, &offset);
    directx->immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
    directx->immediate_context->OMSetRenderTargets(2, &directx->render_target_views[0], directx->depth_stencil_view);
    
    D3D11_BUFFER_DESC constant_buffer_desc = {};
    constant_buffer_desc.ByteWidth = sizeof(ShaderConstants);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;
    constant_buffer_desc.StructureByteStride = 0;
    
    ShaderConstants constants = {};
    constants.view_matrix = view_transform(camera.position,
                                           camera.direction,
                                           camera.up);
    constants.projection_matrix = perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    
    D3D11_BUFFER_DESC material_buffer_desc = {};
    material_buffer_desc.ByteWidth = sizeof(MaterialConstants);
    material_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    material_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    material_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    material_buffer_desc.MiscFlags = 0;
    material_buffer_desc.StructureByteStride = 0;

    MaterialConstants material_constants = {};
    material_constants.iTime = global_iTime;

    D3D11_BUFFER_DESC light_buffer_desc = {};
    light_buffer_desc.ByteWidth = sizeof(LightConstants);
    light_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    light_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    light_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    light_buffer_desc.MiscFlags = 0;
    light_buffer_desc.StructureByteStride = 0;

    LightConstants light_constants = {};

    //light constant buffer
    light_constants.eye_position = vec4(camera.position, 1.0f);
    for(int i = 0; i < MAX_LIGHTS; i++)  {
        ShaderLight shader_light = {};
        shader_light.position = vec4(lights[i].position, 1.0f);
        shader_light.direction = vec4(lights[i].direction, 1.0f);
        shader_light.color = vec4(lights[i].color, 1.0f);
        shader_light.cone_angle = lights[i].cone_angle;
        shader_light.light_type = lights[i].light_type;
        shader_light.enabled = lights[i].enabled;

        light_constants.shader_lights[i] = shader_light;
    }
    
    ID3D11Buffer *constant_buffer;
    ID3D11Buffer *material_buffer;
    ID3D11Buffer *light_buffer;

    D3D11_SUBRESOURCE_DATA light_buffer_data = {};
    light_buffer_data.pSysMem = &light_constants;
    light_buffer_data.SysMemPitch = 0;
    light_buffer_data.SysMemSlicePitch = 0;

    HRESULT create_light_buffer_hr = directx->device->CreateBuffer(&light_buffer_desc, &light_buffer_data, &light_buffer);
    if(FAILED(create_light_buffer_hr)) {
        LOG_ERROR("ERROR", "Cannot create light buffer");
        return false;
    }    
    directx->immediate_context->PSSetConstantBuffers(2, 1, &light_buffer);
    
    int vertex_draw_offset = 0;    
    for(int i = 0; i < entities.size; i++) {
        //vertex constant buffer
        constants.model_matrix = model_transform(entities[i].world_pos, entities[i].orientation);
        constants.wire_frame_on = entities[i].selected;
        constants.entity_id = entities[i].ID;
        
        D3D11_SUBRESOURCE_DATA constant_buffer_data = {};
        constant_buffer_data.pSysMem = &constants;
        constant_buffer_data.SysMemPitch = 0;
        constant_buffer_data.SysMemSlicePitch = 0;

        HRESULT create_constant_buffer_hr = directx->device->CreateBuffer(&constant_buffer_desc, &constant_buffer_data, &constant_buffer);
        if(FAILED(create_constant_buffer_hr)) {
            LOG_ERROR("ERROR", "Cannot create constant buffer");
            return false;
        }
        directx->immediate_context->VSSetConstantBuffers(0, 1, &constant_buffer);

        //material constant buffer
        for(int mat_index = 0; mat_index < entities[i].model.materials.size; mat_index++) {
            material_constants.diffuse = vec4(entities[i].model.materials[mat_index].diffuse, 1.0f);
            material_constants.specular = vec4(entities[i].model.materials[mat_index].specular, 1.0f);
            material_constants.exponent = entities[i].model.materials[mat_index].exponent;
            material_constants.dissolve = entities[i].model.materials[mat_index].dissolve;
            material_constants.illum_model = entities[i].model.materials[mat_index].illum_model;
            D3D11_SUBRESOURCE_DATA material_buffer_data = {};
            material_buffer_data.pSysMem = &material_constants;
            material_buffer_data.SysMemPitch = 0;
            material_buffer_data.SysMemSlicePitch = 0;

            HRESULT create_material_buffer_hr = directx->device->CreateBuffer(&material_buffer_desc, &material_buffer_data, &material_buffer);
            if(FAILED(create_material_buffer_hr)) {
                LOG_ERROR("ERROR", "Cannot create material buffer");
                return false;
            }
            directx->immediate_context->PSSetConstantBuffers(1, 1, &material_buffer);
            directx->immediate_context->Draw(entities[i].model.material_sizes[mat_index], vertex_draw_offset + entities[i].model.material_indices[mat_index]);
        }
        vertex_draw_offset += entities[i].model.vertex_attributes.size;
    }    
    
    return true;
}

bool init_D3D(HWND window, D3D_RESOURCES *directx) {
    HMODULE Direct3D_module_handle = LoadLibraryA("D3D11.dll");
    
    HRESULT device_hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
                                          0, D3D11_CREATE_DEVICE_DEBUG,
                                          0, 0,
                                          D3D11_SDK_VERSION, &directx->device,
                                          &directx->feature_level, &directx->immediate_context);
    if(FAILED(device_hr)) {
        LOG_ERROR("ERROR", "Cannot create Direct3D device");
        return false;
    }

    UINT msaa_sample_count = 4; //TODO: make it configurable by the user
    UINT msaa_quality = 0;
    
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = 0;
    swap_chain_desc.Height = 0;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.Stereo = 0;
    
    HRESULT get_msaa_quality_level_hr = directx->device->CheckMultisampleQualityLevels(swap_chain_desc.Format, msaa_sample_count, &msaa_quality);
    if(FAILED(get_msaa_quality_level_hr)) {
        LOG_ERROR("ERROR", "Cannot check multisample quality level");
        return false;
    }
    swap_chain_desc.SampleDesc.Count = 1; //count=1,quality=0 turns off multisampling
    swap_chain_desc.SampleDesc.Quality = 0;

    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;

    swap_chain_desc.Scaling = DXGI_SCALING_NONE; //NOTE: Maybe DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED? TODO: Check on this
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; //DXGI_SWAP_EFFECT_DISCARD;
        
    swap_chain_desc.Flags = 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    //creating the swap chain
    IDXGIDevice *dxgi_device;
    HRESULT hr = directx->device->QueryInterface( __uuidof(IDXGIDevice), (void **) &dxgi_device);
    if(SUCCEEDED(hr)) {
        IDXGIAdapter *dxgi_adapter = 0;
        hr = dxgi_device->GetParent( __uuidof(IDXGIAdapter), (void **) &dxgi_adapter);
        if(SUCCEEDED(hr)) {
            IDXGIFactory2 *dxgi_factory = 0;
            hr = dxgi_adapter->GetParent( __uuidof(IDXGIFactory), (void **) &dxgi_factory);
            if(SUCCEEDED(hr)) {

                HRESULT swap_chain_hr = dxgi_factory->CreateSwapChainForHwnd(directx->device, window,
                                                                             &swap_chain_desc, 0,
                                                                             0, &directx->swap_chain);
                if(FAILED(swap_chain_hr)) {
                    LOG_ERROR("ERROR", "Cannot create Direct3D swap chain");
                    return false;
                }               

                dxgi_factory->Release();
            } else {
                LOG_ERROR("ERROR", "Cannot create DXGIFactory");
                return false;
            }
            dxgi_adapter->Release();
        } else {
            LOG_ERROR("ERROR", "Cannot create DXGIAdapter");
            return false;
        }
        dxgi_device->Release();
    } else {
        LOG_ERROR("ERROR", "Cannot create DXGIDevice");
        return false;
    }    

    HRESULT get_buffer_hr = directx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&directx->back_buffer);
    if(FAILED(get_buffer_hr)) {
        LOG_ERROR("ERROR", "Cannot retrieve back buffer location");
        return false;
    }    

    D3D11_TEXTURE2D_DESC back_buffer_desc;
    directx->back_buffer->GetDesc(&back_buffer_desc);
    ivec2 back_buffer_dim = ivec2(back_buffer_desc.Width, back_buffer_desc.Height);
    
    D3D11_TEXTURE2D_DESC picking_buffer_desc = {};
    picking_buffer_desc.Width = back_buffer_dim.x;
    picking_buffer_desc.Height = back_buffer_dim.y;
    picking_buffer_desc.MipLevels = 1;
    picking_buffer_desc.ArraySize = 1;
    picking_buffer_desc.Format = DXGI_FORMAT_R32_SINT;
    picking_buffer_desc.SampleDesc.Count = 1;
    picking_buffer_desc.SampleDesc.Quality = 0;
    picking_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    picking_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    picking_buffer_desc.CPUAccessFlags = 0;
    picking_buffer_desc.MiscFlags = 0;
    directx->device->CreateTexture2D(&picking_buffer_desc, 0, &directx->picking_buffer);
    
    HRESULT create_view0_hr = directx->device->CreateRenderTargetView(directx->back_buffer, 0, &directx->render_target_views[0]);
    if(FAILED(create_view0_hr)) {
        LOG_ERROR("ERROR", "Cannot create render target view for back buffer");
        return false;
    }
    HRESULT create_view1_hr = directx->device->CreateRenderTargetView(directx->picking_buffer, 0, &directx->render_target_views[1]);
    if(FAILED(create_view1_hr)) {
        LOG_ERROR("ERROR", "Cannot create render target view for picking buffer");
        return false;
    }
    
    ID3DBlob *vs_blob = (ID3DBlob *)malloc(sizeof(*vs_blob));
    ID3DBlob *gs_blob = (ID3DBlob *)malloc(sizeof(*gs_blob));
    ID3DBlob *ps_blob = (ID3DBlob *)malloc(sizeof(*ps_blob));
    UINT shader_compiler_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    ID3DBlob *error_message;
    bool should_return = false;
    
    HRESULT compile_vs_hr = D3DCompileFromFile(L"../code/unified_shader.h", 0, 0, "vs_main", "vs_4_0", shader_compiler_flags, 0, &vs_blob, &error_message);
    if(FAILED(compile_vs_hr)) {
        LOG_ERROR("ERROR: Cannot compile vertex shader", (char *)error_message->GetBufferPointer());
        should_return = true;
    }
    
    HRESULT compile_gs_hr = D3DCompileFromFile(L"../code/unified_shader.h", 0, 0, "gs_main", "gs_4_0", shader_compiler_flags, 0, &gs_blob, &error_message);
    if(FAILED(compile_gs_hr)) {
        LOG_ERROR("ERROR: Cannot compile geometry shader", (char *)error_message->GetBufferPointer());
        should_return = true;
    }
        
    HRESULT compile_ps_hr = D3DCompileFromFile(L"../code/unified_shader.h", 0, 0, "ps_main", "ps_4_0", shader_compiler_flags, 0, &ps_blob, &error_message);
    if(FAILED(compile_ps_hr)) {
        LOG_ERROR("ERROR: Cannot compile pixel shader", (char *)error_message->GetBufferPointer());
        should_return = true;
    }
    
    if(should_return) return false;
    
    HRESULT create_vs_hr = directx->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &directx->vertex_shader);
    HRESULT create_gs_hr = directx->device->CreateGeometryShader(gs_blob->GetBufferPointer(), gs_blob->GetBufferSize(), 0, &directx->geometry_shader);
    HRESULT create_ps_hr = directx->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &directx->pixel_shader);
    if(FAILED(create_vs_hr)) {
        LOG_ERROR("ERROR", "Cannot create vertex shader");
        should_return = true;
    }
    if(FAILED(create_gs_hr)) {
        LOG_ERROR("ERROR", "Cannot create geometry shader");
        should_return = true;
    }
    if(FAILED(create_ps_hr)) {
        LOG_ERROR("ERROR", "Cannot create pixel shader");
        should_return = true;
    }
    if(should_return) return false;
    
    directx->immediate_context->VSSetShader(directx->vertex_shader, 0, 0);
    directx->immediate_context->GSSetShader(directx->geometry_shader, 0, 0);
    directx->immediate_context->PSSetShader(directx->pixel_shader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC layout_desc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ID3D11InputLayout *input_layout;
    HRESULT create_layout_hr = directx->device->CreateInputLayout(layout_desc, ARRAYSIZE(layout_desc), vs_blob->GetBufferPointer(),
                                                                  vs_blob->GetBufferSize(), &input_layout);
    if(FAILED(create_layout_hr)) {
        LOG_ERROR("ERROR", "Cannot create input layout");
        return false;
    }
    directx->immediate_context->IASetInputLayout(input_layout);

    // viewport
    directx->viewport.Width = (FLOAT)back_buffer_dim.x;
    directx->viewport.Height = (FLOAT)back_buffer_dim.y;
    directx->viewport.MinDepth = 0.0f;
    directx->viewport.MaxDepth = 1.0f;
    directx->viewport.TopLeftX = 0;
    directx->viewport.TopLeftY = 0;
    directx->immediate_context->RSSetViewports(1, &directx->viewport);
        
    { // rasterizer config
        
        D3D11_RASTERIZER_DESC raster_desc = {};
        raster_desc.FillMode = D3D11_FILL_SOLID;
        raster_desc.CullMode = D3D11_CULL_BACK;
        raster_desc.FrontCounterClockwise = true;
        raster_desc.DepthBias = 0;
        raster_desc.DepthBiasClamp = 0;
        raster_desc.SlopeScaledDepthBias = 0;
        raster_desc.DepthClipEnable = false;
        raster_desc.ScissorEnable = 0;
        raster_desc.MultisampleEnable = 1;
        raster_desc.AntialiasedLineEnable = 0;
    
        HRESULT raster_state_hr = directx->device->CreateRasterizerState(&raster_desc, &directx->rasterizer_state);
        if(FAILED(raster_state_hr)) {
            LOG_ERROR("ERROR", "Cannot configurate rasterizer state");
            return false;
        }

        directx->immediate_context->RSSetState(directx->rasterizer_state);
    }

    { // depth buffer config
        
        directx->depth_stencil_texture = {};
        D3D11_TEXTURE2D_DESC depth_stencil_texture_desc = {};
        depth_stencil_texture_desc.Width = back_buffer_dim.x;
        depth_stencil_texture_desc.Height = back_buffer_dim.y;
        depth_stencil_texture_desc.MipLevels = 1;
        depth_stencil_texture_desc.ArraySize = 1;
        depth_stencil_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_stencil_texture_desc.SampleDesc.Count = 1;
        depth_stencil_texture_desc.SampleDesc.Quality = 0;
        depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
        depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depth_stencil_texture_desc.CPUAccessFlags = 0;
        depth_stencil_texture_desc.MiscFlags = 0;
    
        HRESULT create_depth_stencil_hr = directx->device->CreateTexture2D(&depth_stencil_texture_desc, NULL, &directx->depth_stencil_texture);
        if(FAILED(create_depth_stencil_hr)) {
            LOG_ERROR("ERROR", "Cannot create depth stencil texture");
            return false;
        }

        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
        depth_stencil_desc.DepthEnable = true;
        depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
        depth_stencil_desc.StencilEnable = false;
    
        ID3D11DepthStencilState *depth_stencil_state = {};
    
        HRESULT create_depth_stencil_state_hr = directx->device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
        if(FAILED(create_depth_stencil_state_hr)) {
            LOG_ERROR("ERROR", "Cannot create depth stencil state");
            return false;
        }
        directx->immediate_context->OMSetDepthStencilState(depth_stencil_state, 1);
   
        D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
        depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        depth_stencil_view_desc.Flags = 0;
        depth_stencil_view_desc.Texture2D.MipSlice = 0;

        HRESULT create_depth_stencil_view_hr = directx->device->CreateDepthStencilView(directx->depth_stencil_texture, &depth_stencil_view_desc, &directx->depth_stencil_view);
        if(FAILED(create_depth_stencil_view_hr)) {
            LOG_ERROR("ERROR", "Cannot create depth stencil view");
            return false;
        }
    }

    return true;
}

void clean_D3D(D3D_RESOURCES *directx) {
    directx->device->Release();
    directx->immediate_context->Release();
    directx->swap_chain->Release();
    directx->back_buffer->Release();
    directx->picking_buffer->Release();
    directx->render_target_views[0]->Release();
    directx->render_target_views[1]->Release();
    directx->vertex_shader->Release();
    directx->geometry_shader->Release();
    directx->pixel_shader->Release();
    directx->vertex_buffer->Release();
    directx->rasterizer_state->Release();
    directx->depth_stencil_texture->Release();
    directx->depth_stencil_view->Release();
}

bool read_ID3D11Texture2D(D3D11_MAPPED_SUBRESOURCE &mapped_subresource, ID3D11Texture2D *texture, ID3D11Device *device, ID3D11DeviceContext *device_context) {

    D3D11_TEXTURE2D_DESC texture_desc;
    texture->GetDesc(&texture_desc);
    
    ID3D11Texture2D *staging_texture;
    D3D11_TEXTURE2D_DESC staging_texture_desc = {};
    staging_texture_desc.Width = texture_desc.Width;
    staging_texture_desc.Height = texture_desc.Height;
    staging_texture_desc.MipLevels = 1;
    staging_texture_desc.ArraySize = 1;
    staging_texture_desc.Format = texture_desc.Format;
    staging_texture_desc.SampleDesc.Count = 1;
    staging_texture_desc.SampleDesc.Quality = 0;
    staging_texture_desc.Usage = D3D11_USAGE_STAGING;
    staging_texture_desc.BindFlags = 0;
    staging_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_texture_desc.MiscFlags = 0;
    
    HRESULT create_texture_hr = device->CreateTexture2D(&staging_texture_desc, 0, &staging_texture);
    if(FAILED(create_texture_hr)) {
        LOG_ERROR("Error", "Cannot create staging texture when extracting pixels from back buffer");
        return false;
    }

    device_context->CopyResource(staging_texture, texture);
    
    device_context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_subresource);
    device_context->Unmap(staging_texture, 0);
    
    staging_texture->Release();
    
    return true;
}
