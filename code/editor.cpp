
namespace editor {

    //NOTE: Use a hardware accelerated picking method such as rendering entity IDs to a seperate buffer for non-editing use
    int get_picked_entity_index(Camera camera, vec3 up, ivec2 point_in_client, ivec2 client_dim, float FOV, float aspect_ratio, Array<Entity> entities) {
        vec2 normalized_client_point = vec2(((float)point_in_client.x / (float)client_dim.x) * 2.0f - 1.0f, //NOTE: [-1, 1]
                                            ((float)point_in_client.y / (float)client_dim.y) * 2.0f - 1.0f);

        float half_FOV = FOV * 0.5f;
        vec2 point_on_frustum = vec2((float)tan(half_FOV) * normalized_client_point.x * aspect_ratio, (float)tan(half_FOV) * normalized_client_point.y);

        vec3 ro = camera.position;
        vec3 rd = vec3(point_on_frustum.x, point_on_frustum.y, 1.0f);

        up = normalize(up);
        vec3 forward = normalize(camera.direction);
        vec3 right = normalize(cross(up, forward));
        mat33 orientation_transpose = mat33(right.x,   right.y,   right.z,
                                            up.x,      up.y,      up.z,
                                            forward.x, forward.y, forward.z);
        
        rd = normalize(rd * orientation_transpose);

        float epsilon = 0.00001f;
        int picked_entity_index = -1;
        vec3 intersection;

        for(int entity_index = 0; entity_index < entities.size; entity_index++) {
            for(int vertex_attribute_index = 0; vertex_attribute_index < entities[entity_index].model.vertex_attributes.size; vertex_attribute_index+=3) {
                if(ray_intersects_triangle(ro, rd,
                                           entities[entity_index].model.vertex_attributes[vertex_attribute_index + 0].position,
                                           entities[entity_index].model.vertex_attributes[vertex_attribute_index + 1].position,
                                           entities[entity_index].model.vertex_attributes[vertex_attribute_index + 2].position,
                                           intersection)) {
                    vec3 ro_to_intersection = intersection - ro;
                    float t = ro_to_intersection.x / rd.x;
                    if(t > 0.0f) {
                        picked_entity_index = entity_index;
                        break;
                    }
                }
            }
        }

        return picked_entity_index;
    }

}
