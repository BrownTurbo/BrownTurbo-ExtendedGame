# BrownTurbo-ExtendedGame

**BrownTurbo-ExtendedGame** is designed as an extensible game expansion framework for SA-MP, bringing singleplayer gameplay mechanics, advanced character customization, and extended features into multiplayer.

## Features

1. **Full Singleplayer CJ Customization in SA-MP**:
   - Customize all 18 texture slots and 10 3D model component slots.
   - Live Fat & Muscle body transformation (0.0 to 1000.0) with real-time composite mesh morphing.
   - Support for all clothing items, shoes, haircuts, tattoos, accessories, and special suits directly from `player.img`.

2. **Fixes the Infamous SA-MP Model 0 Crash**:
   - In vanilla SA-MP, setting remote players to Skin 0 crashes the game because Model 0 lacks a pre-built static DFF in `gta3.img`.
   - BrownTurbo-ExtendedGame hooks `CPed::SetModelIndex` (0x5E4880) to dynamically initialize `CPlayerData` and rebuild the composite clump with `CClothesBuilder`, preventing any crashes.

3. **Remote & Local Player Synchronization**:
   - Can target the local player (`playerId = 0xFFFF`) or any remote player (`playerId = 0..1000`).
   - Caches custom clothing state so that when players stream in or respawn, their custom outfit is preserved automatically.

4. **Multi-Version SA-MP Compatibility**:
   - Automatically adapts to SA-MP client versions: `0.3.7-R1`, `0.3.7-R3-1`, `0.3.7-R5-1`, and `0.3.DL-1`.
   - Safely preserves and restores SA-MP attached accessories (`SetPlayerAttachedObject`) across model rebuilds.

5. **MTA-style Water Controls**:
   - Set world sea and lake heights independently, optionally change outside-world sea height, and reset world water to its original levels.
   - Set water wave intensity from `0%` to `100%` of GTA:SA's normal waviness; `-1.0` restores native weather-driven waves.
   - For setters that support `playerid = -1`, the change applies to all current players and is retained as the default for future players.

---

## RakNet Packet ID 252 Specification

Communication uses RakNet Packet ID `252` (`0xFC`).

### Packet Header
| Offset | Type | Description |
|---|---|---|
| `0` | `uint8_t` | Packet ID = `252` |
| `1-2` | `uint16_t` | Target Player ID (`0xFFFF` = Local Player, `0..1000` = Remote Player ID) |
| `3` | `uint8_t` | Action ID (`eCJSkinAction`) |

### Action Sub-commands

#### 1. `CJ_ACTION_SET_CLOTHES` (0x01)
Sets a clothes or tattoo item in a specific slot.
| Field | Type | Description |
|---|---|---|
| `slot` | `uint8_t` | Slot index (`0` to `17`) |
| `texture` | String | Texture name in `player.img` (e.g. `"vest"`, `"hoodyam"`) |
| `model` | String | Model name in `player.img` (e.g. `"vest"`, `"hoody"`, or `""` for tattoos) |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 2. `CJ_ACTION_REMOVE_CLOTHES` (0x02)
Removes clothing/tattoo from a slot.
| Field | Type | Description |
|---|---|---|
| `slot` | `uint8_t` | Slot index (`0` to `17`) |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 3. `CJ_ACTION_SET_BODY_STATS` (0x03)
Morphs CJ's body between skinny, fat, and muscular.
| Field | Type | Description |
|---|---|---|
| `fat` | `float` | Fat stat (`0.0` to `1000.0`) |
| `muscle` | `float` | Muscle stat (`0.0` to `1000.0`) |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 4. `CJ_ACTION_RESET_CLOTHES` (0x04)
Resets clothing.
| Field | Type | Description |
|---|---|---|
| `toDefault` | `uint8_t` | `1` = Singleplayer default (tank top, jeans, sneakers, caesar), `0` = Underpants |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 5. `CJ_ACTION_REBUILD` (0x05)
Forces an immediate rebuild of the ped's composite mesh. (No extra payload).

#### 6. `CJ_ACTION_SET_FULL_OUTFIT` (0x06)
Sets an entire outfit and body stats in a single network packet.
| Field | Type | Description |
|---|---|---|
| `fat` | `float` | Fat stat (`0.0` to `1000.0`) |
| `muscle` | `float` | Muscle stat (`0.0` to `1000.0`) |
| `itemCount` | `uint8_t` | Number of clothes items |
| *Items Array* | | `slot` (`uint8_t`), `texture` (String), `model` (String) |

