

char * read_file_to_buffer(char *path) {
    FILE *file_handle = fopen(path, "r");
    if(!file_handle) {
        LOG_ERROR("ERROR", "Cannot open file path");
        return 0;
    } 
    fseek(file_handle, 0, SEEK_END);
    int file_size = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);        
    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    fread(buffer, sizeof(char), file_size, file_handle);

    return buffer;
}

int scan_word(char *str, char *substr) { //returns chars read
    int count = 0;
    while(*str && (*str != ' ') && (*str != '\n')) {
        *substr++ = *str++;
        count++;
    }
    *substr = '\0';
    return count;
}

StaticModel load_obj(char *path) {
    StaticModel loaded_model = {};
    
    FILE *file_handle = fopen(path, "r");
    if(!file_handle) {
        LOG_ERROR("ERROR", "Cannot open .obj file");
        return loaded_model;
    }

    Array<vec3> positions;
    Array<vec3> normals;
    Array<vec2> texcoords;
    
    vec3 position;
    vec3 normal;
    vec2 texcoord;

    bool has_v = false;
    bool has_vt = false;
    bool has_vn = false;

    char line_id[2048];
    while(fscanf(file_handle, "%s", line_id) != EOF) {
        if(strcmp(line_id, "v") == 0) { //vertex position
            fscanf(file_handle, "%f%f%f\n", &position.x, &position.y, &position.z);
            positions.push_back(position);
            has_v = true;

        } else if(strcmp(line_id, "vt") == 0) { //texcoord
            fscanf(file_handle, "%f%f\n", &texcoord.x, &texcoord.y);
            texcoords.push_back(texcoord);
            has_vt = true;
            
        } else if(strcmp(line_id, "vn") == 0) { //normal
            fscanf(file_handle, "%f%f%f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
            has_vn = true;
            
        } else if(strcmp(line_id, "f") == 0) {
            int v1=0, v2=0, v3=0;
            int vt1=0, vt2=0, vt3=0;
            int vn_placeholder=0;

            if(has_v && has_vt && has_vn) { // v/vt/vn
                fscanf(file_handle, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &v1, &vt1, &vn_placeholder, &v2, &vt2, &vn_placeholder, &v3, &vt3, &vn_placeholder);
            } else if(has_v && has_vt && !has_vn) { // v/vt
                fscanf(file_handle, "%d/%d %d/%d %d/%d\n", &v1, &vt1, &v2, &vt2, &v3, &vt3);
            } else if(has_v && !has_vt && has_vn) { // v//vn
                fscanf(file_handle, "%d//%d %d//%d %d//%d\n", &v1, &vn_placeholder, &v2, &vn_placeholder, &v3, &vn_placeholder);
            } else if(has_v && !has_vt && !has_vn) { // v
                fscanf(file_handle, "%d %d %d\n", &v1, &v2, &v3);
            } else {
                LOG_ERROR("ERROR", "Cannot open .obj");
            }
            
            VertexAttribute temp_vertices[3];
            temp_vertices[0].position = positions[v1-1];
            temp_vertices[0].texcoord = texcoords[vt1-1];
            temp_vertices[1].position = positions[v2-1];
            temp_vertices[1].texcoord = texcoords[vt2-1];
            temp_vertices[2].position = positions[v3-1];
            temp_vertices[2].texcoord = texcoords[vt3-1];

            temp_vertices[0].normal = cross(temp_vertices[1].position - temp_vertices[0].position,
                                            temp_vertices[2].position - temp_vertices[0].position);
            temp_vertices[1].normal = temp_vertices[0].normal;
            temp_vertices[2].normal = temp_vertices[0].normal;
            for(int i = 0; i < 3; i++) {
                loaded_model.vertex_attributes.push_back(temp_vertices[i]);
            }
        }
    }
    
    return loaded_model;
}
