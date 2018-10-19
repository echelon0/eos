
void build_adjacency_list(Array<VertexAttribute> &vertices) {
    for(int current_vertex = 0; current_vertex < vertices.size; current_vertex++) {
        vertices[current_vertex].adjacent_vertex_indices = Array<u32>();
        for(int triangle_start_index = 0; triangle_start_index < vertices.size; triangle_start_index += 3) {
            for(int triangle_vertex = 0; triangle_vertex < 3; triangle_vertex++) {
                if(vertices[triangle_start_index + triangle_vertex] == vertices[current_vertex]) {
                    for(int i = 0; i < 3; i++) {
                        if((triangle_start_index + i) != current_vertex)
                            vertices[current_vertex].adjacent_vertex_indices.push_back(triangle_start_index + i);
                    }
                    break;
                }
            }
        }
    }
}

vec2 get_barycentric_coords_for_segment(vec3 a, vec3 b, vec3 point) {
    vec2 barycentric_coords;
    barycentric_coords.x = dot(point - a, normalize(b - a)) / magnitude(b - a);
    barycentric_coords.y = dot(b - point, normalize(b - a)) / magnitude(b - a);
    return barycentric_coords;
}

bool test_edge_region(vec3 a, vec3 b, vec3 c, vec3 d, vec3 point, vec2 barycentric_coords) {
    if((barycentric_coords.x < 1.0f) && (barycentric_coords.y < 1.0f)) {
        vec3 norm_abc = cross(c - a, b - a); //TODO: might not need to test if point it above faces
        vec3 norm_abd = cross(b - a, d - a);
        if((dot(point - a, norm_abc) >= 0.0f) && (dot(point - a, norm_abd) >= 0.0f)) { //A---B
            vec3 segment_center = 0.5f*a + 0.5f*b;
            vec3 n1 = normalize(segment_center - c);
            vec3 n2 = normalize(segment_center - d);
            if((dot(point - a, n1) >= 0) && (dot(point - a, n1) >= 0)) {
                return true;
            }    
        }
    }
    return false;
}

