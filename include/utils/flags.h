#pragma once
#include "Luau/Common.h"

inline void setFlag(const char* name, bool value) {
    for (Luau::FValue<bool>* flag = Luau::FValue<bool>::list; flag; flag = flag->next)
        if (strcmp(flag->name, name) == 0)
            flag->value = value;
}
