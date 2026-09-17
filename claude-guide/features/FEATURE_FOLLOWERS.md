# Feature: Following Pokémon

Branch: `feature/followers` (stacked on `feature/sandbox-menu`). Status: implemented, builds, awaiting manual test.

## Goal

HGSS style: the first Pokémon in the party walks behind the player. Toggleable in game from the Sandbox menu.

## Player-facing behaviour

- The lead party Pokémon follows you in the overworld, enters its Poké Ball for warps and scripted movement, and comes back out.
- Start → SANDBOX → "Following Pokémon" shows ON or OFF. Turning it off recalls the follower immediately; turning it on spawns it.

## Design decisions

- `OW_FOLLOWERS_ENABLED TRUE` in `include/config/overworld.h`. Everything else is the expansion's own follower system.
- `B_FLAG_FOLLOWERS_DISABLED` is the expansion's built-in "pause followers" flag. It is assigned `FLAG_SANDBOX_FOLLOWERS_OFF` (renamed from `FLAG_UNUSED_0x02A`). Because the flag means disabled, the menu row displays inverted: ON when the flag is clear. `Sandbox_SetAll` respects the inversion.
- Toggling calls `RemoveFollowingPokemon` / `UpdateFollowingPokemon` so the change is visible without taking a step.
- Default left at ON (flag clear) for new games.

## Files

- `include/config/overworld.h`: two lines.
- `include/constants/flags.h`: one rename.
- `src/sandbox.c`: `ROW_FOLLOWERS`, `IsRowOn`, side effect.

## Save impact

Followers use an object event slot and vanilla save fields; no struct change. Upstream notes the follower can occupy one of the 16 object event slots on busy maps.

## Known caveats from upstream

- Some vanilla cutscenes were not written with a follower in mind. The expansion ships `data/scripts/follower.inc` and hides the follower in most of them, but if the follower gets stuck or drawn oddly in a specific cutscene, that script needs `hidefollower` / `FLAG_SAFE_FOLLOWER_MOVEMENT` treatment. Report the map and moment.
- `OW_FOLLOWERS_ALLOWED_SPECIES`, `_MET_LVL`, `_MET_LOC` in `overworld.h` can restrict which Pokémon follow; all are 0 (no restriction).
- With `OW_GFX_COMPRESS TRUE` (default) very large follower sprites may show VRAM corruption on crowded maps; set it FALSE if seen.

## How to test

1. New game, get the starter, walk out of the lab: the starter follows.
2. Enter a house, go up stairs, fly, surf: follower recalls and returns.
3. Start → SANDBOX → Following Pokémon → OFF: follower disappears. ON: reappears.
4. Swap party order: the new lead follows.
