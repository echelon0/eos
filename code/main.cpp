#include <atlstr.h>  
/*
  TODO:
  Make sure MSAA can be configured properly on any machine (quality/sample count)
  Send light information to GPU
  Textures
  Gamma correction
  Entity transform/rotation
*/

#define u32 unsigned int
#define f32 float
#define LOG_ERROR(Title, Message) MessageBoxA(0, Message, Title, MB_OK|MB_ICONERROR)

#include <windows.h>
#include <Windowsx.h>
#include <D3Dcompiler.h>
#include <D3D11.h>
#include <stdio.h>
#include "math.h"
#include "array.h"

struct INPUT_STATE {
    ivec2 client_center;
    bool ESC_KEY;
    
    bool W_KEY;
    bool A_KEY;                    
    bool S_KEY;
    bool D_KEY;
    bool UP_ARROW;
    bool DOWN_ARROW;
    bool LEFT_ARROW;
    bool RIGHT_ARROW;
    bool SPACE_BAR;
    bool SHIFT_KEY;

    ivec2 PREV_POS;
    ivec2 CURRENT_POS;
    bool SET_DRAG_VECTOR;
    ivec2 PER_FRAME_DRAG_VECTOR;
    vec2 PER_FRAME_DRAG_VECTOR_PERCENT;
    
    bool LEFT_MOUSE_BUTTON;
    bool LEFT_MOUSE_BUTTON_RELEASED;
    ivec2 LEFT_DOWN_POS;
    ivec2 LEFT_DRAG_VECTOR;
    bool LEFT_CLICKED;

    bool RIGHT_MOUSE_BUTTON;
    bool RIGHT_MOUSE_BUTTON_RELEASED;
    ivec2 RIGHT_DOWN_POS;
    ivec2 RIGHT_DRAG_VECTOR;
    bool RIGHT_CLICKED;
};

static u32 global_iTime = 0;
static bool global_is_running;
static INPUT_STATE global_input;

#include "renderer.cpp"
#include "editor.cpp"
#include "file_loader.cpp"
#include "game.cpp"

LRESULT CALLBACK CallWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch(Msg) {
        case WM_CLOSE: {
            global_is_running = false;
        } break;
            
        case WM_SIZE: {
            WORD window_width = LOWORD(lParam);
            WORD window_height = HIWORD(lParam);           
        } break;

        case WM_KEYDOWN: {
            switch(wParam) {
                case VK_ESCAPE: {
                    global_input.ESC_KEY = true;
                } break;
                case 'W': {
                    global_input.W_KEY = true;
                } break;
                case 'A': {
                    global_input.A_KEY = true;
                } break;
                case 'S': {
                    global_input.S_KEY = true;
                } break;
                case 'D': {
                    global_input.D_KEY = true;
                } break;
                case VK_UP: {
                    global_input.UP_ARROW = true;
                } break;
                case VK_DOWN: {
                    global_input.DOWN_ARROW = true;
                } break;
                case VK_LEFT: {
                    global_input.LEFT_ARROW = true;
                } break;
                case VK_RIGHT: {
                    global_input.RIGHT_ARROW = true;
                } break;
                case VK_SPACE: {
                    global_input.SPACE_BAR = true;
                } break;
                case VK_SHIFT: {
                    global_input.SHIFT_KEY = true;
                } break;
                    
                default:
                    break;
            }
        } break;

        case WM_KEYUP: {
            switch(wParam) {
                case VK_ESCAPE: {
                    global_input.ESC_KEY = false;
                } break;
                case 'W': {
                    global_input.W_KEY = false;
                } break;
                case 'A': {
                    global_input.A_KEY = false;
                } break;
                case 'S': {
                    global_input.S_KEY = false;
                } break;
                case 'D': {
                    global_input.D_KEY = false;
                } break;
                case VK_UP: {
                    global_input.UP_ARROW = false;
                } break;
                case VK_DOWN: {
                    global_input.DOWN_ARROW = false;
                } break;
                case VK_LEFT: {
                    global_input.LEFT_ARROW = false;
                } break;
                case VK_RIGHT: {
                    global_input.RIGHT_ARROW = false;
                } break;
                case VK_SPACE: {
                    global_input.SPACE_BAR = false;
                } break;
                case VK_SHIFT: {
                    global_input.SHIFT_KEY = false;
                } break;
                    
                default:
                    break;
            }
        } break;

        case WM_MOUSEMOVE: {
            global_input.PREV_POS = global_input.CURRENT_POS;
            global_input.CURRENT_POS.x = GET_X_LPARAM(lParam);
            global_input.CURRENT_POS.y = GET_Y_LPARAM(lParam);
            global_input.SET_DRAG_VECTOR = true;
        } break;
            
        case WM_LBUTTONDOWN: {
            if(!global_input.LEFT_MOUSE_BUTTON) {
                global_input.LEFT_DOWN_POS.x = GET_X_LPARAM(lParam);
                global_input.LEFT_DOWN_POS.y = GET_Y_LPARAM(lParam);
            }
            global_input.LEFT_MOUSE_BUTTON = true;
        } break;
            
        case WM_LBUTTONUP: {
            global_input.CURRENT_POS.x = GET_X_LPARAM(lParam);
            global_input.CURRENT_POS.y = GET_Y_LPARAM(lParam);
            global_input.LEFT_MOUSE_BUTTON_RELEASED = true;
        } break;

        case WM_RBUTTONDOWN: {
            if(!global_input.RIGHT_MOUSE_BUTTON) {
                global_input.RIGHT_DOWN_POS.x = GET_X_LPARAM(lParam);
                global_input.RIGHT_DOWN_POS.y = GET_Y_LPARAM(lParam);
            }
            global_input.RIGHT_MOUSE_BUTTON = true;
        } break;
            
        case WM_RBUTTONUP: {
            global_input.CURRENT_POS.x = GET_X_LPARAM(lParam);
            global_input.CURRENT_POS.y = GET_Y_LPARAM(lParam);
            global_input.RIGHT_MOUSE_BUTTON_RELEASED = true;
        } break;
     
        default: {
            result = DefWindowProc(hWnd, Msg, wParam, lParam);
        } break;
    }
    
    return result;
}

