#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>

#include "win32_platform.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "opengl32.lib")

typedef BOOL (*wglChoosePixelFormatARB_function) (HDC hdc,
                                 const int *piAttribIList,
                                 const FLOAT *pfAttribFList,
                                 UINT nMaxFormats,
                                 int *piFormats,
                                 UINT *nNumFormats);

typedef HGLRC (*wglCreateContextAttribsARB_function) (HDC hDC, HGLRC hShareContext,
                                     const int *attribList);

wglChoosePixelFormatARB_function wglChoosePixelFormatARB;
wglCreateContextAttribsARB_function wglCreateContextAttribsARB;

#define WglLoad(name) name = (name ##_function) wglGetProcAddress(#name)

void Win32RequestOpenGLCoreProfile(win32_api *api, win32_window *window, cstring title){
    WglLoad(wglChoosePixelFormatARB);
    WglLoad(wglCreateContextAttribsARB);

    if (wglChoosePixelFormatARB && wglCreateContextAttribsARB) {
        wglMakeCurrent(null, null);
        wglDeleteContext(api->gl_context);
        DestroyWindow(window->handle);

        window->handle = CreateWindowA(
            api->window_class.lpszClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            window->width,
            window->height,
            null,
            null,
            api->window_class.hInstance,
            0
            );
        assert(window->handle);

        window->device_context = GetDC(window->handle);
        assert(window->device_context);

        s32 pixel_format_attributes[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0,
            // multi sample anti aliasing
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, //Number of buffers (must be 1 at time of writing)
            WGL_SAMPLES_ARB, 1,        //Number of samples
            0 // end
        };

        s32 pixel_format;
        u32 pixel_format_count;
        wglChoosePixelFormatARB(window->device_context, pixel_format_attributes, null, 1, &pixel_format, &pixel_format_count);
        SetPixelFormat(window->device_context, pixel_format, null);

        s32 context_attributes[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
 #if 1
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
 #else
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#endif
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };

        api->gl_context = wglCreateContextAttribsARB(window->device_context, null, context_attributes);
        assert(api->gl_context);

        wglMakeCurrent(window->device_context, api->gl_context);
    }    
}

LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
    auto window = (win32_window *) GetWindowLongPtrA(window_handle, GWLP_USERDATA); 
    if (!window) {
        return DefWindowProc(window_handle, message, w_param, l_param);
    } 
    
    switch(message) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;
        
        case WM_SIZE: {
            RECT rectangle;
            GetClientRect(window_handle, &rectangle); 
            
            window->width = rectangle.right - rectangle.left;
            window->height = rectangle.bottom - rectangle.top;
        } break;
    }
    
    return DefWindowProc(window_handle, message, w_param, l_param);
}

CREATE_WINDOW_SIGNATURE(Win32CreateWindow) {
    *window = {};
    
    window->handle = CreateWindowA(
        api->window_class.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        null,
        null,
        api->window_class.hInstance,
        0
        );
    assert(window->handle);
    
    window->width = width;
    window->height = height;
    
    window->device_context = GetDC(window->handle);
    assert(window->device_context);

    PIXELFORMATDESCRIPTOR pixel_format_descriptor = {};
    pixel_format_descriptor.nSize = sizeof(pixel_format_descriptor);
    pixel_format_descriptor.nVersion = 1;
    pixel_format_descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixel_format_descriptor.iPixelType = PFD_TYPE_RGBA;
    pixel_format_descriptor.cColorBits = 32;
    pixel_format_descriptor.cDepthBits = 24;
    pixel_format_descriptor.cStencilBits = 8;
    pixel_format_descriptor.iLayerType = PFD_MAIN_PLANE;
    
    //TODO: check for error
    s32 pixel_format = ChoosePixelFormat(window->device_context, &pixel_format_descriptor);
    SetPixelFormat(window->device_context, pixel_format, &pixel_format_descriptor);
    api->gl_context = wglCreateContext(window->device_context);
    assert(api->gl_context);
    
    wglMakeCurrent(window->device_context, api->gl_context);
    
    Win32RequestOpenGLCoreProfile(api, window, title);

    SetWindowLongPtrA(window->handle, GWLP_USERDATA, (LONG_PTR)window);
    ShowWindow(window->handle, SW_SHOW);

    RECT rectangle;
    GetClientRect(window->handle, &rectangle); 
    
    s32 real_width = 2 * width - (rectangle.right - rectangle.left);
    s32 real_height = 2 * height - (rectangle.bottom - rectangle.top);

    SetWindowPos(window->handle, null, 0, 0, real_width, real_height, SWP_NOMOVE | SWP_NOACTIVATE);
}

DISPLAY_WINDOW_SIGNATURE(Win32DisplayWindow) {
    SwapBuffers(window->device_context);
}

bool Win32HandleMessage(win32_api *api){
    MSG msg;
    
    for (u32 i = 0; i < ArrayCountOf(api->input.keys); i++) {
        api->input.keys[i].half_transition_count = 0;
        api->input.keys[i].half_transition_count_overflow = false;
    }

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {            
        switch (msg.message) {
            case WM_QUIT: {
                return false;
            } break;

            case WM_KEYDOWN: {
                ButtonUpdate(&api->input.keys[msg.wParam], true);
            } break;
            
            case WM_KEYUP: {
                ButtonUpdate(&api->input.keys[msg.wParam], false);
            } break;            

            default:{
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    api->delta_seconds = (time.QuadPart - api->time_in_ticks) / (f32) api->ticks_per_second;
    api->time_in_ticks = time.QuadPart;

    return true;    
}

READ_ENTIRE_FILE_SIGNATURE(Win32ReadEntireFile) {
    HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    
    assert(file_handle);
    
    WIN32_FILE_ATTRIBUTE_DATA attribute_data;
    if (!GetFileAttributesEx(file_path, GetFileExInfoStandard, &attribute_data))
        return {};
    
    assert(attribute_data.nFileSizeHigh == 0);
    u64 byte_count = (((u64)attribute_data.nFileSizeHigh) << 32) | attribute_data.nFileSizeLow;
    
    u8_array data = allocate(arena, byte_count, 1);
    
    DWORD bytes_read_count;
    if (!ReadFile(file_handle, data.base, data.count, &bytes_read_count, null)) {
        CloseHandle(file_handle);
        return {};
    }

    CloseHandle(file_handle);    
    return { data, true };
}

MAKE_MEMORY_ARENA(Win32MakeMemoryArena)
{
    memory_arena arena;
    arena.buffer.base  = (u8 *)VirtualAlloc(null, byte_count, MEM_COMMIT, PAGE_READWRITE);
    arena.buffer.count = byte_count;
    arena.used_count = 0;
    
    return arena;
}

void Win32Init(win32_api *api) {
    *api = {};
    // platform interface
    api->create_window     = Win32CreateWindow;
    api->display_window    = Win32DisplayWindow;
    api->read_entire_file  = Win32ReadEntireFile;
    api->make_memory_arena = Win32MakeMemoryArena;
    
    api->window_class.hInstance = GetModuleHandle(null);
    api->window_class.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
    api->window_class.lpszClassName = "InstaName";
    api->window_class.style = CS_OWNDC;
    api->window_class.hCursor = LoadCursor(null, IDC_ARROW);
    api->window_class.lpfnWndProc = WindowProc;
    
    if (!RegisterClassA(&api->window_class)) {
        printf("RegisterClassA failed with error %d.\n", GetLastError());
    }        

    LARGE_INTEGER ticks_per_second;
    QueryPerformanceFrequency(&ticks_per_second);
    api->ticks_per_second = ticks_per_second.QuadPart;
}
