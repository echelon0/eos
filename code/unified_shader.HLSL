
cbuffer ShaderConstants : register(b0) {
    matrix world_matrix;
    matrix view_matrix;
    matrix projection_matrix;
};

struct VInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
};

struct VOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
};

VOutput vs_main(VInput input) {
    VOutput output;
    output.position = float4(input.position.x, input.position.y, input.position.z, 1.0f);
    output.position = mul(output.position, view_matrix);
    output.position = mul(output.position, transpose(projection_matrix));
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    
    return output;
}


float4 ps_main(VOutput input) : SV_TARGET {
    float4 color = float4(0.5f,0.5f,0.7f,1.0f);
    float3 light_direction = float3(0.5f, 0.0f, 0.5f);
    float3 intensity = dot(light_direction, normalize(input.normal));
    color = float4(abs(intensity.x), abs(intensity.y), abs(intensity.z), 1.0f);
    return color;
}
