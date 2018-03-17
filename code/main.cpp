
/*
  TODO:
  - Object picking
  - Camera translation/orientation with mouse input
  - Realtime shader compilation
  - Line sweep ambient obscurance
     - config the compute shader
 */


#define LOG_ERROR(Message) MessageBoxA(0, Message, "ERROR", MB_OK|MB_ICONERROR)

#include <windows.h>
#include <Windowsx.h>
#include <D3Dcompiler.h>
#include <D3D11.h>
#include <stdio.h>
#include "math.h"
#include "array.h"

struct INPUT_STATE {
    bool W_KEY;
    bool A_KEY;                    
    bool S_KEY;
    bool D_KEY;
    bool UP_ARROW;
    bool DOWN_ARROW;
    bool LEFT_ARROW;
    bool RIGHT_ARROW;

    bool LEFT_MOUSE_BUTTON;
    bool LEFT_MOUSE_BUTTON_RELEASED;
    ivec2 DOWN_POS;
    ivec2 CURRENT_POS;
    ivec2 DRAG_VECTOR;
    bool CLICKED;
};

static bool global_is_running;
static INPUT_STATE global_input;

#include "renderer.cpp"
#include "editor.cpp"
#include "file_loader.cpp"

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
                    
                default:
                    break;
            }
        } break;

        case WM_KEYUP: {
            switch(wParam) {
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
                    
                default:
                    break;
            }
        } break;

        case WM_LBUTTONDOWN: {
            if(!global_input.LEFT_MOUSE_BUTTON) {
                global_input.DOWN_POS.x = GET_X_LPARAM(lParam); 
                global_input.DOWN_POS.y = GET_Y_LPARAM(lParam);
            }
            global_input.LEFT_MOUSE_BUTTON = true;
        } break;
            
        case WM_LBUTTONUP: {
            global_input.CURRENT_POS.x = GET_X_LPARAM(lParam); 
            global_input.CURRENT_POS.y = GET_Y_LPARAM(lParam);      
            global_input.LEFT_MOUSE_BUTTON_RELEASED = true;
        } break;
            
        default: {
            result = DefWindowProc(hWnd, Msg, wParam, lParam);
        } break;
    }
    
    return result;
}

void input_analysis() {
    if(global_input.LEFT_MOUSE_BUTTON) {
        global_input.DRAG_VECTOR = global_input.CURRENT_POS - global_input.DOWN_POS;
        
        if(global_input.LEFT_MOUSE_BUTTON_RELEASED && global_input.DRAG_VECTOR == ivec2(0, 0)) {
            //NOTE(Alex): CLICKED is defined as a left mouse button down followed by
            //            a release in which the cursor stayed in the same position.
            global_input.CLICKED = true; 
        }
    }
}

void reset_input() {
    if(global_input.LEFT_MOUSE_BUTTON && global_input.LEFT_MOUSE_BUTTON_RELEASED) {
        global_input.LEFT_MOUSE_BUTTON = false;
        global_input.LEFT_MOUSE_BUTTON_RELEASED = false;
    }
    global_input.CLICKED = false;
}

ivec2 CalcWindowPos(int window_width, int window_height) {
    ivec2 pos;
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    pos.y = (screen_height - window_height) / 2;
    pos.x = (screen_width - window_width) / 2;
    return pos;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ivec2 window_dim = ivec2(960, 580);
    ivec2 window_pos = CalcWindowPos(window_dim.x, window_dim.y);
    float frame_rate  = 60.0f;
    
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
            RECT client_rect;
            GetClientRect(window, &client_rect);
            ivec2 client_dim = ivec2(client_rect.right, client_rect.bottom);
            global_input = {};
            D3D_RESOURCE *directx = (D3D_RESOURCE *)malloc(sizeof(*directx));

            if(init_D3D(window, directx)) {
                Array<StaticModel> all_models;
                StaticModel test_model = load_obj("../assets/models/dragon1.OBJ");
                Entity test_entity = {0, test_model};
                Array<Entity> entities;
                entities.push_back(test_entity);
                all_models.push_back(test_model);
                set_vertex_buffer(directx, all_models);
                    
                global_is_running = true;
                while(global_is_running) {
                    MSG message;
                    while(PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&message);
                        DispatchMessage(&message);
                    }
                    input_analysis();
                    if(global_input.CLICKED) {
                        get_picked_entity_index(directx->camera.position, global_input.CURRENT_POS, client_dim, 45.0f, 16.0f/9.0f, entities);
                    }
                    set_constant_buffer(directx);
                    draw_frame(directx);
                    
                    reset_input();
                }
            } 
        }
    }
}

