
namespace world_editor {

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


    
}
