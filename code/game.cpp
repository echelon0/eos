
#define RIGHT_EULER_ANGLE (-(f32)PI / 4.0f)
#define DOWN_EULER_ANGLE (RIGHT_EULER_ANGLE + ((f32)PI / 2.0f))
#define UP_EULER_ANGLE (RIGHT_EULER_ANGLE + ((f32)PI * (3.0f / 2.0f)))
#define LEFT_EULER_ANGLE (RIGHT_EULER_ANGLE + (f32)PI)

#include "grid.cpp"

struct Player {
    f32 current_speed;
    f32 walk_speed;
    f32 run_speed;
    f32 target_speed;
    quat target_orientation;
};

struct GameState {
    bool initialized;
    Array<Entity> entities;
    Light lights[MAX_LIGHTS] = {0};
    int num_lights;
    Grid grid;
    Camera camera;
    f32 player_target_y;
    vec2 current_cam_rotation;
    vec2 target_cam_rotation;
    Player player;
};

void init_game_state(GameState *game_state) {
    game_state->camera.position = vec3(-25.0f, 30.0f, -25.0f);
//    game_state->camera.position = vec3();
    game_state->camera.direction = vec3(0.0f, 0.0f, 1.0f);
    game_state->camera.up = vec3(0.0f, 1.0f, 0.0f);

//    game_state->camera.rotate(dtr(45.0f), Y_AXIS);
    game_state->camera.rotate(dtr(45.0f), CAMERA_RIGHT);
        
    game_state->player.walk_speed = 0.08f;
    game_state->player.run_speed = 0.1f;
    game_state->player.current_speed = game_state->player.walk_speed;
    game_state->player.target_speed = game_state->player.current_speed;
    game_state->entities[0].terminal_velocity = 0.2f;

    game_state->initialized = true;
}

