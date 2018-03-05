 
#include <math.h>

#define X_AXIS 1
#define Y_AXIS 2
#define Z_AXIS 3

#define PI 3.14159

struct ivec2 {
    int x;
    int y;

    ivec2() {
        this->x = 0;
        this->y = 0;
    }
    
    ivec2(int x, int y) {
        this->x = x;
        this->y = y;
    }

    bool operator == (ivec2 rhs) {
        return (this->x == rhs.x) && (this->y == rhs.y);
    }
};


struct vec2 {
    float x;
    float y;
    
    vec2() {
        this->x = 0.0f;
        this->y = 0.0f;
    }
    
    vec2(float x, float y) {
        this->x = x;
        this->y = y;
    }

    bool operator == (vec2 rhs) {
        return (this->x == rhs.x) && (this->y == rhs.y);
    }
};

struct vec3 {
    float x;
    float y;
    float z;

    vec3() {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
    }
    
    vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    vec3& operator += (const vec3 &rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
        return *this;
    }

    vec3& operator -= (const vec3 &rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;
        return *this;
    }

    bool operator == (vec3 rhs) {
        return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
    }
};

struct vec4 {
    float x;
    float y;
    float z;
    float w;
    
    vec4() {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
        this->w = 0.0f;
    }
    
    vec4(float x, float y, float z, float w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    
    bool operator == (vec4 rhs) {
        return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z) && (this->w == rhs.w);
    }
};

struct mat44 {
    float data[4][4];
    
    mat44() {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                this->data[i][j] = 0.0f;
            }
        }
    }
    
    mat44(float entry_11, float entry_12, float entry_13, float entry_14,
          float entry_21, float entry_22, float entry_23, float entry_24,
          float entry_31, float entry_32, float entry_33, float entry_34,
          float entry_41, float entry_42, float entry_43, float entry_44) {

        this->data[0][0] = entry_11;
        this->data[0][1] = entry_12;
        this->data[0][2] = entry_13;
        this->data[0][3] = entry_14;

        this->data[1][0] = entry_21;
        this->data[1][1] = entry_22;
        this->data[1][2] = entry_23;
        this->data[1][3] = entry_24;
        
        this->data[2][0] = entry_31;
        this->data[2][1] = entry_32;
        this->data[2][2] = entry_33;
        this->data[2][3] = entry_34;
        
        this->data[3][0] = entry_41;
        this->data[3][1] = entry_42;
        this->data[3][2] = entry_43;
        this->data[3][3] = entry_44;
    }
    
    mat44 operator = (mat44 rhs) {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                this->data[i][j] = rhs.data[i][j];
            }
        }
        return *this;
    }

    bool operator == (mat44 rhs) {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(this->data[i][j] != rhs.data[i][j])
                    return false;
            }
        }
        return true;
    }
};

inline ivec2
operator - (ivec2 lhs, ivec2 rhs) {
    ivec2 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

inline vec2
operator * (vec2 vector, float scaler) {
    vec2 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    return result;
}

inline vec2
operator * (float scaler, vec2 vector) {
    vec2 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    return result;
}

inline vec2
operator / (vec2 vector, float scaler) {
    vec2 result;
    result.x = vector.x / scaler;
    result.y = vector.y / scaler;
    return result;    
}

inline vec2
operator + (vec2 lhs, vec2 rhs) {
    vec2 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    return result;    
}

inline vec2
operator - (vec2 lhs, vec2 rhs) {
    vec2 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;    
}

inline vec3
operator * (vec3 vector, float scaler) {
    vec3 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    result.z = vector.z * scaler;
    return result;
}

inline vec3
operator * (float scaler, vec3 vector) {
    vec3 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    result.z = vector.z * scaler;
    return result;
}

inline vec3
operator / (vec3 vector, float scaler) {
    vec3 result;
    result.x = vector.x / scaler;
    result.y = vector.y / scaler;
    result.z = vector.z / scaler;
    return result;    
}

inline vec3
operator + (vec3 lhs, vec3 rhs) {
    vec3 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;    
}

inline vec3
operator - (vec3 lhs, vec3 rhs) {
    vec3 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;    
}

inline vec4
operator * (vec4 vector, float scaler) {
    vec4 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    result.z = vector.z * scaler;
    result.w = vector.w * scaler;
    return result;
}

inline vec4
operator * (float scaler, vec4 vector) {
    vec4 result;
    result.x = vector.x * scaler;
    result.y = vector.y * scaler;
    result.z = vector.z * scaler;
    result.w = vector.w * scaler;
    return result;
}

inline vec4
operator / (vec4 vector, float scaler) {
    vec4 result;
    result.x = vector.x / scaler;
    result.y = vector.y / scaler;
    result.z = vector.z / scaler;
    result.w = vector.w / scaler;
    return result;    
}

inline vec4
operator + (vec4 lhs, vec4 rhs) {
    vec4 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    result.w = lhs.w + rhs.w;
    return result;    
}

inline vec4
operator - (vec4 lhs, vec4 rhs) {
    vec4 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    result.w = lhs.w - rhs.w;
    return result;    
}

inline mat44
operator * (mat44 lhs, mat44 rhs) {
    mat44 result;
    float ElementValue;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            ElementValue = 0;
            for(int k = 0; k < 4; ++k) {
                ElementValue += (lhs.data[i][k] * rhs.data[k][j]);
            }
            result.data[i][j] = ElementValue;
        }
    }
    return result;
}

