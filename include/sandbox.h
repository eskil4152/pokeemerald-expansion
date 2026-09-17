#ifndef GUARD_SANDBOX_H
#define GUARD_SANDBOX_H

// Sandbox menu: a Start menu entry with toggles for god-mode style features.
// Each toggle is a flag (FLAG_SANDBOX_* in constants/flags.h). Most of them
// are read by existing config hooks in include/config/*.h; the rest are
// applied by Sandbox_OnPlayerStep.

void Sandbox_ShowMenu(void);
void Sandbox_OnPlayerStep(void);
void Sandbox_SetAll(bool32 enabled);

#endif // GUARD_SANDBOX_H
