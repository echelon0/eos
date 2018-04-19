
struct Cell {
    vec3 center;
    Array<VertexAttribute> vertex_attributes;
};

struct Grid {
    float cell_radius;
    Array<Cell> cells;
};


int get_cell_index(Grid *grid, vec3 cell_center) {
    for(int i = 0; i < grid->cells.size; i++) {
        if(grid->cells[i].center == cell_center) {
            return i;
        }
    }
    return -1;
}

vec3 find_appropriate_cell(float cell_radius, vec3 position) {
    return vec3(floor(position.x / cell_radius) * (cell_radius*2),
                floor(position.y / cell_radius) * (cell_radius*2),
                floor(position.z / cell_radius) * (cell_radius*2));    
}

void init_grid(Grid *grid, Array<Entity> &entities) {
    grid->cell_radius = 2.0f; //TODO: Make this value changable with imgui
    for(int entity_index = 0; entity_index < entities.size; entity_index++) {
        for(int i = 0; i < entities[entity_index].model.vertex_attributes.size; i+=3) {
            
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
                
            vec3 min_cell_center = find_appropriate_cell(grid->cell_radius, min);
            vec3 max_cell_center = find_appropriate_cell(grid->cell_radius, max);
            
            for(float x = min_cell_center.x; x <= max_cell_center.x; x += grid->cell_radius*2) {
                for(float y = min_cell_center.y; y <= max_cell_center.y; y += grid->cell_radius*2) {
                    for(float z = min_cell_center.z; z <= max_cell_center.z; z += grid->cell_radius*2) {
                        int possible_cell_index = get_cell_index(grid, vec3(x, y, z));
                        if(possible_cell_index == -1) {
                            Cell new_cell;
                            new_cell.center = vec3(x, y, z);
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
    
}

