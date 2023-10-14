#pragma once

#include "lobject.h"

typedef enum
{
    VM_SHUFFLE7(VM_SHUFFLE_COMMA,
        TM_INDEX, // a1
        TM_NEWINDEX,// a2
        TM_MODE, // a3
        TM_NAMECALL, // a4
        TM_CALL, // a5
        TM_ITER,// a6
        TM_LEN // a7
    ),

    TM_EQ, /* last tag method with `fast' access */

    VM_SHUFFLE7(VM_SHUFFLE_COMMA,
        TM_ADD,
        TM_SUB,
        TM_MUL,
        TM_DIV,
        TM_MOD,
        TM_POW,
        TM_UNM
    ),

    VM_SHUFFLE5(VM_SHUFFLE_COMMA,
        TM_LT,
        TM_LE,
        TM_CONCAT,
        TM_TYPE,
        TM_METATABLE
    ),

    TM_N /* number of elements in the enum */
} TMS;

#define gfasttm(g, et, e) ((et) == NULL ? NULL : ((et)->tmcache & (1u << (e))) ? NULL : luaT_gettm(et, e, (g)->tmname[e]))

#define fasttm(l, et, e) gfasttm(l->global, et, e)
#define fastnotm(et, e) ((et) == NULL || ((et)->tmcache & (1u << (e))))

LUAI_DATA const char* const luaT_typenames[];
LUAI_DATA const char* const luaT_eventname[];

LUAI_FUNC const TValue* luaT_gettm(Table* events, TMS event, TString* ename);
LUAI_FUNC const TValue* luaT_gettmbyobj(lua_State* L, const TValue* o, TMS event);

LUAI_FUNC const TString* luaT_objtypenamestr(lua_State* L, const TValue* o);
LUAI_FUNC const char* luaT_objtypename(lua_State* L, const TValue* o);

LUAI_FUNC void luaT_init(lua_State* L);