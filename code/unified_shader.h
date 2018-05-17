
cbuffer ShaderConstants : register(b0) {
    matrix world_matrix;
    matrix view_matrix;
    matrix projection_matrix;
    bool wire_frame_on;
    uint padding0;
    uint padding1;
    uint padding2;
};

cbuffer MaterialConstants : register(b1) {
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float exponent;
    float dissolve;
    uint illum_model;
    uint iTime;
};

struct Light {
    float4 position;
    float4 direction;
    float4 color;
    float4 cone_angle;
    uint light_type;
    bool enabled;
    uint padding3;
    uint padding4;
};

#define MAX_LIGHTS 64
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOTLIGHT 2
cbuffer LightConstants : register(b2) {
    Light lights[MAX_LIGHTS];
    float4 eye_position;
};

struct VS_IN {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
};

struct VS_OUT {
    float4 position : SV_POSITION;
    float4 world_space_pos : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
    bool wire_frame_on : FOG;
};

struct GS_OUT {
    float4 position : SV_POSITION;
    float4 world_space_pos : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
    bool wire_frame_on : FOG;
    float3 dist : DEPTH; // distance to opposite edge
};

VS_OUT vs_main(VS_IN input) {
    VS_OUT output;
    output.position = float4(input.position.xyz, 1.0f);
    output.position = mul(output.position, view_matrix);
    output.position = mul(output.position, transpose(projection_matrix));
    output.world_space_pos = float4(input.position.xyz, 1.0f); //////
    output.normal = float3(input.normal.x, input.normal.y, input.normal.z);
    output.normal = normalize(output.normal);
    output.texcoord = input.texcoord;
    output.wire_frame_on = wire_frame_on;
    
    return output;
}

[maxvertexcount(3)]
void gs_main(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> triangle_stream) {
    GS_OUT output;

    float3 height[3];
    if(input[0].wire_frame_on) {
        float3 vertex[3];
        vertex[0] = float3(input[0].position.x, input[0].position.y, input[0].position.z);
        vertex[1] = float3(input[1].position.x, input[1].position.y, input[1].position.z);
        vertex[2] = float3(input[2].position.x, input[2].position.y, input[2].position.z);

        float edge[3];
        edge[0] = distance(vertex[0], vertex[1]);
        edge[1] = distance(vertex[1], vertex[2]);
        edge[2] = distance(vertex[2], vertex[0]);
        float half_perimeter = (edge[0] + edge[1] + edge[2]) / 2.0f;
        float area = sqrt(half_perimeter * (half_perimeter - edge[0]) * (half_perimeter - edge[1]) * (half_perimeter - edge[2]));

        //float3 height[3];
        height[0] = float3((2 * area) / edge[1], 0.0f, 0.0f);
        height[1] = float3(0.0f, (2 * area) / edge[2], 0.0f);
        height[2] = float3(0.0f, 0.0f, (2 * area) / edge[0]);
    }
    
    for(int i = 2; i >= 0; i--) {
        output.position = input[i].position;
        output.world_space_pos = input[i].world_space_pos;
        output.normal = input[i].normal;
        output.texcoord = input[i].texcoord;
        output.wire_frame_on = input[i].wire_frame_on;
        output.dist = height[i];
        triangle_stream.Append(output);
    }
}

float4 ps_main(GS_OUT input) : SV_TARGET {
    float4 color;
    for(int i = 0; i < MAX_LIGHTS; i++) {
        if(lights[i].enabled) {
            float3 light_vector = (lights[i].position - input.world_space_pos).xyz;
            float dist_to_light = length(light_vector);
            normalize(light_vector);
            float brightness = 150.0f;
            float intensity = (1.0f / pow(dist_to_light, 2)) * brightness;

            //diffuse
            float light_proj = max(0, dot(input.normal, light_vector));
            float4 diffuse_intensity = lights[i].color * light_proj * intensity; 
            
            //specular
            float3 reflection = normalize(reflect(-light_vector, input.normal));
            float3 eye_vector = normalize(eye_position - input.world_space_pos);
            float eye_proj = abs(dot(reflection, eye_vector));
            float4 specular_intensity = lights[i].color * pow(eye_proj, exponent) * intensity;

            color = ambient + diffuse * diffuse_intensity + specular * specular_intensity;
            
            switch(lights[i].light_type) {
                case DIRECTIONAL_LIGHT: {
                } break;
                
                case POINT_LIGHT: {
                } break;
                
                case SPOTLIGHT: {
                } break;
            }
        }
    }

    // solid wire frame
    if(input.wire_frame_on) {
        float min_dist = min(input.dist.x, input.dist.y);
        min_dist = min(min_dist, input.dist.z);
        float line_width = 0.01f;
        float AA_threshold = 0.2f;
        if(min_dist < line_width + AA_threshold) {
            float4 line_color = float4(1.0f, 1.0f, 1.0f, 1.0f); 
            if(min_dist > line_width) {
                float line_intensity = pow(2.0f, -2.0f * pow(2.0 * (min_dist - line_width) / AA_threshold, 2.0f));
                if(line_intensity > 0.0001f)
                    color = line_intensity * line_color + (1.0f - line_intensity) * color;
            } else {
                color = line_color;
            }
        }
    }

    color = pow(color, (1.0f / 2.2f));
    return color;
}
