
cbuffer ShaderConstants : register(b0) {
    matrix world_matrix;
    matrix view_matrix;
    matrix projection_matrix;
};

struct VS_IN {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
};

struct VS_OUT {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
};

struct GS_OUT {
    float4 position : SV_POSITION;
    float3 normal : TEXCOORD0;
    float3 texcoord : TEXCOORD1;
};

VS_OUT vs_main(VS_IN input) {
    VS_OUT output;
    output.position = float4(input.position.x, -input.position.y, input.position.z, 1.0f);
    output.position = mul(output.position, view_matrix);
    output.position = mul(output.position, transpose(projection_matrix));
    output.normal = float3(input.normal.x, -input.normal.y, input.normal.z);
    output.texcoord = input.texcoord;
    
    return output;
}

[maxvertexcount(3)]
void gs_main(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> triangle_stream) {
    GS_OUT output;
    for(int i = 2; i >= 0; i--) {
        output.position = input[i].position;
        output.normal = input[i].normal;
        output.texcoord = input[i].texcoord;
        triangle_stream.Append(output);
    }
}

float4 ps_main(GS_OUT input) : SV_TARGET {
    float4 color = float4(0.5f,0.5f,0.7f,1.0f);
    float3 light_direction = float3(0.5f, 0.0f, 0.5f);
    float3 intensity = dot(light_direction, normalize(input.normal));
    color = float4(abs(intensity.x), abs(intensity.y), abs(intensity.z), 1.0f);
    return color;
}
