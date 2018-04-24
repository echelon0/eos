
struct Cell {
    vec2 center;
    Array<VertexAttribute> vertex_attributes;
};

struct Grid {
    float cell_radius;
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

vec2 find_appropriate_cell(float cell_radius, vec2 position) { //expects a position on the xz-plane
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
    grid->cell_radius = 2.0f; //TODO: Make this value changable with imgui
    for(int entity_index = 0; entity_index < entities.size; entity_index++) {
        for(int i = 0; i < entities[entity_index].model.vertex_attributes.size; i+=3) {

            //TODO: find a 2d bounding box directly
            //find bouding box of triangle
            vec3 min = entities[entity_index].model.vertex_attributes[i + 0].position;
            vec3 max = entities[entity_index].model.vertex_attributes[i + 0].position;
            for (int v = i; v < 3; v++) {
                if(entities[entity_index].model.vertex_attributes[v].position.x < min.x) min.x = entities[entity_index].model.vertex_attributes[v].position.x;
                if(entities[entity_index].model.vertex_attributes[v].position.y < min.y) min.y = entities[entity_index].model.vertex_attributes[v].position.y;
                if(entities[entity_index].model.vertex_attributes[v].position.z < min.z) min.z = entities[entity_index].model.vertex_attributes[v].position.z;
                if(entities[entity_index].model.vertex_attributes[v].position.x > max.x) max.x = entities[entity_index].model.vertex_attributes[v].position.x;
                if(entities[entity_index].model.vertex_attributes[v].position.y > max.y) max.y = entities[entity_index].model.vertex_attributes[v].position.y;
                if(entities[entity_index].model.vertex_attributes[v].position.z > max.z) max.z = entities[entity_index].model.vertex_attributes[v].position.z;
            }
                
            vec2 min_cell_center = find_appropriate_cell(grid->cell_radius, vec2(min.x, min.z));
            vec2 max_cell_center = find_appropriate_cell(grid->cell_radius, vec2(max.x, max.z));
            
            for(float x = min_cell_center.x; x <= max_cell_center.x; x += grid->cell_radius*2) {
                for(float z = min_cell_center.y; z <= max_cell_center.y; z += grid->cell_radius*2) {
                    int possible_cell_index = get_cell_index(grid, vec2(x, z));
                        if(possible_cell_index == -1) {
                            Cell new_cell;
                            new_cell.center = vec2(x, z);
                            new_cell.vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 0]);
                            new_cell.vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 1]);
                            new_cell.vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 2]);
                            grid->cells.push_back(new_cell);

                        } else {
                            grid->cells[possible_cell_index].vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 0]);
                            grid->cells[possible_cell_index].vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 1]);
                            grid->cells[possible_cell_index].vertex_attributes.push_back(entities[entity_index].model.vertex_attributes[i + 2]);                            
                        }
                }
            }
            
        }
    }
    
}