void game_update(GameState *game_state, D3D_RESOURCES *directx) {
    
    { //player movement
        game_state->entities[0].acceleration = vec3();
        vec3 target_orientation_euler_angles = vec3();
        if(global_input.SHIFT_KEY) {
            game_state->player.target_speed = game_state->player.run_speed;
        } else {
            game_state->player.target_speed = game_state->player.walk_speed;;
        }
        game_state->player.current_speed = lerp(game_state->player.current_speed, game_state->player.target_speed, 0.1f);

        int num_keys_down = 0;
        f32 diagonal_speed_multiplier = 1.0f;
        if(global_input.W_KEY) {
            num_keys_down++;
            if(global_input.A_KEY || global_input.D_KEY) {
                diagonal_speed_multiplier = (f32)sin((f32)PI / 4.0f);
            }
            game_state->entities[0].acceleration += vec3(game_state->camera.direction.x, 0.0f, game_state->camera.direction.z) * game_state->player.current_speed * diagonal_speed_multiplier;
            if(global_input.D_KEY)
                target_orientation_euler_angles.x += ( UP_EULER_ANGLE - ((f32)PI * 2.0f)); //shortest path (needed for diagonal movement)
            else
                target_orientation_euler_angles.x += UP_EULER_ANGLE;
        }
        if(global_input.S_KEY) {
            num_keys_down++;
            if(global_input.A_KEY || global_input.D_KEY) {
                diagonal_speed_multiplier = (f32)sin((f32)PI / 4.0f);
            }
            game_state->entities[0].acceleration += vec3(-game_state->camera.direction.x, 0.0f, -game_state->camera.direction.z) * game_state->player.current_speed * diagonal_speed_multiplier;
            target_orientation_euler_angles.x += DOWN_EULER_ANGLE;
        }
        if(global_input.A_KEY) {
            num_keys_down++;
            game_state->entities[0].acceleration += cross(vec3(game_state->camera.direction.x, 0.0f, game_state->camera.direction.z), game_state->camera.up) * game_state->player.current_speed * diagonal_speed_multiplier;
            target_orientation_euler_angles.x += LEFT_EULER_ANGLE;           
        }
        if(global_input.D_KEY) {
            num_keys_down++;
            game_state->entities[0].acceleration += cross(vec3(-game_state->camera.direction.x, 0.0f, -game_state->camera.direction.z), game_state->camera.up) * game_state->player.current_speed * diagonal_speed_multiplier;
            target_orientation_euler_angles.x += RIGHT_EULER_ANGLE;          
        }
        game_state->entities[0].acceleration = normalize(game_state->entities[0].acceleration);
        if(global_input.MOVEMENT_KEY_DOWN) {
            quat target_orientation = quat_from_euler_angles(target_orientation_euler_angles / (f32)num_keys_down);
            float rotation_speed = 0.15f;
            //game_state->entities[0].orientation = shortest_lerp(game_state->entities[0].orientation, target_orientation, rotation_speed);
        }
        
    }
    
    //apply acceleration
    for(int entity_index = 0; entity_index < game_state->entities.size; entity_index++) {
        game_state->entities[entity_index].velocity += game_state->entities[entity_index].acceleration;
        if(magnitude(game_state->entities[entity_index].velocity) > game_state->entities[entity_index].terminal_velocity) {
            game_state->entities[entity_index].velocity = normalize(game_state->entities[entity_index].velocity) * game_state->entities[entity_index].terminal_velocity;
        }
    }

    //apply velocity
    for(int entity_index = 0; entity_index < game_state->entities.size; entity_index++) {
        game_state->entities[entity_index].world_pos += game_state->entities[entity_index].velocity;
    }

    //friction
    for(int entity_index = 0; entity_index < game_state->entities.size; entity_index++) {
        vec3 friction = game_state->entities[entity_index].velocity * 0.2f;
        game_state->entities[entity_index].velocity -= friction;
        float EPSILON = 0.000001f;
        if(magnitude(game_state->entities[entity_index].velocity) <= EPSILON) {
            game_state->entities[entity_index].velocity = vec3();
        }
    }

    //rotation
    for(int entity_index = 0; entity_index < game_state->entities.size; entity_index++) {
        if(magnitude(game_state->entities[entity_index].velocity) != 0.0f) {
            quat target_orientation;
            vec3 source = vec3(0.0f, 0.0f, -1.0f);
            vec3 destination = normalize(game_state->entities[entity_index].velocity);
    
            vec3 axis_of_rotation = cross(source, destination);
            if(axis_of_rotation == vec3()) {
                target_orientation = quat();
            } else {
            vec3 normalized_axis = normalize(axis_of_rotation);
            f32 sine_theta = find_scalar_multiple(axis_of_rotation, normalized_axis); //NOTE: potential rotation bug
            f32 angle = asinf(sine_theta);
            f32 dot_sign = sign(dot(source, destination));
            if((dot_sign < 0.0f)) {
                if((angle >= 0.0f) && (angle < (f32)PI / 2.0f)) {
                    angle = 2.0f * ((f32)PI / 2.0f - angle) + angle;
                } else if((angle <= 0.0f) && (angle > -(f32)PI / 2.0f)) {
                    angle = 2.0f * ((f32)PI / 2.0f + angle) + 3.0f * (f32)PI / 2.0f;                
                }
            }
            
            target_orientation.x = normalized_axis.x * sinf(angle / 2.0f);
            target_orientation.y = normalized_axis.y * sinf(angle / 2.0f);
            target_orientation.z = normalized_axis.z * sinf(angle / 2.0f);
            target_orientation.w = cosf(angle / 2.0f);
            target_orientation = normalize(target_orientation);
            }
            
            float rotation_speed = 0.2f;
            game_state->entities[0].orientation = shortest_lerp(game_state->entities[0].orientation, target_orientation, rotation_speed);
        }
    }

    float cam_to_player_dist = 20.0f;
    game_state->camera.position = game_state->entities[0].world_pos;
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
            f32 z_angle = (f32)acos(dot(vec3(0.0f, 1.0f, 0.0f), game_state->camera.direction));
            
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
        game_state->camera.rotate(game_state->current_cam_rotation.x, Y_AXIS);
        game_state->camera.rotate(game_state->current_cam_rotation.y, CAMERA_RIGHT);
    }
    game_state->camera.position -= (game_state->camera.direction * cam_to_player_dist);
    float right_offset = 1.0f;
    game_state->camera.position += (cross(game_state->camera.up, game_state->camera.direction) * right_offset);
    

    { //adjust player above above the ground
        f32 player_height = 1.25f;
        f32 forward_push = 0.0f;
        
        vec3 adjusted_player_position = game_state->entities[0].world_pos;
        int cell_index = get_cell_index(&game_state->grid, find_appropriate_cell(game_state->grid.cell_radius, vec2(game_state->entities[0].world_pos.x, game_state->entities[0].world_pos.z)));
        if(cell_index != -1) {
            f32 t = 999999.0f;
            for(int i = 0; i < game_state->grid.cells[cell_index].vertex_attributes.size; i+=3) {
                vec3 intersection;
                if(ray_intersects_triangle(game_state->entities[0].world_pos, vec3(0.0f, -1.0f, 0.0f),
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 0].position,
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 1].position,
                                           game_state->grid.cells[cell_index].vertex_attributes[i + 2].position,
                                           intersection)) {
                    vec3 ro_to_intersection = intersection - game_state->entities[0].world_pos;
                    if((abs(-ro_to_intersection.y) < t) && (ro_to_intersection.y < 0)) {
                        t = -ro_to_intersection.y;
                    }
                }
            }
            if(t != 999999.0f) {
                game_state->player_target_y = game_state->entities[0].world_pos.y - (t - player_height);
            }
        }

        if((game_state->entities[0].world_pos.y != game_state->player_target_y)) {
            game_state->entities[0].world_pos.y = lerp(game_state->entities[0].world_pos.y, game_state->player_target_y, 0.2f);
        }
    }   

}
