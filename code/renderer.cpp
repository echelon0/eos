
struct VertexAttribute {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

struct StaticModel {
    Array<VertexAttribute> vertices;
};

struct Entity {
    unsigned int ID;
    StaticModel model;
};

struct Camera {
    vec3 position;
    vec3 direction;
    vec3 up;
};

struct ShaderConstants {
    mat44 world_matrix;
    mat44 view_matrix;
    mat44 projection_matrix;
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
    ID3D11PixelShader *pixel_shader;
    ID3D11Buffer *vertex_buffer;
    ID3D11Buffer *constant_buffer;
    ID3D11RasterizerState *rasterizer_state;
    ID3D11Texture2D *depth_stencil_texture;
    ID3D11DepthStencilView* depth_stencil_view;
    
    Camera camera;
    int vertex_count;
};

void draw_frame(D3D_RESOURCE *directx) {
    FLOAT color[] = {0.788f, 0.867f, 1.0f, 1.0f};
    

    directx->immediate_context->ClearRenderTargetView(directx->render_target, color);
    directx->immediate_context->ClearDepthStencilView(directx->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
                    
    UINT stride = sizeof(VertexAttribute);
    UINT offset = 0;
    directx->immediate_context->IASetVertexBuffers(0, 1, &directx->vertex_buffer, &stride, &offset);
    directx->immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    directx->immediate_context->Draw(directx->vertex_count, 0);
    HRESULT present_hr = directx->swap_chain->Present(0, 0);
    if(FAILED(present_hr)) {
        LOG_ERROR("Unable to swap buffers");
    }

}

bool set_vertex_buffer(D3D_RESOURCE *directx, Array<StaticModel> models) {
    
    Array<VertexAttribute> all_vertices;

    for(int model_index = 0; model_index < models.size; model_index++) {
        for(int i = 0; i < models[model_index].vertices.size; i++) {
            all_vertices.push_back(models[model_index].vertices[i]);
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
        LOG_ERROR("Cannot map vertex buffer");
        return false;
    }
    
    memcpy(mapped_subresource.pData, all_vertices.data, sizeof(VertexAttribute) * directx->vertex_count);
    directx->immediate_context->Unmap(directx->vertex_buffer, 0);

    return true;
}

bool set_constant_buffer(D3D_RESOURCE *directx) {
    if(global_input.W_KEY) {
        directx->camera.position += directx->camera.direction * 0.1f;
    }
    if(global_input.S_KEY) {
        directx->camera.position -= directx->camera.direction * 0.1f;
    }
    if(global_input.A_KEY) {
        directx->camera.position += cross(directx->camera.direction, directx->camera.up) * 0.1f;
    }
    if(global_input.D_KEY) {
        directx->camera.position -= cross(directx->camera.direction, directx->camera.up) * 0.1f;
    }
    if(global_input.UP_ARROW) {
        directx->camera.position += directx->camera.up * 0.1f;
    }
    if(global_input.DOWN_ARROW) {
        directx->camera.position -= directx->camera.up * 0.1f;
    }
    
    if(global_input.RIGHT_ARROW) {
        rotate(&directx->camera.direction, 0.01f, Y_AXIS);
    }
    if(global_input.LEFT_ARROW) {
        rotate(&directx->camera.direction, -0.01f, Y_AXIS);
    }    
    
    
    ShaderConstants constants = {};
    constants.world_matrix = make_scaling_matrix(1.0f, 1.0f, 1.0f, 1.0f); // NOTE(Alex): not currently used
    constants.view_matrix = view_transform(directx->camera.position,
                                           directx->camera.direction,
                                           directx->camera.up);
    
    constants.projection_matrix = perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

    D3D11_BUFFER_DESC constant_buffer_desc = {};
    constant_buffer_desc.ByteWidth = sizeof(ShaderConstants);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;
    constant_buffer_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA constant_buffer_data = {};
    constant_buffer_data.pSysMem = &constants;
    constant_buffer_data.SysMemPitch = 0;
    constant_buffer_data.SysMemSlicePitch = 0;
    
    HRESULT create_constant_buffer_hr = directx->device->CreateBuffer(&constant_buffer_desc, &constant_buffer_data, &directx->constant_buffer);
    if(FAILED(create_constant_buffer_hr)) {
        LOG_ERROR("Cannot create constant buffer");
        return false;
    }
    directx->immediate_context->VSSetConstantBuffers(0, 1, &directx->constant_buffer);
    
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

    swap_chain_desc.SampleDesc.Count = 1; //count=1,quality=0 turns off multisampling
    swap_chain_desc.SampleDesc.Quality = 0;

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
        LOG_ERROR("Cannot create Direct3D device");
        return false;
    }
    
    // get pointer to the back buffer
    HRESULT get_buffer_hr = directx->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&directx->back_buffer);
    if(FAILED(get_buffer_hr)) {
        LOG_ERROR("Cannot retrieve back buffer location");
        return false;
    }
    
    // create a render target view
    HRESULT create_view_hr = directx->device->CreateRenderTargetView(directx->back_buffer, 0, &directx->render_target);    
    if(FAILED(create_view_hr)) {
        LOG_ERROR("Cannot create render target view");
        return false;
    }
    

    // load shaders
    ID3DBlob *vs_blob = (ID3DBlob *)malloc(sizeof(*vs_blob));
    ID3DBlob *ps_blob = (ID3DBlob *)malloc(sizeof(*ps_blob));
    HRESULT compile_vs_hr = D3DCompileFromFile(L"../code/unified_shader.HLSL", 0, 0, "vs_main", "vs_4_0", 0, 0, &vs_blob, 0);
    HRESULT compile_ps_hr = D3DCompileFromFile(L"../code/unified_shader.HLSL", 0, 0, "ps_main", "ps_4_0", 0, 0, &ps_blob, 0);
    if(FAILED(compile_vs_hr)) {
        LOG_ERROR("Cannot compile vertex shader");
        if(FAILED(compile_ps_hr)) {
            LOG_ERROR("Cannot compile pixel shader");
        }
        return false;
    }

    directx->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &directx->vertex_shader);
    directx->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &directx->pixel_shader);

    directx->immediate_context->VSSetShader(directx->vertex_shader, 0, 0);
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
        LOG_ERROR("Cannot create input layout");
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
        raster_desc.MultisampleEnable = 0;
        raster_desc.AntialiasedLineEnable = 0;
    
        HRESULT raster_state_hr = directx->device->CreateRasterizerState(&raster_desc, &directx->rasterizer_state);
        if(FAILED(raster_state_hr)) {
            LOG_ERROR("Cannot configurate rasterizer state");
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
        depth_stencil_texture_desc.SampleDesc.Count = 1;
        depth_stencil_texture_desc.SampleDesc.Quality = 0;
        depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
        depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depth_stencil_texture_desc.CPUAccessFlags = 0;
        depth_stencil_texture_desc.MiscFlags = 0;
    
        HRESULT create_depth_stencil_hr = directx->device->CreateTexture2D(&depth_stencil_texture_desc, NULL, &directx->depth_stencil_texture);
        if(FAILED(create_depth_stencil_hr)) {
            LOG_ERROR("Cannot create depth stencil texture");
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
            LOG_ERROR("Cannot create depth stencil state");
            return false;
        }
        directx->immediate_context->OMSetDepthStencilState(depth_stencil_state, 1);
   
        D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
        depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depth_stencil_view_desc.Flags = 0;
        depth_stencil_view_desc.Texture2D.MipSlice = 0;

        HRESULT create_depth_stencil_view_hr = directx->device->CreateDepthStencilView(directx->depth_stencil_texture, &depth_stencil_view_desc, &directx->depth_stencil_view);
        if(FAILED(create_depth_stencil_view_hr)) {
            LOG_ERROR("Cannot create depth stencil view");
            return false;
        }
    }
    directx->immediate_context->OMSetRenderTargets(1, &directx->render_target, directx->depth_stencil_view);

    directx->camera.position = vec3(0.0f, 0.0f, 0.0f);
    directx->camera.direction = vec3(0.0f, 0.0f, 1.0f);
    directx->camera.up = vec3(0.0f, 1.0f, 0.0f);
    
    return true;
}