#### 7. `CJ_ACTION_SET_CLOTHES_HASH` (0x07)
Sets clothes using CRC32 hash keys directly (bandwidth-optimized).
| Field | Type | Description |
|---|---|---|
| `slot` | `uint8_t` | Slot index (`0` to `17`) |
| `textureHash` | `uint32_t` | CRC32 hash of texture name |
| `modelHash` | `uint32_t` | CRC32 hash of model name |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 8. `CJ_ACTION_SET_TATTOO` (0x08)
Sets a tattoo in one of the 9 tattoo slots.
| Field | Type | Description |
|---|---|---|
| `tattooSlot` | `uint8_t` | Relative index (`0` to `8`) or absolute (`4` to `12`) |
| `texture` | String | Tattoo texture name in `player.img` |
| `rebuild` | `uint8_t` | `1` = Rebuild mesh immediately, `0` = Wait |

#### 9. `CJ_ACTION_CLEAR_CACHE` (0x09)
Purges client-side cached outfit and body state for the target player ID.
| Field | Type | Description |
|---|---|---|
| *(none)* | | Target player ID is specified in packet header. |

#### 10. `CJ_ACTION_REQUEST_SYNC` (0x0A)
Sent by client ASI to server when a remote ped spawns with Skin 0 but has no cached outfit.
| Field | Type | Description |
|---|---|---|
| *(none)* | | Target player ID is specified in packet header. Server replies with `CJ_ACTION_SET_FULL_OUTFIT`. |

#### 11. `CJ_ACTION_HANDSHAKE_PING` (0x0B)
Server pings client to verify if the BrownTurbo-ExtendedGame ASI plugin is installed.
| Field | Type | Description |
|---|---|---|
| `challenge` | `uint32_t` | Optional nonce / challenge value |

#### 12. `CJ_ACTION_HANDSHAKE_PONG` (0x0C)
Client confirms ASI plugin presence and reports version.
| Field | Type | Description |
|---|---|---|
| `magic` | `uint32_t` | Constant `0x42544547` (`'BTEG'`) |
| `version` | `uint16_t` | Plugin version (e.g. `0x0001` = v0.0.1 Alpha) |
| `challenge` | `uint32_t` | Nonce returned from ping |

#### 20. `EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT` (0x14)
Sets the aircraft ceiling height limit in meters (default: `800.0f`).
| Field | Type | Description |
|---|---|---|
| `heightLimit` | `float` | Aircraft ceiling height limit |

#### 21. `EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT` (0x15)
Sets the jetpack ceiling height limit in meters (default: `100.0f`, `<= 0.0f` for unlimited).
| Field | Type | Description |
|---|---|---|
| `heightLimit` | `float` | Jetpack ceiling height limit |

#### 22. `EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF` (0x16)
Controls whether the rider gets knocked off motorbikes / bicycles on crash.
| Field | Type | Description |
|---|---|---|
| `knockState` | `uint8_t` | `0` = Default, `1` = Never fall off, `2` = Always fall off |

#### 23. `EXTENDEDGAME_ACTION_SET_FIRE_PROOF` (0x17)
Toggles complete immunity to fire and burning.
| Field | Type | Description |
|---|---|---|
| `enable` | `uint8_t` | `1` = Fireproof, `0` = Normal damage |

#### 24. `EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT` (0x18)
Toggles infinite sprint capability (player never runs out of stamina).
| Field | Type | Description |
|---|---|---|
| `enable` | `uint8_t` | `1` = Infinite sprint, `0` = Normal stamina |

#### 25. `EXTENDEDGAME_ACTION_SET_SUN` (0x19)
Toggles the rendering of the sun corona and sky glare.
| Field | Type | Description |
|---|---|---|
| `enable` | `uint8_t` | `1` = Visible, `0` = Hidden |

Actions 26 and 27 are reserved.

#### 35. `EXTENDEDGAME_ACTION_SET_WATER_LEVEL` (0x23)
Sets the world water vertex heights, independently selecting non-sea world water, sea-level world water, and outside-world water.
| Field | Type | Description |
|---|---|---|
| `level` | `float` | Water height in meters |
| `waterParts` | `uint8_t` | Bit 0 = non-sea water, bit 1 = sea water, bit 2 = outside-world water |

