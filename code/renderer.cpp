
struct Material {
    float exponent;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float dissolve;
    int illum_model;
};

struct VertexAttribute {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    Material material;
    unsigned int entity_ID;
};

struct StaticModel {
    Array<VertexAttribute> vertex_attributes;
};

struct Entity {
    unsigned int ID;
    StaticModel model;
    bool selected; //solid wireframe on/off
};

#define CAMERA_RIGHT 4
#define CAMERA_UP    5
#define CAMERA_DIR   6

struct Camera {
    vec3 position;
    vec3 direction;
    vec3 up;

    void rotate(vec3 *vector, float angle, int axis_of_rotation) {
        vec3 vec = *vector;
        switch(axis_of_rotation) {
            case X_AXIS: {
                vector->y = vec.y * (float)cos(angle) + vec.z * -1*(float)sin(angle);
                vector->z = vec.y * (float)sin(angle) + vec.z * (float)cos(angle);
            } break;
        
            case Y_AXIS: {
                vector->x = vec.x * (float)cos(angle) + vec.z * (float)sin(angle);
                vector->z = vec.x * -1*(float)sin(angle) + vec.z * (float)cos(angle);
            } break;
        
            case Z_AXIS: {
                vector->x = vec.x * (float)cos(angle) + vec.y * -1*(float)sin(angle);
                vector->y = vec.x * (float)sin(angle) + vec.y * (float)cos(angle);      
            } break;
        }
    }