inline void
operator *= (mat44 &lhs, mat44 rhs) {
    lhs = lhs * rhs;
}

inline mat44
operator * (mat44 matrix, float scalar) {
    mat44 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.data[i][j] = matrix.data[i][j] * scalar;
        }
    }
    return result;
}

inline mat44
operator * (float scalar, mat44 matrix) {
    mat44 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.data[i][j] = matrix.data[i][j] * scalar;
        }
    }
    return result;
}

// NOTE(Alex): Assumes the matrix's fourth column is set to (0, 0, 0, 1) for accurate results.
inline vec4
operator * (vec4 vector, mat44 matrix) {
    vec4 result;
    result.x = vector.x * matrix.data[0][0] + vector.y * matrix.data[1][0] +
        vector.z * matrix.data[2][0] + vector.w * matrix.data[3][0];
    result.y = vector.x * matrix.data[0][1] + vector.y * matrix.data[1][1] +
        vector.z * matrix.data[2][1] + vector.w * matrix.data[3][1];
    result.z = vector.x * matrix.data[0][2] + vector.y * matrix.data[1][2] +
        vector.z * matrix.data[2][2] + vector.w * matrix.data[3][2];
    result.w = vector.x * matrix.data[0][3] + vector.y * matrix.data[1][3] +
        vector.z * matrix.data[2][3] + vector.w * matrix.data[3][3];
    return result;
}

inline mat44
operator + (mat44 lhs, mat44 rhs) {
    mat44 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.data[i][j] = lhs.data[i][j] + rhs.data[i][j];
        }
    }
    return result;
}

inline mat44
operator - (mat44 lhs, mat44 rhs) {
    mat44 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.data[i][j] = lhs.data[i][j] - rhs.data[i][j];
        }
    }
    return result;
}

