#ifndef BARESIP_MODULE_H
#define BARESIP_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#	define CALL_CONV
#else
#	define CALL_CONV //__stdcall // let's use default __cdecl
#endif

/** \def DECLARE_FN
    \brief Declare function or consistent function + function pointer set depending on _EXPORTING
*/
#ifdef _EXPORTING
#define DECLARE_FN(type, fn, ...) __declspec(dllexport) type CALL_CONV fn(__VA_ARGS__)
#else
#define DECLARE_FN(type, fn, ...) __declspec(dllimport) type CALL_CONV fn(__VA_ARGS__); \
	typedef type (CALL_CONV *pf_##fn)(__VA_ARGS__)
#endif

/** \brief Dll interface version, used to dismiss outdated libraries
*/
enum { DLL_INTERFACE_MAJOR_VERSION = 1 };
/** \brief Dll interface version, used to dismiss outdated libraries
*/
enum { DLL_INTERFACE_MINOR_VERSION = 0 };

/** \brief Holds Dll interface version
*/
struct S_DLL_INTERFACE
{
	int major_version;   ///< major part of interface version
	int minor_version;   ///< minor part of interface version
};

///////////////////////////////////////////////////////////////////////////////
// CALLBACKS
///////////////////////////////////////////////////////////////////////////////

typedef void* (CALL_CONV *mem_alloc_cb)(size_t size, mem_destroy_h *dh);
typedef void* (CALL_CONV *mem_deref_cb)(void *data);
typedef void  (CALL_CONV *aucodec_register_cb)(struct aucodec *ac);
typedef void  (CALL_CONV *aucodec_unregister_cb)(struct aucodec *ac);


///////////////////////////////////////////////////////////////////////////////
// EXPORTED/IMPORTED FUNCTION SET
///////////////////////////////////////////////////////////////////////////////

/** \brief Get dll interface version to check for compatibility
*/
DECLARE_FN(void, get_interface_description, struct S_DLL_INTERFACE*);

DECLARE_FN(int, validate_dll, void);



DECLARE_FN(void, set_mem_alloc_cb, mem_alloc_cb mem_alloc);
DECLARE_FN(void, set_mem_deref_cb, mem_deref_cb mem_deref);
DECLARE_FN(void, set_aucodec_register_cb, aucodec_register_cb aucodec_register);
DECLARE_FN(void, set_aucodec_unregister_cb, aucodec_unregister_cb aucodec_unregister);


#ifdef __cplusplus
}
#endif


#endif
