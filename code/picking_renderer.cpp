
struct PICKING_D3D_RESOURCES {
    D3D_FEATURE_LEVEL feature_level;
    ID3D11Device *device;
    ID3D11DeviceContext *immediate_context;
    ID3D11Texture2D *picking_texture;
    ID3D11RenderTargetView *render_target;
    ID3D11VertexShader *vertex_shader;
    ID3D11PixelShader *pixel_shader;
    ID3D11Buffer *vertex_buffer;
};

bool init_picking_D3D(PICKING_D3D_RESOURCES *directx_picking_picking) {
    HMODULE Direct3D_module_handle = LoadLibraryA("D3D11.dll");    

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Width = 0;
    swap_chain_desc.BufferDesc.Height = 0;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 1; //refresh rate for vsync
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 60;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swap_chain_desc.SampleDesc.Count = 4; //count=1,quality=0 turns off multisampling
    swap_chain_desc.SampleDesc.Quality = 16;

    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.Windowed = true; //TODO(Alex): IDXGISwapChain::SetFullscreenState
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    HRESULT device_and_swap_chain_hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE,
                                                      0, 0,
                                                      0, 0,
                                                      D3D11_SDK_VERSION, &swap_chain_desc,
                                                      &directx->swap_chain, &directx->device,
                                                      
&directx->feature_level, &directx->immediate_context);
    if(FAILED(device_and_swap_chain_hr)) {
        LOG_ERROR("ERROR", "Cannot create Direct3D device and swap chain");
        return false;
    }
    
    // create a render target view
    HRESULT create_view_hr = directx_picking_picking->device->CreateRenderTargetView(directx_picking_picking->picking_texture, 0, &directx_picking_picking->render_target);
    if(FAILED(create_view_hr)) {
        LOG_ERROR("ERROR", "Cannot create render target view");
        return false;
    }
    
    // load shaders
    ID3DBlob *vs_blob = (ID3DBlob *)malloc(sizeof(*vs_blob));
    ID3DBlob *ps_blob = (ID3DBlob *)malloc(sizeof(*ps_blob));
    UINT shader_compiler_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    ID3DBlob *error_message;
    bool should_return = false;
    
    HRESULT compile_vs_hr = D3DCompileFromFile(L"../code/unified_picking_shader.h", 0, 0, "vs_main", "vs_4_0", shader_compiler_flags, 0, &vs_blob, &error_message);
    if(FAILED(compile_vs_hr)) {
        LOG_ERROR("ERROR: Cannot compile vertex shader", (char *)error_message->GetBufferPointer());
        should_return = true;
    }
    
    HRESULT compile_ps_hr = D3DCompileFromFile(L"../code/unified_picking_shader.h", 0, 0, "ps_main", "ps_4_0", shader_compiler_flags, 0, &ps_blob, &error_message);
    if(FAILED(compile_ps_hr)) {
        LOG_ERROR("ERROR: Cannot compile pixel shader", (char *)error_message->GetBufferPointer());
        should_return = true;
    }
    
    if(should_return) return false;
    
    HRESULT create_vs_hr = directx_picking_picking->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &directx_picking_picking->vertex_shader);
    HRESULT create_ps_hr = directx_picking_picking->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &directx_picking_picking->pixel_shader);
    if(FAILED(create_vs_hr)) {
        LOG_ERROR("ERROR", "Cannot create vertex shader");
        should_return = true;
    }
    if(FAILED(create_ps_hr)) {
        LOG_ERROR("ERROR", "Cannot create pixel shader");
        should_return = true;
    }
    if(should_return) return false;
    
    directx_picking_picking->immediate_context->VSSetShader(directx_picking_picking->vertex_shader, 0, 0);
    directx_picking_picking->immediate_context->PSSetShader(directx_picking_picking->pixel_shader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC layout_desc[] = {
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    ID3D11InputLayout *input_layout;
    HRESULT create_layout_hr = directx_picking_picking->device->CreateInputLayout(layout_desc, ARRAYSIZE(layout_desc), vs_blob->GetBufferPointer(),
                                                                  vs_blob->GetBufferSize(), &input_layout);
    if(FAILED(create_layout_hr)) {
        LOG_ERROR("ERROR", "Cannot create input layout");
        return false;
    }
    directx_picking_picking->immediate_context->IASetInputLayout(input_layout);
        
    return true;
}