#### 36. `EXTENDEDGAME_ACTION_SET_WATER_WAVE_HEIGHT` (0x24)
Sets water wave intensity as a percentage of GTA:SA's normal waviness. The input is mapped from `0..100` to a game multiplier of `0..1`; `-1` restores native weather-driven waves.
| Field | Type | Description |
|---|---|---|
| `height` | `float` | Intensity percent (`0.0` to `100.0`, mapped to a `0.0..1.0` GTA waviness multiplier); `-1.0` restores native weather-driven waves |

#### 37. `EXTENDEDGAME_ACTION_RESET_WATER_LEVEL` (0x25)
Restores captured GTA:SA world-water vertex heights and resets outside-world water to its default height. *(No additional payload.)*

#### 38. `EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION` (0x26)
Sets the rotation angle, pivot point, and continuous spin speed for a global TextDraw or Player TextDraw.
| Field | Type | Description |
|---|---|---|
| `textDrawId` | `uint16_t` | TextDraw slot ID (0..2047 for global, 0..255 for player textdraws) |
| `isPlayerTextDraw` | `uint8_t` | `1` = PlayerTextDraw, `0` = Global TextDraw |
| `angle` | `float` | Base rotation angle in degrees (clockwise) |
| `pivotX` | `float` | Pivot X coordinate (`-1.0` = auto-pivot at visual center, `0.0..1.0` = normalized screen space, `>1.0` = SA-MP 640x480 space) |
| `pivotY` | `float` | Pivot Y coordinate (`-1.0` = auto-pivot at visual center, `0.0..1.0` = normalized screen space, `>1.0` = SA-MP 640x480 space) |
| `spinSpeed` | `float` | Continuous client-side spin animation speed in degrees per second (`0.0` = static) |

#### 39. `EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION` (0x27)
Resets rotation and spin animation for a global TextDraw or Player TextDraw.
| Field | Type | Description |
|---|---|---|
| `textDrawId` | `uint16_t` | TextDraw slot ID |
| `isPlayerTextDraw` | `uint8_t` | `1` = PlayerTextDraw, `0` = Global TextDraw |

#### 40. `EXTENDEDGAME_ACTION_CREATE_WATER_ZONE` (0x28)
Creates a swimmable, buoyant, rendered water zone at custom world coordinates.
| Field | Type | Description |
|---|---|---|
| `zoneId` | `uint16_t` | Zone slot ID (`0..127`) |
| `minX` | `float` | Minimum world X |
| `minY` | `float` | Minimum world Y |
| `maxX` | `float` | Maximum world X |
| `maxY` | `float` | Maximum world Y |
| `level` | `float` | Water surface Z height |
| `bottomZ` | `float` | Water bed/bottom Z height (defaults to `-100.0`) |

#### 41. `EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL` (0x29)
Changes the surface level of an existing custom water zone.
| Field | Type | Description |
|---|---|---|
| `zoneId` | `uint16_t` | Zone slot ID (`0..127`) |
| `level` | `float` | New water surface Z height |

#### 42. `EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS` (0x2A)
Updates the bounds and levels of an existing custom water zone.
| Field | Type | Description |
|---|---|---|
| `zoneId` | `uint16_t` | Zone slot ID (`0..127`) |
| `minX` | `float` | Minimum world X |
| `minY` | `float` | Minimum world Y |
| `maxX` | `float` | Maximum world X |
| `maxY` | `float` | Maximum world Y |
| `level` | `float` | Water surface Z height |
| `bottomZ` | `float` | Water bed/bottom Z height |

#### 43. `EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE` (0x2B)
Destroys an active custom water zone and frees its slot.
| Field | Type | Description |
|---|---|---|
| `zoneId` | `uint16_t` | Zone slot ID (`0..127`) |

#### 44. `EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET` (0x2C)
Sets the 3D attachment translation offsets and rotation for the player's jetpack.
| Field | Type | Description |
|---|---|---|
| `offsetX`, `offsetY`, `offsetZ` | `float` | Positional offsets |
| `rotAngle` | `float` | Rotation angle in degrees |
| `rotAxisX`, `rotAxisY`, `rotAxisZ` | `float` | Rotation axis vector components |

