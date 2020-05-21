#include "win32_platform.cpp"

u64 GetLastWriteTime(cstring file) {
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExA(file, GetFileExInfoStandard, &attributes)){
        return (((u64) attributes.ftLastWriteTime.dwHighDateTime) << 32) | (attributes.ftLastWriteTime.dwLowDateTime);
    }
    return 0;
}

#if defined LIVE_CODE_RELOADING

#include "input.cpp"
#include "memory_arena.cpp"

bool LoadCode(code_info *code) {
    bool do_reload = false;
    
    u64 last_write_time = GetLastWriteTime((cstring)code->code_path);
    if(last_write_time > code->last_write_time) {
        
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        if (GetFileAttributesExA("build/compile.lock", GetFileExInfoStandard, &attributes))
            return false;
        
        do_reload = true;
        
        if (code->dll_handle)
            FreeLibrary(code->dll_handle);
        
        code->dll_handle = LoadLibraryA((cstring) code->code_path);
        assert(code->dll_handle);
        code->init   =   (init_function)GetProcAddress(code->dll_handle, "Init");
        code->update = (update_function)GetProcAddress(code->dll_handle, "Update");
        
        if (!code->init || !code->update)
            do_reload = false;
        
        FreeLibrary(code->dll_handle);
        
        if (do_reload) {
            bool ok = CopyFile((cstring) code->code_path, "build/live.dll", false);
            assert(ok);
        }
        
        code->dll_handle = LoadLibraryA("live.dll");
        assert(code->dll_handle);
        
        code->init   =   (init_function)GetProcAddress(code->dll_handle, "Init");
        code->update = (update_function)GetProcAddress(code->dll_handle, "Update");
        code->last_write_time = last_write_time;
    }
    
    return do_reload;
}

void InitCode(code_info *code, cstring application_path) {
    u8 *it = (u8 *)application_path;
    u32 code_path_count = 0;
    do {
        code->code_path[code_path_count++] = *it;
    } while(*(it++) != 0); 

    code->code_path[code_path_count - 4] = 'd';
    code->code_path[code_path_count - 3] = 'l';
    code->code_path[code_path_count - 2] = 'l';
    
    bool ok = LoadCode(code);
    assert(ok);
}

#else

#include "main.cpp"

#define LoadCode(...) 

void InitCode(code_info *code, cstring application_path) {
    code->init = Init;
    code->update = Update;
}

#endif

#if 1

s32 main(s32 argument_count, cstring arguments[]) {

#else

s32 WinMain(HINSTANCE instance, HINSTANCE prev_istance, LPSTR command_line, int show_command) {

    // TODO: FIX THIS!!!!!!!!!
    s32 argument_count;
    cstring *arguments = CommandLineToArgW(command_line, &argument_count);

#endif

    win32_api api;
    Win32Init(&api); 
    
    InitCode(&api.application, arguments[0]);
    
    api.application.init_data = api.application.init(&api);
    
    while(Win32HandleMessage(&api)) {
        api.code_was_reloaded = LoadCode(&api.application);
        
        api.application.update(&api, api.application.init_data);
    }   
    
	return 0;
}


