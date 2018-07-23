
cbuffer ShaderConstants : register(b0) {
    matrix model_matrix;
    matrix view_matrix;
    matrix projection_matrix;
    bool wire_frame_on;
    int entity_id;
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
    float cone_angle;
    uint light_type;
    bool enabled;
    uint padding3;
    uint padding4;
    float3 padding5;
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
    int entity_id : BLENDINDICES;
};

struct GS_OUT {
    float4 position : SV_POSITION;
    float4 world_space_pos : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
    bool wire_frame_on : FOG;
    int entity_id : BLENDINDICES;
    float3 dist : DEPTH; // distance to opposite edge
};

struct PS_OUT {
    float4 color : SV_Target0;
    int entity_id : SV_Target1;
};

VS_OUT vs_main(VS_IN input) {
    VS_OUT output;
    
    output.position = float4(input.position.xyz, 1.0f);
    output.world_space_pos = output.position;
    
    output.position = mul(output.position, model_matrix);    
    output.position = mul(output.position, view_matrix);
    output.position = mul(output.position, transpose(projection_matrix));

    output.normal = float3(input.normal.x, input.normal.y, input.normal.z);
    output.normal = normalize(output.normal);
    
    output.texcoord = input.texcoord;
    output.wire_frame_on = wire_frame_on;
    output.entity_id = entity_id;
    
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
        output.entity_id = input[i].entity_id;
        output.dist = height[i];
        triangle_stream.Append(output);
    }
}

float4 calc_diffuse(float3 light_vector, float3 surface_normal, float4 light_color) {
    float light_proj = max(0, dot(surface_normal, light_vector));
    float4 diffuse_intensity = light_color * light_proj;
    return diffuse_intensity;
}

float4 calc_specular(float3 light_vector, float3 surface_normal, float3 eye_vector, float4 light_color) {
    float3 reflection = normalize(reflect(-light_vector, surface_normal));
    float eye_proj = abs(dot(reflection, eye_vector));
    float4 specular_intensity = light_color * pow(eye_proj, exponent);
    return specular_intensity;
}

PS_OUT ps_main(GS_OUT input) {
    PS_OUT output;
    float4 color;
    float4 total_diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 total_specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for(int i = 0; i < MAX_LIGHTS; i++) {
        if(lights[i].enabled) {
            
            switch(lights[i].light_type) {
                case DIRECTIONAL_LIGHT: {
                    float3 light_vector = -lights[i].direction.xyz;
                    float3 eye_vector = normalize(eye_position - input.world_space_pos).xyz;
                    total_diffuse += calc_diffuse(light_vector, input.normal, lights[i].color);
                    total_specular += calc_specular(light_vector, input.normal, eye_vector, lights[i].color);
                } break;
                
                case POINT_LIGHT: {
                    float3 light_vector = (lights[i].position - input.world_space_pos).xyz;
                    float dist_to_light = length(light_vector);
                    normalize(light_vector);
                    float brightness = 150.0f;
                    float attenuation = (1.0f / pow(dist_to_light, 2)) * brightness;
                    float3 eye_vector = normalize(eye_position - input.world_space_pos).xyz;
                    
                    total_diffuse += calc_diffuse(light_vector, input.normal, lights[i].color) * attenuation;
                    total_specular += calc_specular(light_vector, input.normal, eye_vector, lights[i].color) * attenuation; 
                } break;
                
                case SPOTLIGHT: {
                    float3 light_vector = (lights[i].position - input.world_space_pos).xyz;
                    float dist_to_light = length(light_vector);
                    normalize(light_vector);
                    float brightness = 150.0f;
                    float attenuation = (1.0f / pow(dist_to_light, 2)) * brightness;

                    float min_cos = cos(lights[i].cone_angle);
                    float max_cos = (min_cos + 1.0f) * 2.0f;
                    float cos_angle = dot(lights[i].direction.xyz, -light_vector);
                    float spot_intensity = smoothstep(min_cos, max_cos, cos_angle);
                    float3 eye_vector = normalize(eye_position - input.world_space_pos).xyz;
                    
                    total_diffuse += calc_diffuse(light_vector, input.normal, lights[i].color) * attenuation * spot_intensity;
                    total_specular += calc_specular(light_vector, input.normal, eye_vector, lights[i].color) * attenuation * spot_intensity;
                } break;
            }
        }
    }
    total_diffuse = saturate(total_diffuse);
    total_specular = saturate(total_specular);    
    color = ambient + diffuse * total_diffuse + specular * total_specular;
    
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

    output.color = pow(color, (1.0f / 2.2f));
    output.entity_id = input.entity_id;
    return output;
}