Array<vec3> closest_feature_on_simplex_to_point(Array<vec3> simplex, vec3 point) {
    Array<vec3> feature = Array<vec3>();
    switch(simplex.size) {
        case 1: { //point
            feature.push_back(simplex[0]);
        } break;

        case 2: { //line
            vec2 barycentric_coords = get_barycentric_coords_for_segment(simplex[0], simplex[1], point);

            if(barycentric_coords.x >= 1.0f) {
                feature.push_back(simplex[0]);
            } else if(barycentric_coords.y >= 1.0f) {
                feature.push_back(simplex[1]);
            } else {
                feature.push_back(simplex[0]);
                feature.push_back(simplex[1]);
            }
        } break;

        case 3: { //face
            vec2 barycentric_coords[3];
            barycentric_coords[0] = get_barycentric_coords_for_segment(simplex[0], simplex[1], point); //A---B
            barycentric_coords[1] = get_barycentric_coords_for_segment(simplex[1], simplex[2], point); //B---C
            barycentric_coords[2] = get_barycentric_coords_for_segment(simplex[2], simplex[0], point); //C---A

            if((barycentric_coords[0].x >= 1.0f) && (barycentric_coords[2].y >= 1.0f)) {
                feature.push_back(simplex[0]);
            } else if((barycentric_coords[0].y >= 1.0f) && (barycentric_coords[1].x >= 1.0f)) {
                feature.push_back(simplex[1]);
            } else if((barycentric_coords[1].y >= 1.0f) && (barycentric_coords[2].x >= 1.0f)) {
                feature.push_back(simplex[2]);
            } else {
                feature.push_back(simplex[0]);
                feature.push_back(simplex[1]);
                feature.push_back(simplex[2]);
            }
             
        } break;

        case 4: { //tetrahedron
            vec2 barycentric_coords[6];
            barycentric_coords[0] = get_barycentric_coords_for_segment(simplex[0], simplex[1], point); //A---B
            barycentric_coords[1] = get_barycentric_coords_for_segment(simplex[1], simplex[2], point); //B---C
            barycentric_coords[2] = get_barycentric_coords_for_segment(simplex[2], simplex[0], point); //C---A
            barycentric_coords[3] = get_barycentric_coords_for_segment(simplex[0], simplex[3], point); //A---D
            barycentric_coords[4] = get_barycentric_coords_for_segment(simplex[1], simplex[3], point); //B---D
            barycentric_coords[5] = get_barycentric_coords_for_segment(simplex[2], simplex[3], point); //C---D

            //test point regions
            if((barycentric_coords[0].x <= 0.0f) && (barycentric_coords[2].y <= 0.0f) && (barycentric_coords[3].x <= 0.0f)) {
                feature.push_back(simplex[0]);
            } else if((barycentric_coords[0].y <= 0.0f) && (barycentric_coords[1].x <= 0.0f) && (barycentric_coords[4].x <= 0.0f)) {
                feature.push_back(simplex[1]);
            } else if((barycentric_coords[1].y <= 0.0f) && (barycentric_coords[2].x <= 0.0f) && (barycentric_coords[5].x <= 0.0f)) {
                feature.push_back(simplex[2]);
            } else if((barycentric_coords[3].y <= 0.0f) && (barycentric_coords[4].y <= 0.0f) && (barycentric_coords[5].y <= 0.0f)) {
                feature.push_back(simplex[3]);
                
            } else { //test edge regions

                if(test_edge_region(simplex[0], simplex[1], simplex[2], simplex[3], point, barycentric_coords[0])) { //A---B
                    feature.push_back(simplex[0]);
                    feature.push_back(simplex[1]);
                } else if(test_edge_region(simplex[1], simplex[2], simplex[0], simplex[4], point, barycentric_coords[1])) { //B---C
                    feature.push_back(simplex[1]);
                    feature.push_back(simplex[2]);
                } else if(test_edge_region(simplex[2], simplex[0], simplex[1], simplex[3], point, barycentric_coords[2])) { //C---A
                    feature.push_back(simplex[2]);
                    feature.push_back(simplex[0]);
                } else if(test_edge_region(simplex[0], simplex[3], simplex[1], simplex[2], point, barycentric_coords[3])) { //A---D
                    feature.push_back(simplex[0]);
                    feature.push_back(simplex[3]);
                } else if(test_edge_region(simplex[1], simplex[3], simplex[2], simplex[0], point, barycentric_coords[4])) { //B---D
                    feature.push_back(simplex[1]);
                    feature.push_back(simplex[3]);
                } else if(test_edge_region(simplex[2], simplex[3], simplex[0], simplex[1], point, barycentric_coords[5])) { //C---D
                    feature.push_back(simplex[2]);
                    feature.push_back(simplex[3]);
                    
                } else { //test face regions

                    if((barycentric_coords[0].x < 1.0f) && (barycentric_coords[0].y < 1.0f) &&
                       (barycentric_coords[1].x < 1.0f) && (barycentric_coords[1].y < 1.0f) &&
                       (barycentric_coords[2].x < 1.0f) && (barycentric_coords[2].y < 1.0f) &&
                       (dot(point - simplex[0], cross(simplex[2] - simplex[0], simplex[1] - simplex[0])) >= 0.0f)) { // ABC
                        feature.push_back(simplex[0]);
                        feature.push_back(simplex[1]);
                        feature.push_back(simplex[2]);
                        
                    } else if((barycentric_coords[0].x < 1.0f) && (barycentric_coords[0].y < 1.0f) && // BAD
                              (barycentric_coords[3].x < 1.0f) && (barycentric_coords[3].y < 1.0f) &&
                              (barycentric_coords[4].x < 1.0f) && (barycentric_coords[4].y < 1.0f) &&
                              (dot(point - simplex[1], cross(simplex[3] - simplex[1], simplex[0] - simplex[1])) >= 0.0f)) {
                        feature.push_back(simplex[1]);
                        feature.push_back(simplex[0]);
                        feature.push_back(simplex[3]);
                        
                    } else if((barycentric_coords[2].x < 1.0f) && (barycentric_coords[2].y < 1.0f) && // ACD
                              (barycentric_coords[5].x < 1.0f) && (barycentric_coords[5].y < 1.0f) &&
                              (barycentric_coords[3].x < 1.0f) && (barycentric_coords[3].y < 1.0f) &&
                              (dot(point - simplex[0], cross(simplex[3] - simplex[0], simplex[2] - simplex[0])) >= 0.0f)) {
                        feature.push_back(simplex[0]);
                        feature.push_back(simplex[2]);
                        feature.push_back(simplex[3]);
                        
                    } else if((barycentric_coords[1].x < 1.0f) && (barycentric_coords[1].y < 1.0f) && // CBD
                              (barycentric_coords[4].x < 1.0f) && (barycentric_coords[4].y < 1.0f) &&
                              (barycentric_coords[5].x < 1.0f) && (barycentric_coords[5].y < 1.0f) &&
                              (dot(point - simplex[2], cross(simplex[3] - simplex[2], simplex[1] - simplex[2])) >= 0.0f)) {
                        feature.push_back(simplex[2]);
                        feature.push_back(simplex[1]);
                        feature.push_back(simplex[3]);
                        
                    } else { //internal region
                        feature.push_back(simplex[0]);
                        feature.push_back(simplex[1]);
                        feature.push_back(simplex[2]);
                        feature.push_back(simplex[3]);
                    }
                }
            }
            
        } break;

        default:
            break;
    }
    return feature;
}