#### 45. `EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET` (0x2D)
Resets jetpack placement and orientation to default GTA:SA coordinates. *(No payload)*

#### 46. `EXTENDEDGAME_ACTION_TOGGLE_NIGHT_VISION` (0x2E)
Toggles green night vision post-processing filter.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Enable, `0` = Disable |

#### 47. `EXTENDEDGAME_ACTION_TOGGLE_THERMAL_VISION` (0x2F)
Toggles infrared/heat-seeking thermal vision filter.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Enable, `0` = Disable |

#### 48. `EXTENDEDGAME_ACTION_SET_NO_RELOAD` (0x30)
Disables ammo decrement on weapon firing (infinite clip / no reload required).
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = No reload, `0` = Normal reload |

#### 49. `EXTENDEDGAME_ACTION_TOGGLE_SWITCH_RELOAD` (0x31)
Enables or disables auto-reloading when quickly switching away and back to a weapon.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Switch reload enabled, `0` = Disabled |

#### 50. `EXTENDEDGAME_ACTION_TOGGLE_QUICK_RELOAD` (0x32)
Instantly finishes weapon reload animation so the weapon is ready to fire immediately.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Instant reload, `0` = Normal reload animation |

#### 51. `EXTENDEDGAME_ACTION_TOGGLE_INFINITE_OXYGEN` (0x33)
Grants infinite lung capacity while swimming underwater.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Infinite breath, `0` = Normal oxygen consumption |

#### 52. `EXTENDEDGAME_ACTION_SET_PLAYER_PED_ANIMS` (0x34)
Switches CJ ped movement animations: CJ walk/run vs standard ped locomotion for Model 0.
| Field | Type | Description |
|---|---|---|
| `targetId` | `uint16_t` | Target player ID |
| `mode` | `uint8_t` | `1` = Force CJ locomotion, `2` = Force standard ped locomotion |

#### 53. `EXTENDEDGAME_ACTION_TOGGLE_CURSOR` (0x35)
Toggles the client mouse cursor.
| Field | Type | Description |
|---|---|---|
| `toggle` | `uint8_t` | `1` = Show cursor, `0` = Hide cursor |

#### 54. `EXTENDEDGAME_ACTION_SET_PLANE_RADAR_COLOR` (0x36)
Sets custom color tint for the plane radar altimeter artificial horizon line using a 32-bit hexadecimal color (`0xRRGGBBAA`).
| Field | Type | Description |
|---|---|---|
| `color` | `uint32_t` | Hexadecimal color in `0xRRGGBBAA` format |

#### 55. `EXTENDEDGAME_ACTION_PAUSE_MENU_EVENT` (0x37)
Sent by client ASI to server when the player opens, closes, or navigates submenus in the pause menu.
| Field | Type | Description |
|---|---|---|
| `eventType` | `uint8_t` | `1` = Open, `2` = Close, `3` = Submenu transition |
| `fromPage` | `uint8_t` | Previous pause menu page ID |
| `toPage` | `uint8_t` | Current pause menu page ID |

#### 56. `EXTENDEDGAME_ACTION_SET_SUN_COLOR` (0x38)
Overrides the game timecycle sun core and corona RGB colors.
| Field | Type | Description |
|---|---|---|
| `coreColor` | `uint32_t` | Sun core hexadecimal color (`0xRRGGBBAA`) |
| `coronaColor` | `uint32_t` | Sun corona flare hexadecimal color (`0xRRGGBBAA`) |

#### 57. `EXTENDEDGAME_ACTION_RESET_SUN_COLOR` (0x39)
Restores native weather timecycle sun coloring. *(No payload)*

#### 58. `EXTENDEDGAME_ACTION_SET_MOON_COLOR` (0x3A)
Overrides the moon texture and corona sprite colors.
| Field | Type | Description |
|---|---|---|
| `color` | `uint32_t` | Moon hexadecimal color (`0xRRGGBBAA`) |

#### 59. `EXTENDEDGAME_ACTION_RESET_MOON_COLOR` (0x3B)
Restores native moon coloring. *(No payload)*