    static void 
    rotate(vec3 *vector, float angle, vec3 *point, vec3 *line) { //rotates vector about "line" going through "point"
        vec3 vec = *vector;
        vector->x = (point->x*(line->y*line->y + line->z*line->z) - line->x*(point->y*line->y + point->z*line->z - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (float)cos(angle)) + vec.x*(float)cos(angle) + ((-point->z)*line->y + point->y*line->z - line->z*vec.y + line->y*vec.z) * (float)sin(angle);
    
        vector->y = (point->y*(line->x*line->x + line->z*line->z) - line->y*(point->x*line->x + point->z*line->z - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (float)cos(angle)) + vec.y*(float)cos(angle) + (point->z*line->x - point->x*line->z + line->z*vec.x - line->x*vec.z) * (float)sin(angle);
    
        vector->z = (point->z*(line->x*line->x + line->y*line->y) - line->z*(point->x*line->x + point->y*line->y - line->x*vec.x - line->y*vec.y - line->z*vec.z)) *
            (1.0f - (float)cos(angle)) + vec.z*(float)cos(angle) + ((-point->y)*line->x + point->x*line->y - line->y*vec.x + line->x*vec.y) * (float)sin(angle);
    }
    
    void rotate(float angle, int axis_of_rotation) {
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
    mat44 world_matrix;
    mat44 view_matrix;
    mat44 projection_matrix;
    bool wire_frame_on;
    int padding0;
    int padding1;
    int padding2;
};

struct D3D_RESOURCE {
    D3D_FEATURE_LEVEL feature_level;
    ID3D11Device *device;
    ID3D11DeviceContext *immediate_context;
    IDXGISwapChain *swap_chain;
    ID3D11Texture2D *back_buffer;
    ID3D11RenderTargetView *render_target;
    D3D11_VIEWPORT viewport;
    ID3D11VertexShader *vertex_shader;
    ID3D11GeometryShader *geometry_shader;    
    ID3D11PixelShader *pixel_shader;
    ID3D11Buffer *vertex_buffer;
    ID3D11Buffer *so_buffer;
    ID3D11RasterizerState *rasterizer_state;
    ID3D11Texture2D *depth_stencil_texture;
    ID3D11DepthStencilView* depth_stencil_view;
    
    Camera camera;
    int vertex_count;
};

bool set_vertex_buffer(D3D_RESOURCE *directx, Array<Entity> &entities) {
    
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

bool draw_frame(D3D_RESOURCE *directx, Array<Entity> &entities) {

    {
        FLOAT background_color[] = {0.788f, 0.867f, 1.0f, 1.0f};
        directx->immediate_context->ClearRenderTargetView(directx->render_target, background_color);
        directx->immediate_context->ClearDepthStencilView(directx->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
                    
        UINT stride = sizeof(VertexAttribute);
        UINT offset = 0;
        directx->immediate_context->IASetVertexBuffers(0, 1, &directx->vertex_buffer, &stride, &offset);
        directx->immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    }

    
    D3D11_BUFFER_DESC constant_buffer_desc = {};
    constant_buffer_desc.ByteWidth = sizeof(ShaderConstants);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;
    constant_buffer_desc.StructureByteStride = 0;
    
    ShaderConstants constants = {};
    constants.view_matrix = view_transform(directx->camera.position,
                                           directx->camera.direction,
                                           directx->camera.up);
    constants.projection_matrix = perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

    ID3D11Buffer *constant_buffer;
    int vertex_draw_offset = 0;
    for(int i = 0; i < entities.size; i++) {
        constants.world_matrix = make_scaling_matrix(1.0f, 1.0f, 1.0f, 1.0f); // NOTE(Alex): not currently used
        constants.wire_frame_on = entities[i].selected;
        
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
        directx->immediate_context->Draw(entities[i].model.vertex_attributes.size, vertex_draw_offset);
        vertex_draw_offset += entities[i].model.vertex_attributes.size;
    }
    
    HRESULT present_hr = directx->swap_chain->Present(0, 0);
    if(FAILED(present_hr)) {
        LOG_ERROR("ERROR", "Unable to swap buffers");
        return false;
    }
    
    return true;
}

bool init_D3D(HWND window, D3D_RESOURCE *directx) {
    HMODULE Direct3D_module_handle = LoadLibraryA("D3D11.dll");
    
    directx->feature_level = {};
    directx->device = {};
    directx->immediate_context = {};
    directx->swap_chain = {};
    directx->back_buffer = {};
    directx->render_target = {};
    directx->viewport = {};
    
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Width = 0; //default value 0 auto sets width/height
    swap_chain_desc.BufferDesc.Height = 0;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 1; //refresh rate for vsync
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 60;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //specifies what method the raster uses to create an image
    swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //specifies how to stretch and image to the screen

    swap_chain_desc.SampleDesc.Count = 4; //count=1,quality=0 turns off multisampling
    swap_chain_desc.SampleDesc.Quality = 16;

    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.Windowed = true; //TODO(Alex): IDXGISwapChain::SetFullscreenState
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    HRESULT device_hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE,
                                                      0, 0,
                                                      0, 0,
                                                      D3D11_SDK_VERSION, &swap_chain_desc,
                                                      &directx->swap_chain, &directx->device,
                                                      &directx->feature_level, &directx->immediate_context);
    if(FAILED(device_hr)) {
        LOG_ERROR("ERROR", "Cannot create Direct3D device");
        return false;
    }
    
    // get pointer to the back buffer
    HRESULT get_buffer_hr = directx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&directx->back_buffer);
    if(FAILED(get_buffer_hr)) {
        LOG_ERROR("ERROR", "Cannot retrieve back buffer location");
        return false;
    }
    
    // create a render target view
    HRESULT create_view_hr = directx->device->CreateRenderTargetView(directx->back_buffer, 0, &directx->render_target);    
    if(FAILED(create_view_hr)) {
        LOG_ERROR("ERROR", "Cannot create render target view");
        return false;
    }
    

    // load shaders
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
    
    D3D11_TEXTURE2D_DESC back_buffer_desc;
    directx->back_buffer->GetDesc(&back_buffer_desc);
    
    directx->viewport.Width = (FLOAT)back_buffer_desc.Width;
    directx->viewport.Height = (FLOAT)back_buffer_desc.Height;
    directx->viewport.MinDepth = 0.0f;
    directx->viewport.MaxDepth = 1.0f;
    directx->viewport.TopLeftX = 0;
    directx->viewport.TopLeftY = 0;
    directx->immediate_context->RSSetViewports(1, &directx->viewport);

    { // depth buffer config
        
        directx->depth_stencil_texture = {};
        D3D11_TEXTURE2D_DESC depth_stencil_texture_desc = {};
        depth_stencil_texture_desc.Width = back_buffer_desc.Width;
        depth_stencil_texture_desc.Height = back_buffer_desc.Height;
        depth_stencil_texture_desc.MipLevels = 1;
        depth_stencil_texture_desc.ArraySize = 1;
        depth_stencil_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_stencil_texture_desc.SampleDesc.Count = 4; 
        depth_stencil_texture_desc.SampleDesc.Quality = 16;
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
    directx->immediate_context->OMSetRenderTargets(1, &directx->render_target, directx->depth_stencil_view);

    directx->camera.position = vec3(0.0f, 1.0f, 0.0f);
    directx->camera.direction = vec3(0.0f, 0.0f, 1.0f);
    directx->camera.up = vec3(0.0f, 1.0f, 0.0f);
    
    return true;
}