vec3 compute_search_direction_from_feature(Array<vec3> feature, vec3 point) {
    vec3 search_direction = vec3();
    switch(feature.size) {
        case 1: { //point
            search_direction = normalize(point - feature[0]);
        } break;

        case 2: { //line
            vec3 a_to_b = (feature[1] - feature[0]) / magnitude(feature[1] - feature[0]);
            vec3 a_to_point = (point - feature[0]) / magnitude(point - feature[0]);
            vec3 plane_normal = normalize(cross(a_to_b, a_to_point));
            search_direction = normalize(cross(a_to_b, plane_normal));
            if(dot(a_to_point, search_direction) < 0.0f)
                search_direction = -search_direction;
            
        } break;

        case 3: { //face
            vec3 a_to_b = (feature[1] - feature[0]) / magnitude(feature[1] - feature[0]);
            vec3 a_to_c = (feature[2] - feature[0]) / magnitude(feature[2] - feature[0]);
            search_direction = normalize(cross(a_to_b, a_to_c));
            vec3 a_to_point = (point - feature[0]) / magnitude(point - feature[0]);
            if(dot(a_to_point, search_direction) < 0.0f)
                search_direction = -search_direction;
        } break;

        default:
            break;
    }
    return search_direction;
}
    
int neighbor_vertex_furthest_along_direction(Array<VertexAttribute> &vertices, u32 start_vertex, vec3 direction) {
    direction = normalize(direction);
    f32 largest_dot_product = dot(vertices[start_vertex].position, direction);
    int furthest_vertex_index = start_vertex;
    for(int adjacent_vertex = 0; adjacent_vertex < vertices[start_vertex].adjacent_vertex_indices.size; adjacent_vertex++) {
        vec3 adjacent_vertex_pos = vertices[vertices[start_vertex].adjacent_vertex_indices[adjacent_vertex]].position;
        if(dot(adjacent_vertex_pos, direction) > largest_dot_product) {
            largest_dot_product = dot(adjacent_vertex_pos, direction);
            furthest_vertex_index = vertices[start_vertex].adjacent_vertex_indices[adjacent_vertex];
        }
    }

    return furthest_vertex_index;
}

int support(Array<VertexAttribute> &vertices, u32 start_vertex, vec3 direction) {
    int support_point = neighbor_vertex_furthest_along_direction(vertices, start_vertex, direction);
    while(support_point != neighbor_vertex_furthest_along_direction(vertices, support_point, direction));
    return support_point;
}

bool GJK(Array<VertexAttribute> mesh_A, Array<VertexAttribute> mesh_B) {
    Array<vec3> simplex = Array<vec3>();
    int support_A = support(mesh_A, 0, vec3());
    int support_B = support(mesh_B, 0, vec3());
    vec3 support_point = mesh_A[support_B].position - mesh_B[support_A].position;
    simplex.push_back(support_point);
    vec3 d = vec3();
    for(;;) {
        simplex = closest_feature_on_simplex_to_point(simplex, vec3());
        d = compute_search_direction_from_feature(simplex, vec3());
        support_A = support(mesh_A, support_A, d);
        support_B = support(mesh_B, support_B, d);
        support_point = mesh_A[support_B].position - mesh_B[support_A].position;
        if(simplex.contains(support_point)) {
            return true;
        }
    }
}



