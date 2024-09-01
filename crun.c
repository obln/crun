#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

const char* get_temp_dir() {
    static char temp_dir[512];
    GetTempPathA(512, temp_dir);
    return temp_dir;
}

void make_dir(const char* path) {
    CreateDirectory(path, NULL);
}

int file_exists(const char* path) {
  uint32_t attribs = GetFileAttributes(path);
  return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}

#define execv _execv
#else
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

const char* get_temp_dir() {
    return getenv("TMPDIR");
}

void make_dir(const char* path) {
    mkdir(path, 0700);
}

int file_exists(const char* path) {
    return access(path, F_OK) == 0;
}
#endif

typedef enum {
    CompilerArgStyle_NONE = 0,
    CompilerArgStyle_GCC,  // Clang mirrors GCC
    CompilerArgStyle_MSVC
} CompilerArgStyle;

typedef struct {
    const char* command;
    CompilerArgStyle style;
} CompilerInfo;

#ifdef _WIN32
#define CHECK_COMMAND_EXISTS(cmd) !system("cmd /c \"(help "cmd" > nul || exit 0) && where "cmd" > nul 2> nul\"")

CompilerInfo detect_compiler() {
    // Visual Studio in PATH
    if (CHECK_COMMAND_EXISTS("cl")) return (CompilerInfo){ .command="cl", .style=CompilerArgStyle_MSVC };
    // MinGW in PATH
    else if (CHECK_COMMAND_EXISTS("gcc")) return (CompilerInfo){ .command="gcc", .style=CompilerArgStyle_GCC };
    // Clang in PATH
    else if (CHECK_COMMAND_EXISTS("clang")) return (CompilerInfo){ .command="clang", .style=CompilerArgStyle_GCC };

    return (CompilerInfo){ .command=NULL, .style=CompilerArgStyle_NONE };
}
#else
#define CHECK_COMMAND_EXISTS(cmd) !system("which " cmd " > /dev/null 2>&1")

CompilerInfo detect_compiler() {
    const char* env_cc = getenv("CC");
    // CC environment variable (assuming gcc-like)
    if (env_cc != NULL) return (CompilerInfo){ .command=env_cc, .style=CompilerArgStyle_GCC };
    // GCC in PATH
    else if (CHECK_COMMAND_EXISTS("gcc")) return (CompilerInfo){ .command="gcc", .style=CompilerArgStyle_GCC };
    // Clang in PATH
    else if (CHECK_COMMAND_EXISTS("clang")) return (CompilerInfo){ .command="gcc", .style=CompilerArgStyle_GCC };

    return (CompilerInfo){ .command=NULL, .style=CompilerArgStyle_NONE };
}
#endif

int compile_gcc_style(const char* cmd, const char* source_path, const char* binary_path) {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s %s -o %s", cmd, source_path, binary_path);
    return system(buffer);
}

int compile_msvc_style(const char* cmd, const char* source_path, const char* binary_path) {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s %s /Fe%s", cmd, source_path, binary_path);
    return system(buffer);
}

static int (*const compile_funcs[])(const char*, const char*, const char*) = {
    [CompilerArgStyle_GCC] = compile_gcc_style,
    [CompilerArgStyle_MSVC] = compile_msvc_style
};

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: No input file\n");
        return 1;
    }
    const char* to_compile = argv[1];
    if (!file_exists(to_compile)) {
        fprintf(stderr, "ERROR: File does not exist: %s\n", to_compile);
        return 1;
    }

    const char* temp = get_temp_dir();
    CompilerInfo compiler = detect_compiler();

    if (compiler.style == CompilerArgStyle_NONE) {
        fprintf(stderr, "ERROR: No compiler detected!\n");
        return 1;
    }

    // make_dir(".crun");

    char temp_crun[512];
    strcat(temp_crun, temp);
    strcat(temp_crun, "crun");

    char temp_exe[512];
    strcat(temp_exe, temp_crun);
    strcat(temp_exe, "/crunbin");

    make_dir(temp_crun);

    int code = compile_funcs[compiler.style](compiler.command, to_compile, temp_exe);
    if (code != 0) {
        fprintf(stderr, "ERROR: Compilation failed or compiled with warnings\n");
        return 1;
    }

    if (argc > 2) {
        char** arr = malloc(sizeof(char*) * (argc - 1));
        memcpy(arr, &argv[1], sizeof(char*) * (argc - 1));
        int code = execv(temp_exe, (char* const*)arr);
        free(arr);
        return code;
    } else {
        return system(temp_exe);
    }
}
