
int get_picked_entity_index(vec3 camera_p, ivec2 point_in_client, ivec2 client_dim, float FOV, float aspect_ratio, Array<Entity> entities) {
    vec2 normalized_client_point = vec2(((float)point_in_client.x / (float)client_dim.x) * 2 - 1, //NOTE: [-1, 1]
                                        ((float)point_in_client.y / (float)client_dim.y) * 2 - 1);

    float half_FOV = FOV * 0.5f;
    vec2 point_on_frustum = vec2((float)tan(half_FOV) * point_in_client.x * aspect_ratio, (float)tan(half_FOV) * point_in_client.y);

    vec3 ro = camera_p;
    //vec3 rd = ;
    
    int entity_index = -1;
    for(int entity_index = 0; entity_index < entities.size; entity_index++) {
        for(int vertex_index = 0; vertex_index < entities[entity_index].model.vertices.size; vertex_index++) {
            
        }
    }

    return entity_index;
}
