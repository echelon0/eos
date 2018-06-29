
struct VS_IN {
    uint entity_id : TEXCOORD;
};

struct VS_OUT {
    uint entity_id : TEXCOORD;
};

VS_OUT vs_main(VS_IN input) {
    VS_OUT output;
    output.entity_id = input.entity_id;
    return output;
}

float4 ps_main(GS_OUT input) : SV_TARGET {
    return float4(input.entity_id, input.entity_id, input.entity_id, input.entity_id);
}