void process_input() {
    if(global_input.SET_DRAG_VECTOR) {
        global_input.PER_FRAME_DRAG_VECTOR = global_input.CURRENT_POS - global_input.client_center;
        global_input.PER_FRAME_DRAG_VECTOR_PERCENT = vec2((f32)global_input.PER_FRAME_DRAG_VECTOR.x / (f32)GetSystemMetrics(SM_CXSCREEN),
                                                          (f32)global_input.PER_FRAME_DRAG_VECTOR.y / (f32)GetSystemMetrics(SM_CYSCREEN));
    }
    if(global_input.LEFT_MOUSE_BUTTON) {
        global_input.LEFT_DRAG_VECTOR = global_input.CURRENT_POS - global_input.LEFT_DOWN_POS;
        
        if(global_input.LEFT_MOUSE_BUTTON_RELEASED && global_input.LEFT_DRAG_VECTOR == ivec2(0, 0)) {
            //NOTE(Alex): LEFT_CLICKED is defined as a left mouse button down followed by
            //            a release in which the cursor stayed in the same position.
            global_input.LEFT_CLICKED = true; 
        }
    }
    if(global_input.RIGHT_MOUSE_BUTTON) {
        global_input.LEFT_DRAG_VECTOR = global_input.CURRENT_POS - global_input.LEFT_DOWN_POS;
        
        if(global_input.RIGHT_MOUSE_BUTTON_RELEASED && global_input.RIGHT_DRAG_VECTOR == ivec2(0, 0)) {
            //NOTE(Alex): RIGHT_CLICKED is defined as a right mouse button down followed by
            //            a release in which the cursor stayed in the same position.
            global_input.RIGHT_CLICKED = true; 
        }
    }
}

void reset_input() {
    global_input.SET_DRAG_VECTOR = false;
    global_input.PER_FRAME_DRAG_VECTOR = ivec2(0, 0);
    global_input.PER_FRAME_DRAG_VECTOR_PERCENT = vec2(0.0f, 0.0f);
    
    if(global_input.LEFT_MOUSE_BUTTON && global_input.LEFT_MOUSE_BUTTON_RELEASED) {
        global_input.LEFT_MOUSE_BUTTON = false;
        global_input.LEFT_MOUSE_BUTTON_RELEASED = false;
    }
    global_input.LEFT_CLICKED = false;
    
    if(global_input.RIGHT_MOUSE_BUTTON && global_input.RIGHT_MOUSE_BUTTON_RELEASED) {
        global_input.RIGHT_MOUSE_BUTTON = false;
        global_input.RIGHT_MOUSE_BUTTON_RELEASED = false;
    }
    global_input.RIGHT_CLICKED = false;
}

void editor_camera_control(D3D_RESOURCE *directx) {
    if(global_input.W_KEY) {
        directx->camera.position += directx->camera.direction * 0.1f;
    }
    if(global_input.S_KEY) {
        directx->camera.position -= directx->camera.direction * 0.1f;
    }
    if(global_input.A_KEY) {
        directx->camera.position += cross(directx->camera.direction, directx->camera.up) * 0.1f;
    }
    if(global_input.D_KEY) {
        directx->camera.position -= cross(directx->camera.direction, directx->camera.up) * 0.1f;
    }
    if(global_input.SPACE_BAR && !global_input.SHIFT_KEY) {
        directx->camera.position += directx->camera.up * 0.1f;
    }
    if(global_input.SPACE_BAR && global_input.SHIFT_KEY) {
        directx->camera.position -= directx->camera.up * 0.1f;
    }
    
    if(global_input.RIGHT_ARROW) {
        rotate(&directx->camera.direction, 0.01f, Y_AXIS);
    }
    if(global_input.LEFT_ARROW) {
        rotate(&directx->camera.direction, -0.01f, Y_AXIS);
    }        
}

