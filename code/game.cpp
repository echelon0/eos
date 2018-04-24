
#include "grid.cpp"

struct GameState {
    Array<Entity> entities;
    Grid grid;
};
float temp_global_cam_y;

void game_update(GameState *game_state, D3D_RESOURCE *directx) {
    if(global_input.W_KEY) {
        directx->camera.position += directx->camera.direction * 0.005f;
    }
    if(global_input.S_KEY) {
        directx->camera.position -= directx->camera.direction * 0.005f;
    }
    if(global_input.A_KEY) {
        directx->camera.position += cross(directx->camera.direction, directx->camera.up) * 0.005f;
    }
    if(global_input.D_KEY) {
        directx->camera.position -= cross(directx->camera.direction, directx->camera.up) * 0.005f;
    }
    if(global_input.SPACE_BAR && !global_input.SHIFT_KEY) {
        directx->camera.position += directx->camera.up * 0.08f;
    }
    if(global_input.SPACE_BAR && global_input.SHIFT_KEY) {
        directx->camera.position -= directx->camera.up * 0.08f;
    }
    
    if(global_input.RIGHT_ARROW) {
        rotate(&directx->camera.direction, 0.005f, Y_AXIS);
    }
    if(global_input.LEFT_ARROW) {
        rotate(&directx->camera.direction, -0.005f, Y_AXIS);
    }

    { //adjust camera above above the ground
        float camera_height = 1.25f;
        
        float forward_push = 0.01f;
        vec3 adjusted_camera_position = directx->camera.position + directx->camera.direction * forward_push;
        int cell_index = get_cell_index(&game_state->grid, find_appropriate_cell(game_state->grid.cell_radius, vec2(adjusted_camera_position.x, adjusted_camera_position.z)));
        if(cell_index != -1) {
            for(int i = 0; i < game_state->grid.cells[cell_index].vertex_attributes.size; i+=3) {
                vec3 intersection;
                if(ray_intersects_triangle(adjusted_camera_position, vec3(0.0f, -1.0f, 0.0f),
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 0].position,
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 1].position,
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 2].position,
                                           intersection)) {
                    vec3 ro_to_intersection = intersection - directx->camera.position;
                    float t = -ro_to_intersection.y;
                    temp_global_cam_y = t;
                    directx->camera.position.y -= (t - camera_height);
                    break;
                }
            }
        }
    }

    //temp_global_cam_y = directx->camera.position.y;

    /*
    float camera_height = 1.25f;
    for(int entity_index = 0; entity_index < game_state->entities.size; entity_index++) {
        for(int i = 0; i < game_state->entities[entity_index].model.vertex_attributes.size; i+=3) {    
            vec3 intersection;
            if(ray_intersects_triangle(directx->camera.position + directx->camera.direction * 0.01f, vec3(0.0f, -1.0f, 0.0f),
                                       game_state->entities[entity_index].model.vertex_attributes[i + 0].position,
                                       game_state->entities[entity_index].model.vertex_attributes[i + 1].position,
                                       game_state->entities[entity_index].model.vertex_attributes[i + 2].position,
                                       intersection)) {
                vec3 ro_to_intersection = intersection - directx->camera.position;
                float t = abs(ro_to_intersection.y);
                temp_global_cam_y = t;
                directx->camera.position.y -= (t - camera_height);
                break;
            }
        }
    }
    */
}
