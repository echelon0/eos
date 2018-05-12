
int string_length(char *string) {
    int count = 0;
    while(*string) {
        count++;
        *string++;
    }
    return count;
}

void string_copy(char *dest, char *source) {
    while(*source) {
        *dest++ = *source++;
    }
    *dest = '\0';
}

void string_cat(char *dest, char *source) {
    while(*dest) {
        *dest++;
    }
    while(*source) {
        *dest++ = *source++;
    }
    *dest = '\0';
}

template <typename T>
struct Array {
    T *data;
    int size;
    int capacity;

    Array() {
        size = 0;
        capacity = 1;
        data = (T *)calloc(capacity, sizeof(T));
    }
    
    T &operator[](int i) {
        return data[i];
    }
    
    void push_back(T entry) {
        if(size >= capacity) {
            data = (T *)realloc(data, sizeof(T) * capacity * 2);
            capacity *= 2;
        }
        data[size++] = entry;
    }
};