#### 60. `EXTENDEDGAME_ACTION_TOGGLE_VEHICLE_BLUR` (0x3C)
Toggles high-speed motion blur for vehicles (which was disabled in SA-MP) and sets its intensity.
| Field | Type | Description |
|---|---|---|
| `enable` | `uint8_t` | `1` = Enable speed motion blur, `0` = Disable |
| `intensity` | `uint8_t` | Blur alpha intensity (0..255, default: `36`) |

---

## Clothes Slot Reference

| Slot ID | Constant | Description |
|---|---|---|
| `0` | `CJ_SLOT_TORSO` | Shirts, T-shirts, Hoodies, Jackets, Vests |
| `1` | `CJ_SLOT_HEAD` | Haircuts, Bald, Face Masks |
| `2` | `CJ_SLOT_LEGS` | Jeans, Trackpants, Shorts, Boxers |
| `3` | `CJ_SLOT_SHOES` | Sneakers, Boots, Sandals, Flip-flops |
| `4` | `CJ_SLOT_TATTOO_LOWER_LEFT` | Tattoo: Lower Left Arm |
| `5` | `CJ_SLOT_TATTOO_UPPER_LEFT` | Tattoo: Upper Left Arm |
| `6` | `CJ_SLOT_TATTOO_UPPER_RIGHT` | Tattoo: Upper Right Arm |
| `7` | `CJ_SLOT_TATTOO_LOWER_RIGHT` | Tattoo: Lower Right Arm |
| `8` | `CJ_SLOT_TATTOO_BACK_TOP` | Tattoo: Upper Back |
| `9` | `CJ_SLOT_TATTOO_LEFT_CHEST` | Tattoo: Left Chest |
| `10` | `CJ_SLOT_TATTOO_RIGHT_CHEST` | Tattoo: Right Chest |
| `11` | `CJ_SLOT_TATTOO_STOMACH` | Tattoo: Stomach |
| `12` | `CJ_SLOT_TATTOO_LOWER_BACK` | Tattoo: Lower Back |
| `13` | `CJ_SLOT_NECKLACE` | Chains, Necklaces, Bandanas |
| `14` | `CJ_SLOT_WATCH` | Watches, Bracelets |
| `15` | `CJ_SLOT_GLASSES` | Sunglasses, Tinted Shades, Glasses |
| `16` | `CJ_SLOT_HATS` | Caps, Hats, Helmets, Berets |
| `17` | `CJ_SLOT_SPECIAL` | Special Outfits (Cop, Valet, Medic, Pimp) |

---

## Server-Side (Pawn) Usage & Authoritative Synchronization

BrownTurbo-ExtendedGame includes a comprehensive server-side synchronization engine in [`pawn/extendedgame.inc`](pawn/extendedgame.inc).

### Key Server-Side Features:
- **Authoritative In-Memory Storage**: The server stores each player's active CJ outfit slots (`cj_szTexture`, `cj_szModel`) and body stats (`cj_fFat`, `cj_fMuscle`).
- **Automatic ALS Hooking**:
  - `OnPlayerConnect(playerid)`: Automatically initiates the ASI plugin presence verification handshake.
  - `OnPlayerStreamIn(playerid, forplayerid)`: Whenever `playerid` enters `forplayerid`'s streaming radius, the server automatically transmits `playerid`'s outfit, bike knock-off state, and fire-proof state.
  - `OnPlayerSpawn(playerid)`: Replays the player's bike and fire-proof states to the player and current stream viewers, and synchronizes CJ appearance.
  - `OnPlayerDisconnect(playerid)`: Automatically clears server-side state and broadcasts a cache purge.
  - `OnIncomingPacket(playerid, packetid, bs)`: Handles client sync requests and plugin handshakes. On handshake, the server replays all stored settings for that player's client and resends actor state for that player's currently streamed players.
- **Plugin Presence Verification & Crash Protection**:
  - `HasPlayerExtendedGame(playerid)`: Returns true if the client has verified the ASI plugin.
  - `GetPlayerExtendedGameVersion(playerid)`: Returns the client plugin version.
  - `SetPlayerSkinSafe(playerid, skinid, fallback_skinid)`: Prevents setting Model 0 if the player doesn't have the plugin installed, averting client crashes.
  - `OnPlayerExtendedGameCheck(playerid, bool:installed, version)`: Callback triggered once client handshake completes or times out.
