
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
