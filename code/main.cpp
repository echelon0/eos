
/*
  TODO:
  - Camera translation/orientation with mouse input
  - Realtime shader compilation
  - Line sweep ambient obscurance
     - config the compute shader
 */


#define LOG_ERROR(Message) MessageBoxA(0, Message, "ERROR", MB_OK|MB_ICONERROR)

#include <windows.h>
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
};

static bool global_is_running;
static INPUT_STATE global_input;

#include "renderer.cpp"
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
            
        default: {
            result = DefWindowProc(hWnd, Msg, wParam, lParam);
        } break;
    }
    
    return result;
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
    int window_width  = 960;
    int window_height = 580;
    ivec2 window_pos = CalcWindowPos(window_width, window_height);
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
            window_width, window_height,
            0, 0,
            hInstance, 0);
        
        if(window) {
            global_input = {};
            D3D_RESOURCE *directx = (D3D_RESOURCE *)malloc(sizeof(*directx));

            if(init_D3D(window, directx)) {
                Array<StaticModel> all_models;
                StaticModel test_model = load_obj("../assets/models/dragon2.OBJ");
                all_models.push_back(test_model);
                set_vertex_buffer(directx, all_models);
                    
                global_is_running = true;
                while(global_is_running) {
                    set_constant_buffer(directx);
                    DrawFrame(directx);

                    MSG message;
                    while(PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&message);
                        DispatchMessage(&message);
                    }
                }
            } 
        }
    }
}

