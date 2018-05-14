
#include "grid.cpp"

struct Player {
    f32 current_speed;
    f32 walk_speed;
    f32 run_speed;
    f32 target_speed;
};

struct GameState {
    bool initialized;
    Array<Entity> entities;
    Grid grid;
    f32 camera_target_y;
    vec2 current_cam_rotation;
    vec2 target_cam_rotation;
    Player player;
};

void game_update(GameState *game_state, D3D_RESOURCE *directx) {
    if(!game_state->initialized) {
        game_state->player.walk_speed = 0.08f;
        game_state->player.run_speed = 0.15f;
        game_state->player.current_speed = game_state->player.walk_speed;
        game_state->player.target_speed = game_state->player.current_speed;
        
        game_state->initialized = true;
    }

    { //player movement
        if(global_input.SHIFT_KEY) {
            game_state->player.target_speed = game_state->player.run_speed;
        } else {
            game_state->player.target_speed = game_state->player.walk_speed;;
        }
        game_state->player.current_speed = lerp(game_state->player.current_speed, game_state->player.target_speed, 0.1f);

        f32 diagonal_speed_multiplier = 1.0f;
        if(global_input.W_KEY) {
            if(global_input.A_KEY || global_input.D_KEY) {
                diagonal_speed_multiplier = (f32)sin((f32)PI / 4.0f);
            }
            directx->camera.position += directx->camera.direction * game_state->player.current_speed * diagonal_speed_multiplier;
        }
        if(global_input.S_KEY) {
            if(global_input.A_KEY || global_input.D_KEY) {
                diagonal_speed_multiplier = (f32)sin((f32)PI / 4.0f);
            }
            directx->camera.position -= directx->camera.direction * game_state->player.walk_speed * diagonal_speed_multiplier;
        }
        if(global_input.A_KEY) {
            directx->camera.position += cross(directx->camera.direction, directx->camera.up) * game_state->player.current_speed * diagonal_speed_multiplier;
        }
        if(global_input.D_KEY) {
            directx->camera.position -= cross(directx->camera.direction, directx->camera.up) * game_state->player.current_speed * diagonal_speed_multiplier;
        }
    }
    
    { //camera look around
        f32 upper_angle_constraint = dtr(25.0f);
        f32 lower_angle_constraint = dtr(160.0f);
        f32 max_rotation = dtr(15.0f);
        { // left-right rotation
            f32 rotation_angle = global_input.PER_FRAME_DRAG_VECTOR_PERCENT.x;
            if(rotation_angle > max_rotation) {
                rotation_angle = max_rotation;
            }
            game_state->target_cam_rotation.x = rotation_angle;
        }   
        { // up-down rotation
            f32 rotation_angle = global_input.PER_FRAME_DRAG_VECTOR_PERCENT.y;
            if(rotation_angle > max_rotation) {
                rotation_angle = max_rotation;
            }
            f32 z_angle = (f32)acos(dot(vec3(0.0f, 1.0f, 0.0f), directx->camera.direction));
            
            f32 dist_to_upper = z_angle - upper_angle_constraint;
            f32 dist_to_lower = lower_angle_constraint - z_angle;
            if(rotation_angle < 0.0f) {
                if(abs(rotation_angle) > dist_to_upper)
                    rotation_angle = -dist_to_upper;
            } else {
                if(rotation_angle > dist_to_lower)
                    rotation_angle = dist_to_lower;
            }
            game_state->target_cam_rotation.y = rotation_angle;
        }
        f32 cam_rotation_lerp_speed = 0.85f;
        game_state->current_cam_rotation = lerp(game_state->current_cam_rotation, game_state->target_cam_rotation, cam_rotation_lerp_speed);
        directx->camera.rotate(game_state->current_cam_rotation.x, Y_AXIS);
        directx->camera.rotate(game_state->current_cam_rotation.y, CAMERA_RIGHT);
    }

    { //adjust camera above above the ground
        f32 camera_height = 1.25f;
        f32 forward_push = 0.0f;
        
        vec3 adjusted_camera_position = directx->camera.position + directx->camera.direction * forward_push + vec3(0.0f, 0.01f, 0.0f);
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
                    f32 t = -ro_to_intersection.y;
                    game_state->camera_target_y = directx->camera.position.y - (t - camera_height);
                    break;
                }
            }
        }

        if((directx->camera.position.y != game_state->camera_target_y)) {
            directx->camera.position.y = lerp(directx->camera.position.y, game_state->camera_target_y, 0.2f);
        }
    }   

}
