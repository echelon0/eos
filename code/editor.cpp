
namespace editor {

    bool add_light(Light lights[], int &num_lights,
                   Array<Entity> entities,  u32 entity_ID,
                   u32 light_type, vec3 pos_offset,
                   f32 intensity, vec3 color,
                   vec3 direction, f32 cone_angle) {
        
        //input check
        if(num_lights == MAX_LIGHTS) {
            LOG_ERROR("ERROR", "Maximum number of lights exceeded when attempting to create light source.");
            return false;
        }
            
        if(light_type != DIRECTIONAL_LIGHT && light_type != POINT_LIGHT && light_type != SPOTLIGHT) {
            LOG_ERROR("ERROR", "Invalid light type when creating light source.");
            return false;
        }
            
        int index = -1;
        for(int i = 0; i < entities.size; i++) {
            if(entities[i].ID == entity_ID) {
                index = i;
                break;
            }
        }
        if(index == -1) {
            LOG_ERROR("ERROR", "Entity does not exist when creating light source.");
            return false;
        }
        
        Light new_light = {};
        new_light.entity_ID = entity_ID;
        new_light.light_type = light_type;
        new_light.position = entities[index].model.vertex_attributes[0].position + pos_offset; //TODO: CHANGE THIS TO ENTITY'S CENTER POSITION
        new_light.color = color;
        new_light.direction = direction;
        new_light.cone_angle = cone_angle;
        new_light.enabled = true;
        
        lights[num_lights++] = new_light;
        
        return true;
    }
    
    //NOTE: This function is depricated, read picking data from D3D_RESOURCES struct directly.
    int get_picked_entity_index(Camera camera, vec3 up, ivec2 point_in_client, ivec2 client_dim, f32 FOV, f32 aspect_ratio, Array<Entity> entities) {
        vec2 normalized_client_point = vec2(((f32)point_in_client.x / (f32)client_dim.x) * 2.0f - 1.0f, //NOTE: [-1, 1]
                                            ((f32)point_in_client.y / (f32)client_dim.y) * 2.0f - 1.0f);

        f32 half_FOV = FOV * 0.5f;
        vec2 point_on_frustum = vec2((f32)tan(half_FOV) * normalized_client_point.x * aspect_ratio, (f32)tan(half_FOV) * normalized_client_point.y);

        vec3 ro = camera.position;
        vec3 rd = vec3(point_on_frustum.x, point_on_frustum.y, 1.0f);

        up = normalize(up);
        vec3 forward = normalize(camera.direction);
        vec3 right = normalize(cross(up, forward));
        mat33 orientation_transpose = mat33(right.x,   right.y,   right.z,
                                            up.x,      up.y,      up.z,
                                            forward.x, forward.y, forward.z);
        
        rd = normalize(rd * orientation_transpose);

        f32 epsilon = 0.00001f;
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
                    f32 t = ro_to_intersection.x / rd.x;
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