- **Synchronization Scope**: Player-attached state (CJ appearance, bike knock-off, and fire-proof state, including default/disabled values) is sent to the player and viewers streaming them on stream-in and spawn. Client-wide environment and HUD settings remain scoped to the selected recipient; use `playerid = -1` for supported global setters to apply them to all current players and retain the value for future connections. Every stored client setting is replayed after the plugin handshake, and currently streamed actor state is replayed to late-handshaking clients.
- **Network Bandwidth Optimization**: Player-attached state is routed to the target and players currently streaming them (`IsPlayerStreamedIn` / Virtual World), rather than wasteful global broadcasts.

1. Copy [`pawn/extendedgame.inc`](pawn/extendedgame.inc) into your server's `pawno/include/` directory.
2. In your gamemode or filterscript:
```pawn
#include <a_samp>
#include <Pawn.RakNet>
#include <extendedgame>

// Callback triggered when plugin presence is resolved for a player
public OnPlayerExtendedGameCheck(playerid, bool:installed, version) {
    if (installed) {
        printf("[Plugin] Player %d has BrownTurbo-ExtendedGame ASI installed (v%d.%d)!", playerid, (version >> 8) & 0xFF, version & 0xFF);
    } else {
        printf("[Plugin] Player %d does NOT have BrownTurbo-ExtendedGame ASI installed.", playerid);
    }
    return 1;
}

// Example: Safely assign skin 0 only if player has the plugin installed
SetPlayerSkinSafe(playerid, 0, 299); // sets 299 if player doesn't have ASI installed!

// Example: Change player's shirt to green hoodie (auto-syncs to all streaming players)
SetPlayerCJClothes(-1, playerid, CJ_SLOT_TORSO, "hoodyam", "hoody", true);

// Example: Change player's haircut to Afro
SetPlayerCJClothes(-1, playerid, CJ_SLOT_HEAD, "afro", "head", true);

// Example: Put on aviator sunglasses
SetPlayerCJClothes(-1, playerid, CJ_SLOT_GLASSES, "shades", "glasses", true);

// Example: Make CJ muscular (fat: 0, muscle: 1000)
SetPlayerCJBodyStats(-1, playerid, 0.0, 1000.0, true);

// Example: Set full coordinated outfit in one network packet
new slots[4] = { CJ_SLOT_TORSO, CJ_SLOT_LEGS, CJ_SLOT_SHOES, CJ_SLOT_HEAD };
new textures[4][32] = { "hoodyam", "jeansdenim", "sneakerbklc", "dreads" };
new models[4][32] = { "hoody", "jeans", "sneaker", "head" };
SetPlayerCJFullOutfit(-1, playerid, 100.0, 600.0, slots, textures, models, 4);

// Example: Query player's server-side outfit
if (IsPlayerUsingCJSkin(playerid)) {
    new Float:fat, Float:muscle, tex[32], mdl[32];
    GetPlayerCJBodyStats(playerid, fat, muscle);
    GetPlayerCJClothes(playerid, CJ_SLOT_TORSO, tex, mdl);
}

// Example: Reset to default singleplayer clothes
ResetPlayerCJClothes(-1, playerid, true, true);

// Example: Clear client cache
ClearPlayerCJCache(-1, playerid);

// --- UG:MP Parity Gameplay & Environment Functions ---

// 1. Set aircraft height ceiling (in meters)
SetAircraftHeightLimitForPlayer(playerid, 1500.0);
new Float:curAir = GetPlayerAircraftHeightLimit(playerid);

// 2. Set jetpack height ceiling (in meters)
SetJetpackHeightLimitForPlayer(playerid, 350.0);
new Float:curJet = GetPlayerJetpackHeightLimit(playerid);

// 3. Control bike knock-off behavior
SetPlayerKnockedOffBikeState(playerid, BIKE_KNOCK_OFF_NEVER); // 0=Default, 1=Never, 2=Always

// 4. Set fire immunity
SetPlayerFireProof(playerid, true);

// 5. Toggle infinite sprint (never tired)
TogglePlayerInfiniteSprint(playerid, true);

// 6. Toggle sun and flare visibility
TogglePlayerSun(playerid, false); // Hides sun

// --- UG:MP Parity Sky & Environment Controls ---

// Toggle moon globally or per-player
ToggleMoon(false); // Global
TogglePlayerMoon(playerid, true); // Per-player
new bool:moonOn = IsPlayerMoonEnabled(playerid);

// Toggle stars globally or per-player
ToggleStars(false);
TogglePlayerStars(playerid, true);
new bool:starsOn = IsPlayerStarsEnabled(playerid);

// Toggle low clouds billboards globally or per-player
ToggleLowClouds(false);
TogglePlayerLowClouds(playerid, true);
new bool:lowCloudsOn = IsPlayerLowCloudsEnabled(playerid);

// Toggle fluffy volumetric clouds globally or per-player
ToggleFluffyClouds(false);
TogglePlayerFluffyClouds(playerid, true);
new bool:fluffyOn = IsPlayerFluffyCloudsEnabled(playerid);

// Toggle rainbow globally or per-player
ToggleRainbow(true);
TogglePlayerRainbow(playerid, false);
new bool:rainbowOn = IsPlayerRainbowEnabled(playerid);

// --- MTA-style Water Controls ---

// Set this player's sea and non-sea world water; optionally include outside-world water.
SetPlayerWaterLevel(playerid, 5.0, true, true, false);
new Float:waterLevel = GetPlayerWaterLevel(playerid);
ResetPlayerWaterLevel(playerid);

// Set wave intensity percent (0..100); -1 restores native weather-driven waves.
SetPlayerWaterWaveHeight(playerid, 5.0);
SetPlayerWaterWaveHeight(-1, 12.0); // Set for all current and future players
new Float:waveHeight = GetPlayerWaterWaveHeight(playerid);

// --- UG:MP Parity HUD Components Controls ---

// Toggle individual or multiple HUD components using bitmask
// Component masks:
//   HUD_COMPONENT_AMMO         (1)
//   HUD_COMPONENT_WEAPONICON   (2)
//   HUD_COMPONENT_HEALTH       (4)
//   HUD_COMPONENT_OXYGEN       (8)
//   HUD_COMPONENT_ARMOUR       (16)
//   HUD_COMPONENT_RADAR        (32)
//   HUD_COMPONENT_CROSSHAIR    (64)
//   HUD_COMPONENT_MONEY        (128)
//   HUD_COMPONENT_LASERDOT     (256)
//   HUD_COMPONENT_ALL          (511)
TogglePlayerHUDComponents(playerid, HUD_COMPONENT_ALL & ~HUD_COMPONENT_MONEY); // Hide money only
new hudMask = GetPlayerHUDComponents(playerid);

// Change colors of specific HUD components (RGBA integer)
SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_HEALTH, 0xFF0000FF); // Red health bar
SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_LASERDOT, 0x00FF00FF); // Green sniper laser dot
new col = GetPlayerHUDComponentColour(playerid, HUD_COMPONENT_HEALTH);

// --- TextDraw Rotation & Spin Animation ---

// Rotate a global TextDraw 45 degrees with auto-pivot at its visual center
SetTextDrawRotation(myTextDraw, 45.0);

// Rotate and continuously spin a global TextDraw at 90 deg/s
SetTextDrawRotation(myTextDraw, 0.0, -1.0, -1.0, 90.0);

// Spin a player textdraw around an explicit screen pivot (640x480 coordinate space)
SetPlayerTextDrawRotation(playerid, myPlayerTD, 0.0, 320.0, 240.0, 180.0);

// Reset rotation
ResetTextDrawRotation(myTextDraw);
ResetPlayerTextDrawRotation(playerid, myPlayerTD);

// --- Custom Water Zones ---

// Create a global swimmable water pool (minX, minY, maxX, maxY, surfaceZ, bottomZ)
new zoneId = CreateWaterZone(1000.0, -2000.0, 1100.0, -1900.0, 15.0, -5.0);

// Dynamically change water height
SetWaterZoneLevel(zoneId, 18.0);

// Query water zone properties
new Float:level;
GetWaterZoneLevel(zoneId, level);

// Destroy custom water zone
DestroyWaterZone(zoneId);

// --- Vision Controls ---
ToggleNightVision(playerid, true);
ToggleThermalVision(playerid, false);

// --- Weapon Reload Mechanics ---
SetPlayerNoReload(playerid, true);           // Infinite magazine clip
TogglePlayerSwitchReload(playerid, false);   // Disable weapon switch auto-reload
ToggleQuickReloadForPlayer(playerid, true);  // Instant reload animation

// --- Physics & Movement ---
TogglePlayerInfiniteOxygen(playerid, true);  // Infinite underwater breath
SetPlayerPedAnims(playerid, false);          // Disable CJ walk/run anims (standard ped anims)
TogglePlayerCursor(playerid, true);          // Show cursor

// --- Plane Radar Altimeter Color (Hexadecimal 0xRRGGBBAA) ---
SetPlaneRadarColor(playerid, 0x00FFFFFF);      // Cyan altimeter horizon line
SetPlaneRadarColorForAll(0xFF0000FF);          // Red altimeter for all players

// --- Sun Color Overrides (Hexadecimal 0xRRGGBBAA) ---
SetPlayerSunColor(playerid, 0xFF2020FF, 0xFF8800AA); // Red core, orange corona
ResetPlayerSunColor(playerid);                       // Restore default timecycle sun
SetSunColorForAll(0xFFFF00FF, 0xFFA500FF);           // Set gold sun for all players
ResetSunColorForAll();

// --- Moon Color Overrides (Hexadecimal 0xRRGGBBAA) ---
SetPlayerMoonColor(playerid, 0xFF2020FF);      // Blood moon
ResetPlayerMoonColor(playerid);                // Restore default moon
SetMoonColorForAll(0x4080FFFF);                // Blue moon for all players
ResetMoonColorForAll();

// --- Vehicle Motion Blur (Restored in SA-MP) ---
TogglePlayerVehicleBlur(playerid, true, 36);   // Enable speed motion blur (intensity 0..255)
ToggleVehicleBlurForAll(true, 50);             // Enable motion blur for all players

// --- Pause Menu Tracking Callbacks ---
public OnPlayerOpenPauseMenu(playerid) {
    printf("Player %d opened the pause menu.", playerid);
    return 1;
}

public OnPlayerClosePauseMenu(playerid) {
    printf("Player %d closed the pause menu.", playerid);
    return 1;
}

public OnPlayerEnterPauseSubmenu(playerid, from, to) {
    printf("Player %d moved in pause menu from page %d to %d.", playerid, from, to);
    return 1;
}
```

