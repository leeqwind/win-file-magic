#include <stdint.h>
#include <stdio.h>
#include <Windows.h>

#define LIBMAGIC "libmagic.dll"
#define DEFMAGIC "magic.mgc"

#define magic_t PVOID

typedef magic_t (__cdecl *fmagic_open)(INT);
typedef VOID    (__cdecl *fmagic_close)(magic_t);
typedef PCHAR   (__cdecl *fmagic_error)(magic_t);
typedef INT     (__cdecl *fmagic_errno)(magic_t);
typedef PCHAR   (__cdecl *fmagic_file)(magic_t, PCHAR);
typedef PCHAR   (__cdecl *fmagic_buffer)(magic_t, PVOID, SIZE_T);
typedef INT     (__cdecl *fmagic_load)(magic_t, PCHAR);
typedef INT     (__cdecl *fmagic_setflags)(magic_t, INT);
typedef INT     (__cdecl *fmagic_check)(magic_t, PCHAR);
typedef INT     (__cdecl *fmagic_compile)(magic_t, PCHAR);

static fmagic_open     magic_open     = NULL;
static fmagic_close    magic_close    = NULL;
static fmagic_error    magic_error    = NULL;
static fmagic_errno    magic_errno    = NULL;
static fmagic_file     magic_file     = NULL;
static fmagic_buffer   magic_buffer   = NULL;
static fmagic_load     magic_load     = NULL;
static fmagic_setflags magic_setflags = NULL;
static fmagic_check    magic_check    = NULL;
static fmagic_compile  magic_compile  = NULL;

CONST INT MAGIC_NONE            = 0x000000; // No flags
CONST INT MAGIC_DEBUG           = 0x000001; // Turn on debugging
CONST INT MAGIC_SYMLINK         = 0x000002; // Follow symlinks
CONST INT MAGIC_COMPRESS        = 0x000004; // Check inside compressed files
CONST INT MAGIC_DEVICES         = 0x000008; // Look at the contents of devices
CONST INT MAGIC_MIME            = 0x000010; // Return a mime string
CONST INT MAGIC_MIME_ENCODING   = 0x000400; // Return the MIME encoding
CONST INT MAGIC_CONTINUE        = 0x000020; // Return all matches
CONST INT MAGIC_CHECK           = 0x000040; // Print warnings to stderr
CONST INT MAGIC_PRESERVE_ATIME  = 0x000080; // Restore access time on exit
CONST INT MAGIC_RAW             = 0x000100; // Don't translate unprintable chars
CONST INT MAGIC_ERROR           = 0x000200; // Handle ENOENT etc as real errors

CONST INT MAGIC_NO_CHECK_COMPRESS = 0x001000; // Don't check for compressed files
CONST INT MAGIC_NO_CHECK_TAR    = 0x002000;  // Don't check for tar files
CONST INT MAGIC_NO_CHECK_SOFT   = 0x004000;  // Don't check magic entries
CONST INT MAGIC_NO_CHECK_APPTYPE = 0x008000; // Don't check application type
CONST INT MAGIC_NO_CHECK_ELF    = 0x010000;  // Don't check for elf details
CONST INT MAGIC_NO_CHECK_ASCII  = 0x020000;  // Don't check for ascii files
CONST INT MAGIC_NO_CHECK_TROFF  = 0x040000;  // Don't check ascii/troff
CONST INT MAGIC_NO_CHECK_FORTRAN = 0x080000; // Don't check ascii/fortran
CONST INT MAGIC_NO_CHECK_TOKENS = 0x100000;  // Don't check ascii/tokens

BOOL FindFunc(HMODULE hModule)
{
    magic_open     = (fmagic_open)    GetProcAddress(hModule, "magic_open");
    magic_close    = (fmagic_close)   GetProcAddress(hModule, "magic_close");
    magic_error    = (fmagic_error)   GetProcAddress(hModule, "magic_error");
    magic_errno    = (fmagic_errno)   GetProcAddress(hModule, "magic_errno");
    magic_file     = (fmagic_file)    GetProcAddress(hModule, "magic_file");
    magic_buffer   = (fmagic_buffer)  GetProcAddress(hModule, "magic_buffer");
    magic_load     = (fmagic_load)    GetProcAddress(hModule, "magic_load");
    magic_setflags = (fmagic_setflags)GetProcAddress(hModule, "magic_setflags");
    magic_check    = (fmagic_check)   GetProcAddress(hModule, "magic_check");
    magic_compile  = (fmagic_compile) GetProcAddress(hModule, "magic_compile");
    return (magic_open     != NULL &&
            magic_close    != NULL &&
            magic_error    != NULL &&
            magic_errno    != NULL &&
            magic_file     != NULL &&
            magic_buffer   != NULL &&
            magic_load     != NULL &&
            magic_setflags != NULL &&
            magic_check    != NULL &&
            magic_compile  != NULL);
}

INT main(INT argc, CHAR *argv[])
{
    INT     iReturn = 0;
    HMODULE hModule = NULL;
    PCHAR   BinTemp = NULL;
    PCHAR   BinPath = NULL;
    CHAR    LibPath[MAX_PATH] = { 0 };
    INT     flags   = MAGIC_NONE;
    magic_t cookie  = NULL;

    do
    {
        if (argc < 2)
        {
            break;
        }
        BinPath = argv[1];
        for (INT i = 2; i < argc; i++)
        {
            if (_stricmp(argv[i], "--mime-type") == 0)
            {
                flags |= MAGIC_MIME;
                continue;
            }
            if (_stricmp(argv[i], "--mime-encoding") == 0)
            {
                flags |= MAGIC_MIME_ENCODING;
                continue;
            }
            if (_stricmp(argv[i], "--keep-going") == 0)
            {
                flags |= MAGIC_CONTINUE;
                continue;
            }
            if (_stricmp(argv[i], "--uncompress") == 0)
            {
                flags |= MAGIC_COMPRESS;
                continue;
            }
        }
        hModule = LoadLibraryA(LIBMAGIC);
        if (hModule == NULL)
        {
            break;
        }
        if (!FindFunc(hModule))
        {
            break;
        }
        GetModuleFileNameA(NULL, LibPath, MAX_PATH);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }
        PCHAR p = strrchr(LibPath, '\\');
        if (p == NULL)
        {
            break;
        }
        p[1] = 0x00; // 获取当前目录.
        strcat_s(LibPath, sizeof(LibPath), DEFMAGIC);
        cookie = magic_open(flags);
        magic_load(cookie, LibPath);
        BinTemp = magic_file(cookie, BinPath);

    } while (FALSE);

    printf("%s", BinTemp != NULL ? BinTemp : "none\r\n");

    if (cookie != NULL)
    {
        magic_close(cookie);
        cookie = NULL;
    }
    return iReturn;
}
