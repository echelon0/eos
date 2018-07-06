
#define f32 float
#define u32 unsigned int
#define LOG_ERROR(Title, Message) MessageBoxA(0, Message, Title, MB_OK|MB_ICONERROR)

#include <atlstr.h>
/*
  TODO:
  Fix picking (use a texture rather than ray tracing) *IP* NOTE: possibly use the same renderer with two render targets?
  Entity transform/rotation
  Configure imgui *IP*
  Textures
*/

#include <windows.h>
#include <Windowsx.h>
#include <D3Dcompiler.h>
#include <D3D11.h>
#include <DXGI.h>
#include <DXGI1_2.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <stdio.h>
#include "math.h"
#include "array.h"

#include "..\code\imgui\imgui_draw.cpp"
#include "..\code\imgui\imgui_demo.cpp"
#include "..\code\imgui\imgui.cpp"
#include "imgui_config.cpp"

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
    bool CONTROL_KEY_TOGGLE;
    bool F1_KEY;

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

bool edit_mode = false;
static u32 global_iTime = 0;
static bool global_is_running;
static INPUT_STATE global_input = {};

#include "renderer.cpp"
#include "editor.cpp"
#include "file_loader.cpp"
#include "game.cpp"

LRESULT CALLBACK CallWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    if(GetActiveWindow() != hWnd) {
        result = DefWindowProc(hWnd, Msg, wParam, lParam);
    } else {
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
                    case VK_CONTROL: {
                        global_input.CONTROL_KEY_TOGGLE = !global_input.CONTROL_KEY_TOGGLE;
                    } break;
                    case VK_F1: {
                        global_input.F1_KEY = true;
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
                    case VK_F1: {
                        global_input.F1_KEY = false;
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
            global_input.LEFT_CLICKED = true; 
        }
    }
    if(global_input.RIGHT_MOUSE_BUTTON) {
        global_input.LEFT_DRAG_VECTOR = global_input.CURRENT_POS - global_input.LEFT_DOWN_POS;
        
        if(global_input.RIGHT_MOUSE_BUTTON_RELEASED && global_input.RIGHT_DRAG_VECTOR == ivec2(0, 0)) {
            global_input.RIGHT_CLICKED = true; 
        }
    }
    
    if(global_input.F1_KEY) {
        edit_mode = !edit_mode;
        global_input.F1_KEY = false;
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
            global_input.client_center = client_dim / 2;
            POINT client_center_pt = {(LONG)global_input.client_center.x, (LONG)global_input.client_center.y};
            ClientToScreen(window, &client_center_pt);

            f32 FRAME_RATE = 60.0f;
            f32 FRAME_FREQUENCY = (1000.0f / FRAME_RATE);
            
            D3D_RESOURCES *directx = (D3D_RESOURCES *)malloc(sizeof(*directx));
            if(init_D3D(window, directx)) {
                global_is_running = true;
                GameState game_state = {};
                
                //NOTE: TEST CODE for entity creation
                StaticModel model;
                model = load_obj("island.obj");
                if(model.vertex_attributes.size == 0) { global_is_running = false; }
                Entity test_entity = {};
                test_entity.model = model;
                test_entity.ID = 4;
                game_state.entities.push_back(test_entity);
                /////////////////////////                

                //imgui setup
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGui::StyleColorsDark();
                ImGuiIO& imguiIO = ImGui::GetIO();              
                ImGui_ImplDX11_Init(window, directx->device, directx->immediate_context);
                imguiIO.DisplaySize.x = (f32)client_dim.x;
                imguiIO.DisplaySize.y = (f32)client_dim.y;
                
                ImGuiWindowFlags general_imgui_window_flags = 0;
                //general_imgui_window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
                
                //editor variables
                int picked_entity = -1;
                int light_type = -1;
                                
                ShowCursor(0);
                bool initialized = false;
                while(global_is_running) {
                    if(!initialized) {
                        set_vertex_buffer(directx, game_state.entities);
                        init_grid(&game_state.grid, game_state.entities);
                        initialized = true;
                    }
                    
                    LARGE_INTEGER begin_count;
                    QueryPerformanceCounter(&begin_count);

                    ImGui_ImplDX11_NewFrame();
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

                    //imgui input
                    ImGuiIO& imguiIO = ImGui::GetIO();
                    imguiIO.DeltaTime = 1.0f / FRAME_RATE;
                    imguiIO.MouseDown[0] = global_input.LEFT_MOUSE_BUTTON;
                    imguiIO.MouseDown[1] = global_input.RIGHT_MOUSE_BUTTON;
                    imguiIO.MousePos = global_input.CURRENT_POS;                    

                    bool should_show_cursor = false;
                    bool should_center_cursor = (GetActiveWindow() == window);
                    bool game_paused = false;
                    if(edit_mode) {
                        ImGui::Begin("Debug", 0, general_imgui_window_flags);
                        //enable cursor use
                        if(global_input.CONTROL_KEY_TOGGLE) {
                            should_show_cursor = true;
                            should_center_cursor = false;
                            game_paused = true;
                            
                            //entity picking
                            if(global_input.LEFT_CLICKED) {
                                if(!read_ID3D11Texture2D(directx->picking_data, directx->picking_buffer,
                                                         directx->device, directx->immediate_context)) break;
                                int byte_offset = (global_input.CURRENT_POS.y * directx->picking_data.RowPitch) + (global_input.CURRENT_POS.x * sizeof(int));
                                picked_entity = *((int *)directx->picking_data.pData + (byte_offset / sizeof(int)));
                                for(int i = 0; i < game_state.entities.size; i++) {
                                    game_state.entities[i].selected = false;
                                    if(picked_entity != -1)
                                        game_state.entities[picked_entity].selected = true;
                                }
                            }
                        }
                        
                        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                        if(picked_entity != -1) {
                            ImGui::End();
                            ImGui::Begin(game_state.entities[picked_entity].model.str_name, 0, general_imgui_window_flags);
                            if(light_type == -1) {
                                ImGui::Text("Add light:");
                                ImGui::SameLine();
                                if(ImGui::Button("Directional"))
                                    light_type = DIRECTIONAL_LIGHT;
                                ImGui::SameLine();
                                if(ImGui::Button("Point"))
                                    light_type = POINT_LIGHT;
                                ImGui::SameLine();
                                if(ImGui::Button("Spotlight"))
                                    light_type = SPOTLIGHT;
                            } else {
                                if(light_type == DIRECTIONAL_LIGHT) {
                                    ImGui::Text("Add light: Directional");
                                } else if(light_type == POINT_LIGHT) {
                                    ImGui::Text("Add light: Point");
                                } else if(light_type == SPOTLIGHT){
                                    ImGui::Text("Add light: Spotlight");
                                }
                                f32 intensity;
                                f32 color[3];
                                f32 direction[3];
                                f32 cone_angle;
                                ImGui::SliderFloat("intensity", &intensity, 0.0f, 1.0f);
                                ImGui::ColorEdit3("color", color);
                                if(light_type != POINT_LIGHT) {
                                    ImGui::SliderFloat3("direction", direction, -1.0f, 1.0f);
                                }
                                if(light_type == SPOTLIGHT) {
                                    ImGui::SliderFloat("cone angle", &cone_angle, 0.0f, 1.0f);
                                }
                                if(ImGui::Button("Add light")) {
                                    editor::add_light(game_state.lights, game_state.num_lights, game_state.entities, picked_entity, DIRECTIONAL_LIGHT);
                                    light_type = -1;
                                }
                                ImGui::SameLine();
                                if(ImGui::Button("Cancel")) {
                                    light_type = -1;
                                }
                            }
                            ImGui::End();
                            ImGui::Begin("Debug", 0, general_imgui_window_flags);
                        }
                        ImGui::End();
                    }

                    if(!game_paused) {
                        game_update(&game_state, directx);
                    }
                    
                    if(!draw_frame(directx, game_state.entities, game_state.lights, game_state.camera)) break;
                    
                    ImGui::Render();
                    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                    DXGI_PRESENT_PARAMETERS present_parameters = { 0, NULL, NULL, NULL}; // TODO: fix fullscreen
                    HRESULT present_hr = directx->swap_chain->Present1(0, 0, &present_parameters);
                    if(FAILED(present_hr)) {
                        LOG_ERROR("ERROR", "Unable to swap buffers");
                        break;
                    }
                    
                    reset_input();

                    f32 frame_duration = get_elapsed_time(begin_count);
                    if(frame_duration < FRAME_FREQUENCY) {
                        Sleep((DWORD)(FRAME_FREQUENCY - frame_duration));
                    }
                    global_iTime++;

                    //configure mouse cursor
                    ShowCursor(should_show_cursor);
                    if(should_center_cursor) {
                        SetCursorPos(client_center_pt.x, client_center_pt.y);
                    }
                }
                ImGui::DestroyContext();
            }
            clean_D3D(directx);
        }
    }
}