See [`pawn/extendedgame_example.pwn`](pawn/extendedgame_example.pwn) for a complete working filterscript with commands (`/shirt`, `/pants`, `/shoes`, `/hair`, `/glasses`, `/hat`, `/body`, `/tattoo`, `/outfit`, `/bare`, `/clearcache`, `/myoutfit`, `/checkplugin`, `/setcj`, `/airlimit`, `/jetlimit`, `/bikerule`, `/fireproof`, `/sprint`, `/sun`, `/moon`, `/stars`, `/lowclouds`, `/fluffyclouds`, `/rainbow`, `/hud`, `/hudcol`, `/tdhelp`, `/tdcreate`, `/tdrotate`, `/tdspin`, `/tdreset`, `/tddestroy`, `/ptdcreate`, `/ptdrotate`, `/ptdspin`, `/ptdreset`, `/ptddestroy`, `/wzcreate`, `/wzlevel`, `/wzdestroy`, `/nv`, `/tv`, `/noreload`, `/switchreload`, `/quickreload`, `/oxygen`, `/pedanims`, `/cursor`, `/planeradar`, `/suncolor`, `/resetsuncolor`, `/mooncolor`, `/resetmooncolor`, `/vehicleblur`, `/hudwanted`, `/hudradio`, `/hudclock`, `/ispause`).

---

## Client-Side Installation

1. Compile the plugin (or use pre-built binary):
   Output is generated at `bin/Release/brownturbo-extendedgame.asi`.
2. Copy `brownturbo-extendedgame.asi` into your GTA San Andreas root directory (where `gta_sa.exe` and an ASI loader such as Silent's ASI Loader or VorbisFile ASI Loader are installed).

---

## Building from Source

### Prerequisites
- Windows OS
- CMake (>= 3.23)
- Visual Studio 2022 / 2026 Build Tools (with C++ Desktop workload and x86 compiler)

### Build
```powershell
# Generate Visual Studio 32-bit project
cmake -B build -A Win32

# Compile Release binary
cmake --build build --config Release
```
