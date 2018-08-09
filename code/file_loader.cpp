
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

StaticModel load_obj(char *file_name) {
    StaticModel loaded_model = {};
    
    char folder_path[] = "../assets/models/";
    int folder_path_length = string_length(folder_path);
    
    char *file_path = (char *)malloc((folder_path_length + string_length(file_name) + 1) * sizeof(char));
    string_copy(file_path, folder_path);
    string_cat(file_path, file_name);
        
    FILE *obj_file_handle = fopen(file_path, "r");
    if(!obj_file_handle) {
        char *msg = (char *)calloc(string_length("Cannot open .obj file: "), sizeof(char));
        string_cat(msg, "Cannot open .obj file: ");
        string_cat(msg, file_name);
        LOG_ERROR("ERROR", msg);
        return loaded_model;
    }

    //mtl file path
    int file_path_length = string_length(file_path) - 4;
    char mtl_file_extension[] = ".mtl";
    for(int i = file_path_length, j = 0; i < file_path_length + 4, j < 4; i++, j++) {
        file_path[i] = mtl_file_extension[j];
    }

    FILE *mtl_file_handle = fopen(file_path, "r");
    if(!mtl_file_handle) {
        char error_msg[] = "Material file not found for ";
        char *full_error_msg = (char *)malloc((string_length(error_msg) + string_length(file_name) + 1) * sizeof(char));
        string_copy(full_error_msg, error_msg);
        string_cat(full_error_msg, file_name);
        LOG_ERROR("Message", full_error_msg);
        delete full_error_msg;
        return loaded_model;
    }

    char line_id[2048];
    struct MaterialWithID {
        char name[1024];
        bool recorded;
        Material mat;
    };
    Array<MaterialWithID> materials;
    while(fscanf(mtl_file_handle, "%s", line_id) != EOF) {
        if(strcmp(line_id, "newmtl") == 0) {
            MaterialWithID mat = {};
            fscanf(mtl_file_handle, "%s", mat.name);
            materials.push_back(mat);
                        
        } else if(strcmp(line_id, "Ka") == 0) { //ambient
            fscanf(mtl_file_handle, "%f%f%f\n",
                   &materials[materials.size - 1].mat.ambient.x,
                   &materials[materials.size - 1].mat.ambient.y,
                   &materials[materials.size - 1].mat.ambient.z);
            
        } else if(strcmp(line_id, "Kd") == 0) { //diffuse
            fscanf(mtl_file_handle, "%f%f%f\n",
                   &materials[materials.size - 1].mat.diffuse.x,
                   &materials[materials.size - 1].mat.diffuse.y,
                   &materials[materials.size - 1].mat.diffuse.z);
            
        } else if(strcmp(line_id, "Ks") == 0) { //specular
            fscanf(mtl_file_handle, "%f%f%f\n",
                   &materials[materials.size - 1].mat.specular.x,
                   &materials[materials.size - 1].mat.specular.y,
                   &materials[materials.size - 1].mat.specular.z);

        } else if(strcmp(line_id, "Ns") == 0) { //specular exponent
            fscanf(mtl_file_handle, "%f", &materials[materials.size - 1].mat.exponent);

        } else if(strcmp(line_id, "d") == 0) { //dissolve
            fscanf(mtl_file_handle, "%f\n", &materials[materials.size - 1].mat.dissolve);
            
        } else if(strcmp(line_id, "Tr") == 0) { //1 - dissolve
            fscanf(mtl_file_handle, "%f\n", &materials[materials.size - 1].mat.dissolve);
            materials[materials.size - 1].mat.dissolve = 1.0f - materials[materials.size - 1].mat.dissolve;
            
        }  else if(strcmp(line_id, "illum") == 0) { //illumination model
            fscanf(mtl_file_handle, "%d\n", &materials[materials.size - 1].mat.illum_model);
        }
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

    int current_mat_index = -1;
    while(fscanf(obj_file_handle, "%s", line_id) != EOF) {
        if(strcmp(line_id, "v") == 0) { //vertex position
            fscanf(obj_file_handle, "%f%f%f\n", &position.x, &position.y, &position.z);
            positions.push_back(position);
            has_v = true;

        } else if(strcmp(line_id, "vt") == 0) { //texcoord
            fscanf(obj_file_handle, "%f%f\n", &texcoord.x, &texcoord.y);
            texcoords.push_back(texcoord);
            has_vt = true;
            
        } else if(strcmp(line_id, "vn") == 0) { //normal
            fscanf(obj_file_handle, "%f%f%f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
            has_vn = true;

        } else if(strcmp(line_id, "usemtl") == 0) {
            char mat_name[1024];
            fscanf(obj_file_handle, "%s", mat_name);
            for(int i = 0; i < materials.size; i++) {
                if(strcmp(materials[i].name, mat_name) == 0) {
                    current_mat_index = i;
                }
            }
            
        } else if(strcmp(line_id, "f") == 0) {
            int v1=0, v2=0, v3=0;
            int vt1=0, vt2=0, vt3=0;
            int vn_placeholder=0;

            if(has_v && has_vt && has_vn) { // v/vt/vn
                fscanf(obj_file_handle, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &v1, &vt1, &vn_placeholder, &v2, &vt2, &vn_placeholder, &v3, &vt3, &vn_placeholder);
            } else if(has_v && has_vt && !has_vn) { // v/vt
                fscanf(obj_file_handle, "%d/%d %d/%d %d/%d\n", &v1, &vt1, &v2, &vt2, &v3, &vt3);
            } else if(has_v && !has_vt && has_vn) { // v//vn
                fscanf(obj_file_handle, "%d//%d %d//%d %d//%d\n", &v1, &vn_placeholder, &v2, &vn_placeholder, &v3, &vn_placeholder);
            } else if(has_v && !has_vt && !has_vn) { // v
                fscanf(obj_file_handle, "%d %d %d\n", &v1, &v2, &v3);
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
                if(!materials[current_mat_index].recorded) {
                    Material final_material = { materials[current_mat_index].mat.ambient,
                                                materials[current_mat_index].mat.diffuse,
                                                materials[current_mat_index].mat.specular,
                                                materials[current_mat_index].mat.exponent,
                                                materials[current_mat_index].mat.dissolve,
                                                materials[current_mat_index].mat.illum_model };
                    
                    loaded_model.materials.push_back(final_material);
                    loaded_model.material_indices.push_back(loaded_model.vertex_attributes.size);
                    if(loaded_model.materials.size >= 2) {
                        int prev_mat_size = loaded_model.material_indices[loaded_model.material_indices.size - 1] - loaded_model.material_indices[loaded_model.material_indices.size - 2];
                        loaded_model.material_sizes.push_back(prev_mat_size);
                    }
                    materials[current_mat_index].recorded = true;
                }
                loaded_model.vertex_attributes.push_back(temp_vertices[i]);
            }
        }
    }
    
    if(loaded_model.materials.size >= 2) {
        int prev_mat_size = loaded_model.vertex_attributes.size - loaded_model.material_indices[loaded_model.material_indices.size - 2];
        loaded_model.material_sizes.push_back(prev_mat_size);
    } else if(loaded_model.materials.size == 1) {
        loaded_model.material_sizes.push_back(loaded_model.vertex_attributes.size);
    }

    string_copy(loaded_model.str_name, file_name);
    
    delete file_path;
    delete positions.data;
    delete normals.data;
    delete texcoords.data;

    fclose(obj_file_handle);
    fclose(mtl_file_handle);
    
    return loaded_model;
}
