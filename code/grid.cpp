
struct Cell {
    vec2 center;
    Array<VertexAttribute> vertex_attributes;
};

struct Grid {
    f32 cell_radius;
    Array<Cell> cells;
};

int get_cell_index(Grid *grid, vec2 cell_center) {
    for(int i = 0; i < grid->cells.size; i++) {
        if(grid->cells[i].center == cell_center) {
            return i;
        }
    }
    return -1;
}

vec2 find_appropriate_cell(f32 cell_radius, vec2 position) { //expects a position on the xz-plane
    vec2 result;
    if(position.x < 0)
        result.x = ceil(position.x / cell_radius) * (cell_radius*2);
    else if(position.x >= 0)
        result.x = floor(position.x / cell_radius) * (cell_radius*2);
    
    if(position.y < 0)
        result.y = ceil(position.y / cell_radius) * (cell_radius*2);
    else if(position.y >= 0)
        result.y = floor(position.y / cell_radius) * (cell_radius*2);

    return result;
}

void init_grid(Grid *grid, Array<Entity> &entities) {
    int iter_count = 0;
    grid->cell_radius = 2.0f; //TODO: Make this value changable with imgui
    VertexAttribute offset_vertex_attributes[3];
    for(int entity_index = 1; entity_index < entities.size; entity_index++) { //TODO possible bug
        for(int i = 0; i < entities[entity_index].model.vertex_attributes.size; i+=3) {

            //find bouding box of triangle
            //NOTE: y coordinate in vec2 represents a z coordinate in world space
            for(int index = 0; index < 3; index++) {
                offset_vertex_attributes[index] = entities[entity_index].model.vertex_attributes[i + index];
                offset_vertex_attributes[index].position += entities[entity_index].world_pos;
            }
            
            vec2 min = vec2(offset_vertex_attributes[0].position.x, offset_vertex_attributes[0].position.z);
            vec2 max = min;
            for (int index = 0; index < 3; index++) {
                if(offset_vertex_attributes[index].position.x < min.x) min.x = offset_vertex_attributes[index].position.x;
                if(offset_vertex_attributes[index].position.z < min.y) min.y = offset_vertex_attributes[index].position.z;
                
                if(offset_vertex_attributes[index].position.x > max.x) max.x = offset_vertex_attributes[index].position.x;
                if(offset_vertex_attributes[index].position.z > max.y) max.y = offset_vertex_attributes[index].position.z;
            }
            
            vec2 min_cell_center = find_appropriate_cell(grid->cell_radius, vec2(min.x, min.y));
            vec2 max_cell_center = find_appropriate_cell(grid->cell_radius, vec2(max.x, max.y));
            
            for(f32 x = min_cell_center.x; x <= max_cell_center.x; x += grid->cell_radius*2) {
                for(f32 z = min_cell_center.y; z <= max_cell_center.y; z += grid->cell_radius*2) {
                    int possible_cell_index = get_cell_index(grid, vec2(x, z));
                        if(possible_cell_index == -1) {
                            Cell new_cell;
                            new_cell.center = vec2(x, z);
                            new_cell.vertex_attributes.push_back(offset_vertex_attributes[0]);
                            new_cell.vertex_attributes.push_back(offset_vertex_attributes[1]);
                            new_cell.vertex_attributes.push_back(offset_vertex_attributes[2]);
                            grid->cells.push_back(new_cell);

                        } else {
                            grid->cells[possible_cell_index].vertex_attributes.push_back(offset_vertex_attributes[0]);
                            grid->cells[possible_cell_index].vertex_attributes.push_back(offset_vertex_attributes[1]);
                            grid->cells[possible_cell_index].vertex_attributes.push_back(offset_vertex_attributes[2]);                            
                        }
                }
            }
            
            iter_count++;
            if(iter_count > entities[0].model.vertex_attributes.size + entities[1].model.vertex_attributes.size + 10) {
                LOG_ERROR("ERROR", "Exceeded expected iteration count");
                break;
            }
        }
    }
}