f32 get_elapsed_time(LARGE_INTEGER begin_count) {
    timeBeginPeriod(1);
    LARGE_INTEGER frequency;
    if(QueryPerformanceFrequency(&frequency)) {
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        timeEndPeriod(1);
        return ((f32)(count.QuadPart - begin_count.QuadPart) / (f32)frequency.QuadPart) * 1000.0f; // time in miliseconds
    } else {
        return (f32)GetTickCount();
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    ivec2 window_dim = ivec2(screen_width, screen_height);
    ivec2 window_pos;    
    window_pos.y = (screen_height - window_dim.y) / 2;
    window_pos.x = (screen_width - window_dim.x) / 2;
    f32 frame_rate  = 60.0f;
    
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = CallWindowProc;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "Eos";                        
    
    if(RegisterClass(&window_class)) {
        HWND window = CreateWindow(
            window_class.lpszClassName,
            "Eos",
            WS_VISIBLE|WS_OVERLAPPEDWINDOW,
            window_pos.x, window_pos.y,
            window_dim.x, window_dim.y,
            0, 0,
            hInstance, 0);
        
        if(window) {
            global_input = {};
            ShowCursor(0);
            
            RECT client_rect;
            GetClientRect(window, &client_rect);
            ivec2 client_dim = ivec2(client_rect.right, client_rect.bottom);
            global_input.client_center = client_dim / 2;
            POINT client_center_pt = {(LONG)global_input.client_center.x, (LONG)global_input.client_center.y};
            ClientToScreen(window, &client_center_pt);

            D3D_RESOURCE *directx = (D3D_RESOURCE *)malloc(sizeof(*directx));

            if(init_D3D(window, directx)) {
                global_is_running = true;
                GameState game_state = {};
                bool edit_mode = false;
                
                //entity creation
                StaticModel model;
                model = load_obj("sample_terrain.obj");
                if(model.vertex_attributes.size == 0) { global_is_running = false; }
                Entity test_entity = {};
                test_entity.model = model;
                game_state.entities.push_back(test_entity);

                //light creation
                editor::add_light(game_state.lights, game_state.num_lights, game_state.entities, POINT_LIGHT, 0);
                
                bool initialized = false;
                int picked_entity = -1;

                f32 FRAME_RATE = 60.0f;
                f32 FRAME_FREQUENCY = (1000.0f / FRAME_RATE);
                while(global_is_running) {
                    if(!initialized) {
                        set_vertex_buffer(directx, game_state.entities);
                        init_grid(&game_state.grid, game_state.entities);
                        initialized = true;
                    }
                    
                    LARGE_INTEGER begin_count;
                    QueryPerformanceCounter(&begin_count);
                    
                    MSG message;
                    while(PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&message);
                        DispatchMessage(&message);
                    }
                    if(global_input.ESC_KEY) {
                        global_is_running = false;
                        break;
                    }
                    process_input();

                    { //EDIT MODE
                        if(edit_mode) {
                            if(global_input.RIGHT_MOUSE_BUTTON) {
                                if(abs(global_input.PER_FRAME_DRAG_VECTOR_PERCENT.x) > abs(global_input.PER_FRAME_DRAG_VECTOR_PERCENT.y)) {
                                    directx->camera.rotate(-global_input.PER_FRAME_DRAG_VECTOR_PERCENT.x*2.0f, Y_AXIS);
                                } else if(abs(global_input.PER_FRAME_DRAG_VECTOR_PERCENT.x) < abs(global_input.PER_FRAME_DRAG_VECTOR_PERCENT.y)) {
                                    directx->camera.rotate(-global_input.PER_FRAME_DRAG_VECTOR_PERCENT.y*2.0f, X_AXIS);
                                }
                            }
                    
                            if(global_input.LEFT_CLICKED) {
                                picked_entity = editor::get_picked_entity_index(directx->camera, vec3(0.0f, 1.0f, 0.0f), global_input.CURRENT_POS, client_dim, 45.0f, 16.0f/9.0f, game_state.entities);
                                for(int i = 0; i < game_state.entities.size; i++) {
                                    game_state.entities[i].selected = false;
                                    if(picked_entity != -1)
                                        game_state.entities[picked_entity].selected = true;
                                }
                            }
                            editor_camera_control(directx);
                        }
                    }

                    game_update(&game_state, directx);
                    if(!draw_frame(directx, game_state.entities, game_state.lights, directx->camera.position)) break;
                    
                    reset_input();

                    f32 frame_duration = get_elapsed_time(begin_count);
                    if(frame_duration < FRAME_FREQUENCY) {
                        Sleep((DWORD)(FRAME_FREQUENCY - frame_duration));
                    }
                    global_iTime++;
                    
                    // set mouse cursor to center of screen
                    if(!edit_mode) {
                        SetCursorPos(client_center_pt.x, client_center_pt.y);
                    }
                    
                    // REMOVE:
                    frame_duration = get_elapsed_time(begin_count);
                    CString s;
                    s.Format(_T("%f"), frame_duration);
                    SetWindowText(window, s);
                    //////
                }
            } 
        }
    }
}