inline float
dot(vec3 lhs, vec3 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

static vec3
cross(vec3 lhs, vec3 rhs) {
    vec3 result;
    result.x = (lhs.y * rhs.z) - (lhs.z * rhs.y);
    result.y = (lhs.z * rhs.x) - (lhs.x * rhs.z);
    result.z = (lhs.x * rhs.y) - (lhs.y * rhs.x);
    return result;
}

inline float
magnitude(vec3 source) {
    return sqrtf(source.x * source.x + source.y * source.y + source.z * source.z);
}

inline vec3
normalize(vec3 source) {
    vec3 result;
    float source_length = magnitude(source);
    float multiplier = 1.0f / source_length;
    
    result.x = source.x * multiplier;
    result.y = source.y * multiplier;
    result.z = source.z * multiplier;
    
    return result;
}

static void
scale(vec3 *vector, float x, float y, float z) {
    vector->x *= x;
    vector->y *= y;
    vector->z *= z;
}

static void
rotate(vec3 *vector, float angle, int axis_of_rotation) {
    switch(axis_of_rotation) {
        case X_AXIS: {
            vector->y = vector->y * (float)cos(angle) + vector->z * -1*(float)sin(angle);
            vector->z = vector->y * (float)sin(angle) + vector->z * (float)cos(angle);
        } break;
        
        case Y_AXIS: {
            vector->x = vector->x * (float)cos(angle) + vector->z * (float)sin(angle);
            vector->z = vector->x * -1*(float)sin(angle) + vector->z * (float)cos(angle);
        } break;
        
        case Z_AXIS: {
            vector->x = vector->x * (float)cos(angle) + vector->y * -1*(float)sin(angle);
            vector->y = vector->x * (float)sin(angle) + vector->y * (float)cos(angle);      
        } break;
    }
}

inline float
dot(vec4 lhs, vec4 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

static vec4
cross(vec4 lhs, vec4 rhs) {
    vec4 result;
    result.x = (lhs.y * rhs.z) - (lhs.z * rhs.y);
    result.y = (lhs.z * rhs.x) - (lhs.x * rhs.z);
    result.z = (lhs.x * rhs.y) - (lhs.y * rhs.x);
    return result;
}

inline float
magnitude(vec4 source) {
    return sqrtf(source.x * source.x + source.y * source.y + source.z * source.z + source.w * source.w);
}

inline vec4
normalize(vec4 source) {
    vec4 result;
    float source_length = magnitude(source);
    float multiplier = 1.0f / source_length;
    
    result.x = source.x * multiplier;
    result.y = source.y * multiplier;
    result.z = source.z * multiplier;
    result.w = source.w * multiplier;
    
    return result;
}

static void
scale(vec4 *vector, float x, float y, float z, float w) {
    vector->x *= x;
    vector->y *= y;
    vector->z *= z;
    vector->w *= w;
}

static void
rotate(vec4 *vector, float angle, int axis_of_rotation) {
    switch(axis_of_rotation) {
        case X_AXIS: {
            vector->y = vector->y * (float)cos(angle) + vector->z * -1*(float)sin(angle);
            vector->z = vector->y * (float)sin(angle) + vector->z * (float)cos(angle);
        } break;
        
        case Y_AXIS: {
            vector->x = vector->x * (float)cos(angle) + vector->z * (float)sin(angle);
            vector->z = vector->x * -1*(float)sin(angle) + vector->z * (float)cos(angle);
        } break;
        
        case Z_AXIS: {
            vector->x = vector->x * (float)cos(angle) + vector->y * -1*(float)sin(angle);
            vector->y = vector->x * (float)sin(angle) + vector->y * (float)cos(angle);      
        } break;
    }
}

static mat44
make_scaling_matrix(float x, float y, float z, float w) {
    return mat44(x, 0.0f, 0.0f, 0.0f,
                 0.0f, y, 0.0f, 0.0f,
                 0.0f, 0.0f, z, 0.0f,
                 0.0f, 0.0f, 0.0f, z);   
}

static mat44
transpose(mat44 matrix) {
    mat44 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.data[i][j] = matrix.data[j][i];
        }       
    }
    return result;
}

static mat44
view_transform(vec3 position, vec3 direction, vec3 up) {
    up = normalize(up);
    vec3 forward = normalize(direction);
    vec3 right = normalize(cross(up, forward)); 

    
    mat44 translation = mat44(1.0f, 0.0f, 0.0f, -position.x,
                              0.0f, 1.0f, 0.0f, -position.y,
                              0.0f, 0.0f, 1.0f, -position.z,
                              0.0f, 0.0f, 0.0f,  1.0f);

    mat44 orientation = mat44(right.x,   right.y,   right.z,   0.0f,
                              up.x,      up.y,      up.z,      0.0f,
                              forward.x, forward.y, forward.z, 0.0f,
                              0.0f,      0.0f,      0.0f,      1.0f);

    return orientation * translation;
}

static mat44
perspective(float FOV, float aspect, float z_near, float z_far) {
    mat44 result;
    float tan_half_FOV = (float)tan(FOV * 0.5 * (float)PI / 180.0f); 
    result.data[0][0] = 1.0f / (tan_half_FOV * aspect); 
    result.data[1][1] = 1.0f / tan_half_FOV; 
    result.data[2][2] = (z_far + z_near) / (z_far - z_near); 
    result.data[3][2] = -(2.0f * z_far * z_near) / (z_far - z_near);
    result.data[2][3] = 1.0f;
    return result;
}

inline vec2
lerp(vec2 a, vec2 b, float t) {
    return t*b + (1-t)*a;
}
