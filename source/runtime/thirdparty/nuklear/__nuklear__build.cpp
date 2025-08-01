#include "core/RpgPlatform.h"

#define NK_ASSERT						RPG_Assert
#define NK_UINT_DRAW_INDEX
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_KEYSTATE_BASED_INPUT

#define NK_IMPLEMENTATION

#pragma push_macro("free")
#undef free

#include "nuklear.h"

#pragma pop_macro("free")
