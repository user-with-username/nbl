#include "analysis/visitor.h"

bool definesTick(Luau::AstStatBlock* root)
{
    for (Luau::AstStat* stat : root->body)
    {
        if (auto* fn = stat->as<Luau::AstStatFunction>())
        {
            if (auto* global = fn->name->as<Luau::AstExprGlobal>())
            {
                if (global->name == "tick")
                    return true;
            }
        }
    }
    return false;
}