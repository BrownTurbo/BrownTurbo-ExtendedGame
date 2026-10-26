#if defined _extendedgame_included
    #endinput
#endif
#define _extendedgame_included
#define _extendedgame_included


#if !defined _pawn_raknet_included && !defined PAWNRAKNET_INC_
    #tryinclude <Pawn.RakNet>
#endif

#if !defined _pawn_raknet_included && !defined PAWNRAKNET_INC_
    #error "Please include <Pawn.RakNet as it is a requirement for extendedgame."
#endif

#if !defined MAX_PLAYERS
    #define MAX_PLAYERS                 (1000)
#endif

// Packet Identification
#define EXTENDEDGAME_PACKET_ID          (252)
#define EXTENDEDGAME_TARGET_LOCAL_PLAYER (0xFFFF)

// Versioning & Protocol Magic
#define EXTENDEDGAME_VERSION            (0x0001)     // v0.0.1 (Alpha)
#define EXTENDEDGAME_VERSION_STR        "0.0.1-alpha"
#define EXTENDEDGAME_HANDSHAKE_MAGIC    (0x42544547) // 'BTEG' (BrownTurbo ExtendedGame)

// Packet Sub-actions
#define CJ_ACTION_SET_CLOTHES           (1)
#define CJ_ACTION_REMOVE_CLOTHES        (2)
#define CJ_ACTION_SET_BODY_STATS        (3)
#define CJ_ACTION_RESET_CLOTHES         (4)
#define CJ_ACTION_REBUILD               (5)
#define CJ_ACTION_SET_FULL_OUTFIT       (6)
#define CJ_ACTION_SET_CLOTHES_HASH      (7)
#define CJ_ACTION_SET_TATTOO            (8)
#define CJ_ACTION_CLEAR_CACHE           (9)
#define CJ_ACTION_REQUEST_SYNC          (10) // Client fallback request for uncached stream-in
#define CJ_ACTION_HANDSHAKE_PING        (11) // Server pings client to verify ASI plugin presence
#define CJ_ACTION_HANDSHAKE_PONG        (12) // Client responds to confirm ASI plugin presence & version
#define CJ_ACTION_HANDSHAKE_ACK         (13) // Server acknowledges handshake; confirms communication channel active

// ExtendedGame Gameplay & Environment Actions (UG:MP Parity)
#define EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT (20)
#define EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT  (21)
#define EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF        (22)
#define EXTENDEDGAME_ACTION_SET_FIRE_PROOF            (23)
#define EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT (24)
#define EXTENDEDGAME_ACTION_SET_SUN             (25)
// Actions 26 and 27 are reserved.

// ExtendedGame Sky & Environment Controls (UG:MP Parity)
#define EXTENDEDGAME_ACTION_TOGGLE_MOON               (28)
#define EXTENDEDGAME_ACTION_TOGGLE_STARS              (29)
#define EXTENDEDGAME_ACTION_TOGGLE_LOW_CLOUDS         (30)
#define EXTENDEDGAME_ACTION_TOGGLE_FLUFFY_CLOUDS      (31)
#define EXTENDEDGAME_ACTION_TOGGLE_RAINBOW            (32)

// ExtendedGame HUD Component Controls (UG:MP Parity)
#define EXTENDEDGAME_ACTION_TOGGLE_HUD_COMPONENTS     (33)
#define EXTENDEDGAME_ACTION_SET_HUD_COMPONENTS_COLOUR (34)
#define EXTENDEDGAME_ACTION_SET_WATER_LEVEL           (35)
#define EXTENDEDGAME_ACTION_SET_WATER_WAVE_HEIGHT     (36)
#define EXTENDEDGAME_ACTION_RESET_WATER_LEVEL         (37)

// ExtendedGame TextDraw Rotation Actions
#define EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION     (38)
#define EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION   (39)

// ExtendedGame Custom Water Zones Actions
#define EXTENDEDGAME_ACTION_CREATE_WATER_ZONE         (40)
#define EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL      (41)
#define EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS     (42)
#define EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE        (43)

// ExtendedGame Jetpack Placement & Orientation Actions
#define EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET        (44)
#define EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET      (45)

#define MAX_WATER_ZONES                               (128)
#define INVALID_WATER_ZONE                            (0xFFFF)

enum E_EXTG_WATER_ZONE {
    bool:wzActive,
    Float:wzMinX,
    Float:wzMinY,
    Float:wzMaxX,
    Float:wzMaxY,
    Float:wzLevel,
    Float:wzBottomZ
};
static stock g_GlobalWaterZones[MAX_WATER_ZONES][E_EXTG_WATER_ZONE];
static stock g_ActiveWaterZonesCount = 0;

static stock bool:g_GlobalWaterLevelSet;
static stock Float:g_GlobalWaterLevel;
static stock bool:g_GlobalWaterIncludeNonSea = true;
static stock bool:g_GlobalWaterIncludeSea = true;
static stock bool:g_GlobalWaterIncludeOutside = true;
static stock Float:g_GlobalWaterWaveHeight;

enum E_EXTG_TD_ROTATION {
    bool:tdRotEnabled,
    Float:tdRotAngle,
    Float:tdRotPivotX,
    Float:tdRotPivotY,
    Float:tdRotSpinSpeed
};
static stock g_GlobalTextDrawRotation[MAX_TEXT_DRAWS][E_EXTG_TD_ROTATION];
static stock g_ActiveTextDrawRotationsCount = 0;
static stock g_PlayerTextDrawRotation[MAX_PLAYERS][MAX_PLAYER_TEXT_DRAWS][E_EXTG_TD_ROTATION];

// HUD Component Defines (UG:MP Parity)
#define HUD_COMPONENT_AMMO                  1
#define HUD_COMPONENT_WEAPONICON            2
#define HUD_COMPONENT_HEALTH                4
#define HUD_COMPONENT_OXYGEN                8
#define HUD_COMPONENT_ARMOUR                16
#define HUD_COMPONENT_RADAR                 32
#define HUD_COMPONENT_CROSSHAIR             64
#define HUD_COMPONENT_MONEY                 128
#define HUD_COMPONENT_LASERDOT              256
#define HUD_COMPONENT_ALL                   (511)

//// Affects how players can get knocked off bikes
#define BIKE_KNOCK_OFF_DEFAULT          (0)
#define BIKE_KNOCK_OFF_NEVER            (1)
#define BIKE_KNOCK_OFF_ALWAYS           (2)

// Clothes Slot IDs (eClothesTexturePart)
#define CJ_SLOT_TORSO                   (0)  // Shirts, Jackets, Hoodies, Vests
#define CJ_SLOT_HEAD                    (1)  // Haircuts, Bald, Masks
#define CJ_SLOT_LEGS                    (2)  // Pants, Jeans, Shorts, Trackpants
#define CJ_SLOT_SHOES                   (3)  // Sneakers, Boots, Sandals
#define CJ_SLOT_TATTOO_LOWER_LEFT       (4)  // Lower Left Arm Tattoo
#define CJ_SLOT_TATTOO_UPPER_LEFT       (5)  // Upper Left Arm Tattoo
#define CJ_SLOT_TATTOO_UPPER_RIGHT      (6)  // Upper Right Arm Tattoo
#define CJ_SLOT_TATTOO_LOWER_RIGHT      (7)  // Lower Right Arm Tattoo
#define CJ_SLOT_TATTOO_BACK_TOP         (8)  // Upper Back Tattoo
#define CJ_SLOT_TATTOO_LEFT_CHEST       (9)  // Left Chest Tattoo
#define CJ_SLOT_TATTOO_RIGHT_CHEST      (10) // Right Chest Tattoo
#define CJ_SLOT_TATTOO_STOMACH          (11) // Stomach Tattoo
#define CJ_SLOT_TATTOO_LOWER_BACK       (12) // Lower Back Tattoo
#define CJ_SLOT_NECKLACE                (13) // Chains, Necklaces, Bandanas
#define CJ_SLOT_WATCH                   (14) // Watches, Bracelets
#define CJ_SLOT_GLASSES                 (15) // Sunglasses, Shades
#define CJ_SLOT_HATS                    (16) // Baseball Caps, Hats, Helmets
#define CJ_SLOT_SPECIAL                 (17) // Special Outfits / Suits (Cop, Valet, etc.)
#define CJ_SLOT_COUNT                   (18)

// Tattoo Relative Slots (0 to 8)
#define CJ_TATTOO_LOWER_LEFT            (0)
#define CJ_TATTOO_UPPER_LEFT            (1)
#define CJ_TATTOO_UPPER_RIGHT           (2)
#define CJ_TATTOO_LOWER_RIGHT           (3)
#define CJ_TATTOO_BACK_TOP              (4)
#define CJ_TATTOO_LEFT_CHEST            (5)
#define CJ_TATTOO_RIGHT_CHEST           (6)
#define CJ_TATTOO_STOMACH               (7)
#define CJ_TATTOO_LOWER_BACK            (8)

// Popular Singleplayer Clothing Presets (Texture, Model)
#define CJ_CLOTHES_TORSO_VEST           "vest", "vest"
#define CJ_CLOTHES_TORSO_HOODIE_BLACK   "hoodyam", "hoody"
#define CJ_CLOTHES_TORSO_TSHIRT_WHITE   "tshirtwhite", "tshirt"
#define CJ_CLOTHES_TORSO_SHIRT_PLAID    "shirtbplaid", "shirt"
#define CJ_CLOTHES_TORSO_LEATHER        "leather", "leather"

#define CJ_CLOTHES_LEGS_JEANS           "jeansdenim", "jeans"
#define CJ_CLOTHES_LEGS_TRACKPANTS      "tracktrblue", "tracktr"
#define CJ_CLOTHES_LEGS_BOXERS          "boxer", "boxer"

#define CJ_CLOTHES_SHOES_SNEAKERS       "sneakerbincblk", "sneaker"
#define CJ_CLOTHES_SHOES_COWBOY         "cowboyboot", "cowboy"
#define CJ_CLOTHES_SHOES_FLIPFLOPS      "flipflop", "flipflop"

#define CJ_CLOTHES_HEAD_CAESAR          "player_face", "head"
#define CJ_CLOTHES_HEAD_AFRO            "afro", "afro"
#define CJ_CLOTHES_HEAD_DREADS          "cornrows", "cornrows"
#define CJ_CLOTHES_HEAD_CORNROWS        "cornrows", "cornrows"
#define CJ_CLOTHES_HEAD_BALD            "bald", "head"

// Server-side authoritative storage
static stock bool:g_PlayerCJActive[MAX_PLAYERS];
static stock Float:g_PlayerCJFat[MAX_PLAYERS];
static stock Float:g_PlayerCJMuscle[MAX_PLAYERS];
static stock g_PlayerCJTexture[MAX_PLAYERS][CJ_SLOT_COUNT][32];
static stock g_PlayerCJModel[MAX_PLAYERS][CJ_SLOT_COUNT][32];

// Server-side presence tracking for client ASI plugin
static stock bool:g_PlayerHasPlugin[MAX_PLAYERS];
static stock g_PlayerPluginVersion[MAX_PLAYERS];
static stock bool:g_PlayerPluginChecked[MAX_PLAYERS];

// ExtendedGame server-side authoritative state storage
static stock Float:g_PlayerAircraftHeightLimit[MAX_PLAYERS];
static stock Float:g_PlayerJetpackHeightLimit[MAX_PLAYERS];
static stock Float:g_GlobalAircraftHeightLimit = 800.0;
static stock Float:g_GlobalJetpackHeightLimit = 100.0;

// ExtendedGame Jetpack Placement & Orientation storage
static stock Float:g_PlayerJetpackOffsetX[MAX_PLAYERS];
static stock Float:g_PlayerJetpackOffsetY[MAX_PLAYERS];
static stock Float:g_PlayerJetpackOffsetZ[MAX_PLAYERS];
static stock Float:g_PlayerJetpackRotAngle[MAX_PLAYERS];
static stock Float:g_PlayerJetpackRotAxisX[MAX_PLAYERS];
static stock Float:g_PlayerJetpackRotAxisY[MAX_PLAYERS];
static stock Float:g_PlayerJetpackRotAxisZ[MAX_PLAYERS];

static stock Float:g_GlobalJetpackOffsetX  = 0.10;
static stock Float:g_GlobalJetpackOffsetY  = -0.38;
static stock Float:g_GlobalJetpackOffsetZ  = 0.00;
static stock Float:g_GlobalJetpackRotAngle = 90.0;
static stock Float:g_GlobalJetpackRotAxisX = 0.0;
static stock Float:g_GlobalJetpackRotAxisY = 1.0;
static stock Float:g_GlobalJetpackRotAxisZ = 0.0;
static stock g_PlayerKnockOffBike[MAX_PLAYERS];
static stock bool:g_PlayerFireProof[MAX_PLAYERS];
static stock bool:g_PlayerInfiniteSprint[MAX_PLAYERS];
static stock bool:g_PlayerSunEnabled[MAX_PLAYERS];
static stock g_GlobalKnockOffBike;
static stock bool:g_GlobalFireProof;
static stock bool:g_GlobalInfiniteSprint;
static stock bool:g_GlobalSunEnabled = true;

// Global Sky Defaults
static stock bool:g_GlobalMoonEnabled         = true;
static stock bool:g_GlobalStarsEnabled        = true;
static stock bool:g_GlobalLowCloudsEnabled    = true;
static stock bool:g_GlobalFluffyCloudsEnabled = true;
static stock bool:g_GlobalRainbowEnabled      = true;

// ExtendedGame Sky and HUD storage
static stock bool:g_PlayerMoonEnabled[MAX_PLAYERS];
static stock bool:g_PlayerStarsEnabled[MAX_PLAYERS];
static stock bool:g_PlayerLowCloudsEnabled[MAX_PLAYERS];
static stock bool:g_PlayerFluffyCloudsEnabled[MAX_PLAYERS];
static stock bool:g_PlayerRainbowEnabled[MAX_PLAYERS];

static stock g_PlayerHUDComponents[MAX_PLAYERS];
static stock g_PlayerHUDColourAmmo[MAX_PLAYERS];
static stock g_PlayerHUDColourWeaponIcon[MAX_PLAYERS];
static stock g_PlayerHUDColourHealth[MAX_PLAYERS];
static stock g_PlayerHUDColourOxygen[MAX_PLAYERS];
static stock g_PlayerHUDColourArmour[MAX_PLAYERS];
static stock g_PlayerHUDColourRadar[MAX_PLAYERS];
static stock g_PlayerHUDColourCrosshair[MAX_PLAYERS];
static stock g_PlayerHUDColourMoney[MAX_PLAYERS];
static stock g_PlayerHUDColourLaserDot[MAX_PLAYERS];
static stock g_GlobalHUDComponents = 511;
static stock g_GlobalHUDColours[9];
static stock bool:g_PlayerWaterLevelSet[MAX_PLAYERS];
static stock Float:g_PlayerWaterLevel[MAX_PLAYERS];
static stock bool:g_PlayerWaterIncludeNonSea[MAX_PLAYERS];
static stock bool:g_PlayerWaterIncludeSea[MAX_PLAYERS];
static stock bool:g_PlayerWaterIncludeOutside[MAX_PLAYERS];
static stock Float:g_PlayerWaterWaveHeight[MAX_PLAYERS];

static stock bool:_IsPlayerStreamedIn(targetid, forplayerid)
{
    if (targetid < 0 || targetid >= MAX_PLAYERS || forplayerid < 0 || forplayerid >= MAX_PLAYERS)
        return false;
    if (!IsPlayerConnected(targetid) || !IsPlayerConnected(forplayerid))
        return false;
    return !!IsPlayerStreamedIn(targetid, forplayerid);
}

static stock _SendPlayerBikeState(recipient, targetid)
{
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF,
        PR_UINT8, g_PlayerKnockOffBike[targetid]
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

static stock _SendPlayerFireProofState(recipient, targetid)
{
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_FIRE_PROOF,
        PR_UINT8, g_PlayerFireProof[targetid] ? 1 : 0
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

static stock _SyncExtGameActorState(targetid, recipient)
{
    if (targetid < 0 || targetid >= MAX_PLAYERS || !IsPlayerConnected(targetid) ||
        recipient < 0 || recipient >= MAX_PLAYERS || !IsPlayerConnected(recipient))
        return 0;

    if (g_PlayerCJActive[targetid] || GetPlayerSkin(targetid) == 0)
        SyncPlayerCJForPlayer(targetid, recipient);

    _SendPlayerBikeState(recipient, targetid);
    _SendPlayerFireProofState(recipient, targetid);
    return 1;
}

static stock _SyncPlayerExtendedGameSettings(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid))
        return 0;

    SetAircraftHeightLimitForPlayer(playerid, g_PlayerAircraftHeightLimit[playerid]);
    SetJetpackHeightLimitForPlayer(playerid, g_PlayerJetpackHeightLimit[playerid]);
    _SendJetpackOffsetPacket(playerid,
        g_PlayerJetpackOffsetX[playerid],
        g_PlayerJetpackOffsetY[playerid],
        g_PlayerJetpackOffsetZ[playerid],
        g_PlayerJetpackRotAngle[playerid],
        g_PlayerJetpackRotAxisX[playerid],
        g_PlayerJetpackRotAxisY[playerid],
        g_PlayerJetpackRotAxisZ[playerid]
    );
    TogglePlayerInfiniteSprint(playerid, g_PlayerInfiniteSprint[playerid]);
    TogglePlayerSun(playerid, g_PlayerSunEnabled[playerid]);
    TogglePlayerMoon(playerid, g_PlayerMoonEnabled[playerid]);
    TogglePlayerStars(playerid, g_PlayerStarsEnabled[playerid]);
    TogglePlayerLowClouds(playerid, g_PlayerLowCloudsEnabled[playerid]);
    TogglePlayerFluffyClouds(playerid, g_PlayerFluffyCloudsEnabled[playerid]);
    TogglePlayerRainbow(playerid, g_PlayerRainbowEnabled[playerid]);
    TogglePlayerHUDComponents(playerid, g_PlayerHUDComponents[playerid]);

    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_AMMO, g_PlayerHUDColourAmmo[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_WEAPONICON, g_PlayerHUDColourWeaponIcon[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_HEALTH, g_PlayerHUDColourHealth[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_OXYGEN, g_PlayerHUDColourOxygen[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_ARMOUR, g_PlayerHUDColourArmour[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_RADAR, g_PlayerHUDColourRadar[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_CROSSHAIR, g_PlayerHUDColourCrosshair[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_MONEY, g_PlayerHUDColourMoney[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_LASERDOT, g_PlayerHUDColourLaserDot[playerid]);

    if (g_PlayerWaterLevelSet[playerid]) {
        SetPlayerWaterLevel(
            playerid,
            g_PlayerWaterLevel[playerid],
            g_PlayerWaterIncludeNonSea[playerid],
            g_PlayerWaterIncludeSea[playerid],
            g_PlayerWaterIncludeOutside[playerid]
        );
    } else {
        ResetPlayerWaterLevel(playerid);
    }
    SetPlayerWaterWaveHeight(playerid, g_PlayerWaterWaveHeight[playerid]);

    if (g_ActiveTextDrawRotationsCount > 0) {
        for (new _td = 0; _td < MAX_TEXT_DRAWS; _td++) {
            if (g_GlobalTextDrawRotation[_td][tdRotEnabled]) {
                SetTextDrawRotationForPlayer(
                    playerid,
                    Text:_td,
                    g_GlobalTextDrawRotation[_td][tdRotAngle],
                    g_GlobalTextDrawRotation[_td][tdRotPivotX],
                    g_GlobalTextDrawRotation[_td][tdRotPivotY],
                    g_GlobalTextDrawRotation[_td][tdRotSpinSpeed]
                );
            }
        }
    }

    if (g_ActiveWaterZonesCount > 0) {
        for (new _wz = 0; _wz < MAX_WATER_ZONES; _wz++) {
            if (g_GlobalWaterZones[_wz][wzActive]) {
                CreateWaterZoneForPlayer(
                    playerid,
                    _wz,
                    g_GlobalWaterZones[_wz][wzMinX],
                    g_GlobalWaterZones[_wz][wzMinY],
                    g_GlobalWaterZones[_wz][wzMaxX],
                    g_GlobalWaterZones[_wz][wzMaxY],
                    g_GlobalWaterZones[_wz][wzLevel],
                    g_GlobalWaterZones[_wz][wzBottomZ]
                );
            }
        }
    }
    return 1;
}

// Forward callbacks triggered when a player's plugin status is resolved
forward OnPlayerExtendedGameCheck(playerid, bool:installed, version);

/**
 * Checks if a player has the BrownTurbo-ExtendedGame ASI plugin installed
 */
stock bool:HasPlayerExtendedGame(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return g_PlayerHasPlugin[playerid];
}

/**
 * Returns the version of the BrownTurbo-ExtendedGame ASI plugin installed by the player
 * Returns 0 if not installed
 */
stock GetPlayerExtendedGameVersion(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    return g_PlayerPluginVersion[playerid];
}

/**
 * Sends an active presence-check ping to a player
 */
stock CheckPlayerExtendedGame(playerid, challenge = 0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, CJ_ACTION_HANDSHAKE_PING,
        PR_UINT32, challenge
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Safely sets player skin: if skin is 0 (CJ) and player does not have the ASI plugin installed,
 * sets fallback_skinid instead to prevent GTA:SA client crash!
 */
stock SetPlayerSkinSafe(playerid, skinid, fallback_skinid = 299) {
    if (skinid == 0 && !HasPlayerExtendedGame(playerid)) {
        SetPlayerSkin(playerid, fallback_skinid);
        return 0; // Prevented crash
    }
    SetPlayerSkin(playerid, skinid);
    return 1;
}

/**
 * Internal helper to write an 8-bit length-prefixed ASCII string to BitStream
 * Works across all versions of Pawn.RakNet
 */
static stock _BS_WriteCJString(BitStream:bs, const string[]) {
    #if defined BS_WriteString8
        BS_WriteString8(bs, string);
    #elseif defined PR_STRING8
        BS_WriteValue(bs, PR_STRING8, string);
    #else
        new len = strlen(string);
        BS_WriteValue(bs, PR_UINT8, len);
        if (len > 0) {
            BS_WriteValue(bs, PR_STRING, string, len);
        }
    #endif
}

/**
 * Resets a player's server-side CJ outfit data
 */
stock ResetPlayerCJData(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerCJActive[playerid] = false;
    g_PlayerCJFat[playerid] = 0.0;
    g_PlayerCJMuscle[playerid] = 0.0;
    for (new s = 0; s < CJ_SLOT_COUNT; s++) {
        g_PlayerCJTexture[playerid][s][0] = '\0';
        g_PlayerCJModel[playerid][s][0] = '\0';
    }
    return 1;
}

/**
 * Checks whether a player currently has an active custom CJ skin on the server
 */
stock bool:IsPlayerUsingCJSkin(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return g_PlayerCJActive[playerid];
}

/**
 * Retrieves a player's current CJ body stats from server memory
 */
stock GetPlayerCJBodyStats(playerid, &Float:fat, &Float:muscle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    fat = g_PlayerCJFat[playerid];
    muscle = g_PlayerCJMuscle[playerid];
    return 1;
}

/**
 * Retrieves a player's current clothes item in a specific slot from server memory
 */
stock GetPlayerCJClothes(playerid, slot, texture[], model[], max_len = 32) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || slot < 0 || slot >= CJ_SLOT_COUNT) return 0;
    format(texture, max_len, "%s", g_PlayerCJTexture[playerid][slot]);
    format(model, max_len, "%s", g_PlayerCJModel[playerid][slot]);
    return 1;
}

static stock _PR_BroadcastPacket(BitStream:bs) {
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            PR_SendPacket(bs, i, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        }
    }
}

/**
 * Sends a raw clothes packet to a recipient
 */
static stock _SendRawCJClothesPacket(recipient, targetid, slot, const texture[], const model[], bool:rebuild) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_SET_CLOTHES,
        PR_UINT8, slot
    );
    _BS_WriteCJString(bs, texture);
    _BS_WriteCJString(bs, model);
    BS_WriteValue(bs, PR_UINT8, rebuild ? 1 : 0);

    if (recipient == -1) {
        _PR_BroadcastPacket(bs);
    } else if (IsPlayerConnected(recipient)) {
        PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

/**
 * Sends a raw full outfit packet to a recipient
 */
static stock _SendRawCJOutfitPacket(recipient, targetid, Float:fat, Float:muscle, const slots[], const textures[][], const models[][], count) {
    if (count > CJ_SLOT_COUNT) count = CJ_SLOT_COUNT;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_SET_FULL_OUTFIT,
        PR_FLOAT, fat,
        PR_FLOAT, muscle,
        PR_UINT8, count
    );
    for (new i = 0; i < count; i++) {
        BS_WriteValue(bs, PR_UINT8, slots[i]);
        _BS_WriteCJString(bs, textures[i]);
        _BS_WriteCJString(bs, models[i]);
    }

    if (recipient == -1) {
        _PR_BroadcastPacket(bs);
    } else if (IsPlayerConnected(recipient)) {
        PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

/**
 * Synchronizes targetid's complete CJ outfit to forplayerid
 * Called automatically when targetid streams in for forplayerid
 */
stock SyncPlayerCJForPlayer(targetid, forplayerid) {
    if (targetid < 0 || targetid >= MAX_PLAYERS || !IsPlayerConnected(targetid)) return 0;
    if (forplayerid < 0 || forplayerid >= MAX_PLAYERS || !IsPlayerConnected(forplayerid)) return 0;

    if (g_PlayerCJActive[targetid]) {
        new slots[CJ_SLOT_COUNT];
        new textures[CJ_SLOT_COUNT][32];
        new models[CJ_SLOT_COUNT][32];
        new count = 0;

        for (new s = 0; s < CJ_SLOT_COUNT; s++) {
            if (g_PlayerCJTexture[targetid][s][0] != '\0') {
                slots[count] = s;
                format(textures[count], 32, "%s", g_PlayerCJTexture[targetid][s]);
                format(models[count], 32, "%s", g_PlayerCJModel[targetid][s]);
                count++;
            }
        }

        _SendRawCJOutfitPacket(forplayerid, targetid,
            g_PlayerCJFat[targetid],
            g_PlayerCJMuscle[targetid],
            slots, textures, models, count
        );
        return 1;
    } else if (GetPlayerSkin(targetid) == 0) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, targetid,
            PR_UINT8, CJ_ACTION_RESET_CLOTHES,
            PR_UINT8, 1,
            PR_UINT8, 1
        );
        PR_SendPacket(bs, forplayerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
        return 1;
    }
    return 0;
}

/**
 * Synchronizes targetid's CJ outfit to targetid AND to all players streaming targetid
 */
stock SyncPlayerCJ(targetid) {
    if (targetid < 0 || targetid >= MAX_PLAYERS || !IsPlayerConnected(targetid)) return 0;

    SyncPlayerCJForPlayer(targetid, targetid);

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (i != targetid && IsPlayerConnected(i)) {
            if (_IsPlayerStreamedIn(targetid, i)) {
                SyncPlayerCJForPlayer(targetid, i);
            }
        }
    }
    return 1;
}

/**
 * Sets clothes/tattoo for a player, updates server-side authoritative state,
 * and synchronizes with all streamed players (or specific recipient).
 */
stock SetPlayerCJClothes(playerid, targetid, slot, const texture[], const model[], bool:rebuild = true) {
    if (slot < 0 || slot >= CJ_SLOT_COUNT) return 0;

    if (targetid >= 0 && targetid < MAX_PLAYERS) {
        g_PlayerCJActive[targetid] = true;
        format(g_PlayerCJTexture[targetid][slot], 32, "%s", texture);
        format(g_PlayerCJModel[targetid][slot], 32, "%s", model);
    }

    if (playerid == -1) {
        if (targetid >= 0 && targetid < MAX_PLAYERS && IsPlayerConnected(targetid)) {
            _SendRawCJClothesPacket(targetid, targetid, slot, texture, model, rebuild);
            for (new i = 0; i < MAX_PLAYERS; i++) {
                if (i != targetid && IsPlayerConnected(i)) {
                    if (_IsPlayerStreamedIn(targetid, i)) {
                        _SendRawCJClothesPacket(i, targetid, slot, texture, model, rebuild);
                    }
                }
            }
        } else {
            _SendRawCJClothesPacket(-1, targetid, slot, texture, model, rebuild);
        }
    } else {
        _SendRawCJClothesPacket(playerid, targetid, slot, texture, model, rebuild);
    }
    return 1;
}

/**
 * Removes clothes or tattoo from a specific slot
 */
stock RemovePlayerCJClothes(playerid, targetid, slot, bool:rebuild = true) {
    return SetPlayerCJClothes(playerid, targetid, slot, "", "", rebuild);
}

/**
 * Sets CJ fat and muscle stats (0.0 to 1000.0) and synchronizes
 */
stock SetPlayerCJBodyStats(playerid, targetid, Float:fat, Float:muscle, bool:rebuild = true) {
    if (targetid >= 0 && targetid < MAX_PLAYERS) {
        g_PlayerCJActive[targetid] = true;
        g_PlayerCJFat[targetid] = fat;
        g_PlayerCJMuscle[targetid] = muscle;
    }

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_SET_BODY_STATS,
        PR_FLOAT, fat,
        PR_FLOAT, muscle,
        PR_UINT8, rebuild ? 1 : 0
    );

    if (playerid == -1) {
        if (targetid >= 0 && targetid < MAX_PLAYERS && IsPlayerConnected(targetid)) {
            PR_SendPacket(bs, targetid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
            for (new i = 0; i < MAX_PLAYERS; i++) {
                if (i != targetid && IsPlayerConnected(i)) {
                    if (_IsPlayerStreamedIn(targetid, i)) {
                        PR_SendPacket(bs, i, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
                    }
                }
            }
        } else {
            _PR_BroadcastPacket(bs);
        }
    } else if (IsPlayerConnected(playerid)) {
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

/**
 * Resets CJ clothes to default singleplayer or bare
 */
stock ResetPlayerCJClothes(playerid, targetid, bool:to_default = true, bool:rebuild = true) {
    if (targetid >= 0 && targetid < MAX_PLAYERS) {
        ResetPlayerCJData(targetid);
        g_PlayerCJActive[targetid] = true;
        if (to_default) {
            format(g_PlayerCJTexture[targetid][CJ_SLOT_TORSO], 32, "vest");
            format(g_PlayerCJModel[targetid][CJ_SLOT_TORSO], 32, "vest");
            format(g_PlayerCJTexture[targetid][CJ_SLOT_LEGS], 32, "jeansdenim");
            format(g_PlayerCJModel[targetid][CJ_SLOT_LEGS], 32, "jeans");
            format(g_PlayerCJTexture[targetid][CJ_SLOT_SHOES], 32, "sneakerbincblk");
            format(g_PlayerCJModel[targetid][CJ_SLOT_SHOES], 32, "sneaker");
            format(g_PlayerCJTexture[targetid][CJ_SLOT_HEAD], 32, "player_face");
            format(g_PlayerCJModel[targetid][CJ_SLOT_HEAD], 32, "head");
        }
    }

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_RESET_CLOTHES,
        PR_UINT8, to_default ? 1 : 0,
        PR_UINT8, rebuild ? 1 : 0
    );

    if (playerid == -1) {
        if (targetid >= 0 && targetid < MAX_PLAYERS && IsPlayerConnected(targetid)) {
            PR_SendPacket(bs, targetid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
            for (new i = 0; i < MAX_PLAYERS; i++) {
                if (i != targetid && IsPlayerConnected(i)) {
                    if (_IsPlayerStreamedIn(targetid, i)) {
                        PR_SendPacket(bs, i, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
                    }
                }
            }
        } else {
            _PR_BroadcastPacket(bs);
        }
    } else if (IsPlayerConnected(playerid)) {
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

/**
 * Forces rebuild of the CJ ped mesh
 */
stock RebuildPlayerCJ(playerid, targetid) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_REBUILD
    );

    if (playerid == -1) {
        _PR_BroadcastPacket(bs);
    } else if (IsPlayerConnected(playerid)) {
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

/**
 * Sets a complete coordinated CJ outfit, updates server authoritative data,
 * and synchronizes with all streamed players (or specific recipient).
 */
stock SetPlayerCJFullOutfit(playerid, targetid, Float:fat, Float:muscle, const slots[], const textures[][], const models[][], count) {
    if (count > CJ_SLOT_COUNT) count = CJ_SLOT_COUNT;

    if (targetid >= 0 && targetid < MAX_PLAYERS) {
        ResetPlayerCJData(targetid);
        g_PlayerCJActive[targetid] = true;
        g_PlayerCJFat[targetid] = fat;
        g_PlayerCJMuscle[targetid] = muscle;
        for (new i = 0; i < count; i++) {
            new s = slots[i];
            if (s >= 0 && s < CJ_SLOT_COUNT) {
                format(g_PlayerCJTexture[targetid][s], 32, "%s", textures[i]);
                format(g_PlayerCJModel[targetid][s], 32, "%s", models[i]);
            }
        }
    }

    if (playerid == -1) {
        if (targetid >= 0 && targetid < MAX_PLAYERS && IsPlayerConnected(targetid)) {
            _SendRawCJOutfitPacket(targetid, targetid, fat, muscle, slots, textures, models, count);
            for (new i = 0; i < MAX_PLAYERS; i++) {
                if (i != targetid && IsPlayerConnected(i)) {
                    if (_IsPlayerStreamedIn(targetid, i)) {
                        _SendRawCJOutfitPacket(i, targetid, fat, muscle, slots, textures, models, count);
                    }
                }
            }
        } else {
            _SendRawCJOutfitPacket(-1, targetid, fat, muscle, slots, textures, models, count);
        }
    } else {
        _SendRawCJOutfitPacket(playerid, targetid, fat, muscle, slots, textures, models, count);
    }
    return 1;
}

/**
 * Sets a tattoo on CJ (tattoo_slot 0..8 relative, or 4..12 absolute)
 */
stock SetPlayerCJTattoo(playerid, targetid, tattoo_slot, const texture[], bool:rebuild = true) {
    new absSlot = tattoo_slot;
    if (absSlot <= 8) {
        absSlot += CJ_SLOT_TATTOO_LOWER_LEFT;
    }
    return SetPlayerCJClothes(playerid, targetid, absSlot, texture, "", rebuild);
}

/**
 * Clears cached CJ skin state on the client and resets server memory
 */
stock ClearPlayerCJCache(playerid, targetid) {
    if (targetid >= 0 && targetid < MAX_PLAYERS) {
        ResetPlayerCJData(targetid);
    }

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, targetid,
        PR_UINT8, CJ_ACTION_CLEAR_CACHE
    );

    if (playerid == -1) {
        _PR_BroadcastPacket(bs);
    } else if (IsPlayerConnected(playerid)) {
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    }
    BS_Delete(bs);
    return 1;
}

// Global broadcast convenience macros (visible to all players)
#define BroadcastPlayerCJClothes(%0,%1,%2,%3,%4)        SetPlayerCJClothes(-1, %0, %1, %2, %3, %4)
#define BroadcastRemovePlayerCJClothes(%0,%1,%2)        RemovePlayerCJClothes(-1, %0, %1, %2)
#define BroadcastPlayerCJBodyStats(%0,%1,%2,%3)         SetPlayerCJBodyStats(-1, %0, %1, %2, %3)
#define BroadcastResetPlayerCJClothes(%0,%1,%2)         ResetPlayerCJClothes(-1, %0, %1, %2)
#define BroadcastRebuildPlayerCJ(%0)                    RebuildPlayerCJ(-1, %0)
#define BroadcastPlayerCJFullOutfit(%0,%1,%2,%3,%4,%5,%6) SetPlayerCJFullOutfit(-1, %0, %1, %2, %3, %4, %5, %6)
#define BroadcastPlayerCJTattoo(%0,%1,%2,%3)            SetPlayerCJTattoo(-1, %0, %1, %2, %3)
#define BroadcastClearPlayerCJCache(%0)                 ClearPlayerCJCache(-1, %0)

/*
 * =============================================================================
 * ExtendedGame Gameplay & Environmental Controls (UG:MP Parity)
 * =============================================================================
 */

// Internal helpers for packet delivery
static stock _SendAircraftHeightLimitPacket(recipient, Float:limit) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, recipient,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT,
        PR_FLOAT, limit
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

static stock _SendJetpackHeightLimitPacket(recipient, Float:limit) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, recipient,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT,
        PR_FLOAT, limit
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

/**
 * Sets the aircraft ceiling height limit for a player (or -1 for global broadcast)
 */
stock SetAircraftHeightLimitForPlayer(playerid, Float:heightLimit) {
    if (playerid == -1) {
        g_GlobalAircraftHeightLimit = heightLimit;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            g_PlayerAircraftHeightLimit[i] = heightLimit;
            if (IsPlayerConnected(i)) {
                _SendAircraftHeightLimitPacket(i, heightLimit);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerAircraftHeightLimit[playerid] = heightLimit;

    if (IsPlayerConnected(playerid)) {
        _SendAircraftHeightLimitPacket(playerid, heightLimit);
    }
    return 1;
}

stock Float:GetPlayerAircraftHeightLimit(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 800.0;
    return g_PlayerAircraftHeightLimit[playerid];
}

/**
 * Sets the jetpack ceiling height limit for a player (or -1 for global broadcast)
 */
stock SetJetpackHeightLimitForPlayer(playerid, Float:heightLimit) {
    if (playerid == -1) {
        g_GlobalJetpackHeightLimit = heightLimit;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            g_PlayerJetpackHeightLimit[i] = heightLimit;
            if (IsPlayerConnected(i)) {
                _SendJetpackHeightLimitPacket(i, heightLimit);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerJetpackHeightLimit[playerid] = heightLimit;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackHeightLimitPacket(playerid, heightLimit);
    }
    return 1;
}

stock Float:GetPlayerJetpackHeightLimit(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 100.0;
    return g_PlayerJetpackHeightLimit[playerid];
}

static stock _SendJetpackOffsetPacket(recipient, Float:x, Float:y, Float:z, Float:angle, Float:axisX, Float:axisY, Float:axisZ) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, recipient,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET,
        PR_FLOAT, x,
        PR_FLOAT, y,
        PR_FLOAT, z,
        PR_FLOAT, angle,
        PR_FLOAT, axisX,
        PR_FLOAT, axisY,
        PR_FLOAT, axisZ
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

static stock _SendResetJetpackOffsetPacket(recipient) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, recipient,
        PR_UINT8, EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

/**
 * Sets the jetpack clump translation offset for a player (or -1 for global broadcast)
 */
stock SetJetpackOffsetForPlayer(playerid, Float:x, Float:y, Float:z) {
    if (playerid == -1) {
        g_GlobalJetpackOffsetX = x;
        g_GlobalJetpackOffsetY = y;
        g_GlobalJetpackOffsetZ = z;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            g_PlayerJetpackOffsetX[i] = x;
            g_PlayerJetpackOffsetY[i] = y;
            g_PlayerJetpackOffsetZ[i] = z;
            if (IsPlayerConnected(i)) {
                _SendJetpackOffsetPacket(i, x, y, z, g_PlayerJetpackRotAngle[i], g_PlayerJetpackRotAxisX[i], g_PlayerJetpackRotAxisY[i], g_PlayerJetpackRotAxisZ[i]);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerJetpackOffsetX[playerid] = x;
    g_PlayerJetpackOffsetY[playerid] = y;
    g_PlayerJetpackOffsetZ[playerid] = z;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackOffsetPacket(playerid, x, y, z, g_PlayerJetpackRotAngle[playerid], g_PlayerJetpackRotAxisX[playerid], g_PlayerJetpackRotAxisY[playerid], g_PlayerJetpackRotAxisZ[playerid]);
    }
    return 1;
}

stock GetPlayerJetpackOffset(playerid, &Float:x, &Float:y, &Float:z) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) {
        x = g_GlobalJetpackOffsetX;
        y = g_GlobalJetpackOffsetY;
        z = g_GlobalJetpackOffsetZ;
        return 0;
    }
    x = g_PlayerJetpackOffsetX[playerid];
    y = g_PlayerJetpackOffsetY[playerid];
    z = g_PlayerJetpackOffsetZ[playerid];
    return 1;
}

/**
 * Sets the jetpack clump rotation for a player (or -1 for global broadcast)
 */
stock SetJetpackRotationForPlayer(playerid, Float:angle, Float:axisX = 0.0, Float:axisY = 1.0, Float:axisZ = 0.0) {
    if (playerid == -1) {
        g_GlobalJetpackRotAngle = angle;
        g_GlobalJetpackRotAxisX = axisX;
        g_GlobalJetpackRotAxisY = axisY;
        g_GlobalJetpackRotAxisZ = axisZ;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            g_PlayerJetpackRotAngle[i] = angle;
            g_PlayerJetpackRotAxisX[i] = axisX;
            g_PlayerJetpackRotAxisY[i] = axisY;
            g_PlayerJetpackRotAxisZ[i] = axisZ;
            if (IsPlayerConnected(i)) {
                _SendJetpackOffsetPacket(i, g_PlayerJetpackOffsetX[i], g_PlayerJetpackOffsetY[i], g_PlayerJetpackOffsetZ[i], angle, axisX, axisY, axisZ);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerJetpackRotAngle[playerid] = angle;
    g_PlayerJetpackRotAxisX[playerid] = axisX;
    g_PlayerJetpackRotAxisY[playerid] = axisY;
    g_PlayerJetpackRotAxisZ[playerid] = axisZ;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackOffsetPacket(playerid, g_PlayerJetpackOffsetX[playerid], g_PlayerJetpackOffsetY[playerid], g_PlayerJetpackOffsetZ[playerid], angle, axisX, axisY, axisZ);
    }
    return 1;
}

stock GetPlayerJetpackRotation(playerid, &Float:angle, &Float:axisX, &Float:axisY, &Float:axisZ) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) {
        angle = g_GlobalJetpackRotAngle;
        axisX = g_GlobalJetpackRotAxisX;
        axisY = g_GlobalJetpackRotAxisY;
        axisZ = g_GlobalJetpackRotAxisZ;
        return 0;
    }
    angle = g_PlayerJetpackRotAngle[playerid];
    axisX = g_PlayerJetpackRotAxisX[playerid];
    axisY = g_PlayerJetpackRotAxisY[playerid];
    axisZ = g_PlayerJetpackRotAxisZ[playerid];
    return 1;
}

/**
 * Resets the jetpack clump placement and rotation to default
 */
stock ResetJetpackOffsetForPlayer(playerid) {
    if (playerid == -1) {
        g_GlobalJetpackOffsetX  = 0.10;
        g_GlobalJetpackOffsetY  = -0.38;
        g_GlobalJetpackOffsetZ  = 0.00;
        g_GlobalJetpackRotAngle = 90.0;
        g_GlobalJetpackRotAxisX = 0.0;
        g_GlobalJetpackRotAxisY = 1.0;
        g_GlobalJetpackRotAxisZ = 0.0;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            g_PlayerJetpackOffsetX[i]  = 0.10;
            g_PlayerJetpackOffsetY[i]  = -0.38;
            g_PlayerJetpackOffsetZ[i]  = 0.00;
            g_PlayerJetpackRotAngle[i] = 90.0;
            g_PlayerJetpackRotAxisX[i] = 0.0;
            g_PlayerJetpackRotAxisY[i] = 1.0;
            g_PlayerJetpackRotAxisZ[i] = 0.0;
            if (IsPlayerConnected(i)) {
                _SendResetJetpackOffsetPacket(i);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerJetpackOffsetX[playerid]  = 0.10;
    g_PlayerJetpackOffsetY[playerid]  = -0.38;
    g_PlayerJetpackOffsetZ[playerid]  = 0.00;
    g_PlayerJetpackRotAngle[playerid] = 90.0;
    g_PlayerJetpackRotAxisX[playerid] = 0.0;
    g_PlayerJetpackRotAxisY[playerid] = 1.0;
    g_PlayerJetpackRotAxisZ[playerid] = 0.0;

    if (IsPlayerConnected(playerid)) {
        _SendResetJetpackOffsetPacket(playerid);
    }
    return 1;
}

/**
 * Affects how a player can get knocked off bikes
 * knockState: BIKE_KNOCK_OFF_DEFAULT (0), BIKE_KNOCK_OFF_NEVER (1), BIKE_KNOCK_OFF_ALWAYS (2)
 */
static stock _ApplyKnockedOffBike(playerid, knockState) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerKnockOffBike[playerid] = knockState;

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF,
            PR_UINT8, knockState
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);

        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (i != playerid && IsPlayerConnected(i)) {
                if (_IsPlayerStreamedIn(playerid, i)) {
                    PR_SendPacket(bs, i, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
                }
            }
        }
        BS_Delete(bs);
    }
    return 1;
}

stock SetPlayerKnockedOffBikeState(playerid, knockState) {
    if (playerid == -1) {
        g_GlobalKnockOffBike = knockState;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyKnockedOffBike(i, knockState);
        }
        return 1;
    }
    return _ApplyKnockedOffBike(playerid, knockState);
}

stock GetPlayerKnockedOffBikeState(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return BIKE_KNOCK_OFF_DEFAULT;
    return g_PlayerKnockOffBike[playerid];
}

static stock _ApplyFireProof(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerFireProof[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_FIRE_PROOF,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);

        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (i != playerid && IsPlayerConnected(i)) {
                if (_IsPlayerStreamedIn(playerid, i)) {
                    PR_SendPacket(bs, i, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
                }
            }
        }
        BS_Delete(bs);
    }
    return 1;
}

stock SetPlayerFireProof(playerid, enable) {
    if (playerid == -1) {
        g_GlobalFireProof = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyFireProof(i, enable);
        }
        return 1;
    }
    return _ApplyFireProof(playerid, enable);
}

stock bool:IsPlayerFireProof(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return g_PlayerFireProof[playerid];
}

static stock _ApplyInfiniteSprint(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerInfiniteSprint[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock TogglePlayerInfiniteSprint(playerid, enable) {
    if (playerid == -1) {
        g_GlobalInfiniteSprint = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyInfiniteSprint(i, enable);
        }
        return 1;
    }
    return _ApplyInfiniteSprint(playerid, enable);
}

stock bool:IsPlayerInfiniteSprintEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return g_PlayerInfiniteSprint[playerid];
}

static stock _ApplySun(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerSunEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_SUN,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock TogglePlayerSun(playerid, enable) {
    if (playerid == -1) {
        g_GlobalSunEnabled = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplySun(i, enable);
        }
        return 1;
    }
    return _ApplySun(playerid, enable);
}

stock bool:IsPlayerSunEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerSunEnabled[playerid];
}

/**
 * Toggles the visibility of the moon globally for all players
 */
static stock _ApplyPlayerMoon(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerMoonEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_MOON,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ToggleMoon(enable) {
    g_GlobalMoonEnabled = (enable != 0);
    for (new i = 0; i < MAX_PLAYERS; i++) {
        _ApplyPlayerMoon(i, enable);
    }
    return 1;
}

stock TogglePlayerMoon(playerid, enable) {
    if (playerid == -1) {
        return ToggleMoon(enable);
    }
    return _ApplyPlayerMoon(playerid, enable);
}

stock bool:IsPlayerMoonEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerMoonEnabled[playerid];
}

static stock _ApplyPlayerStars(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerStarsEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_STARS,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ToggleStars(enable) {
    g_GlobalStarsEnabled = (enable != 0);
    for (new i = 0; i < MAX_PLAYERS; i++) {
        _ApplyPlayerStars(i, enable);
    }
    return 1;
}

stock TogglePlayerStars(playerid, enable) {
    if (playerid == -1) {
        return ToggleStars(enable);
    }
    return _ApplyPlayerStars(playerid, enable);
}

stock bool:IsPlayerStarsEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerStarsEnabled[playerid];
}

static stock _ApplyPlayerLowClouds(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerLowCloudsEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_LOW_CLOUDS,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ToggleLowClouds(enable) {
    g_GlobalLowCloudsEnabled = (enable != 0);
    for (new i = 0; i < MAX_PLAYERS; i++) {
        _ApplyPlayerLowClouds(i, enable);
    }
    return 1;
}

stock TogglePlayerLowClouds(playerid, enable) {
    if (playerid == -1) {
        return ToggleLowClouds(enable);
    }
    return _ApplyPlayerLowClouds(playerid, enable);
}

stock bool:IsPlayerLowCloudsEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerLowCloudsEnabled[playerid];
}

static stock _ApplyPlayerFluffyClouds(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerFluffyCloudsEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_FLUFFY_CLOUDS,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ToggleFluffyClouds(enable) {
    g_GlobalFluffyCloudsEnabled = (enable != 0);
    for (new i = 0; i < MAX_PLAYERS; i++) {
        _ApplyPlayerFluffyClouds(i, enable);
    }
    return 1;
}

stock TogglePlayerFluffyClouds(playerid, enable) {
    if (playerid == -1) {
        return ToggleFluffyClouds(enable);
    }
    return _ApplyPlayerFluffyClouds(playerid, enable);
}

stock bool:IsPlayerFluffyCloudsEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerFluffyCloudsEnabled[playerid];
}

static stock _ApplyPlayerRainbow(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerRainbowEnabled[playerid] = (enable != 0);

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_RAINBOW,
            PR_UINT8, enable ? 1 : 0
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ToggleRainbow(enable) {
    g_GlobalRainbowEnabled = (enable != 0);
    for (new i = 0; i < MAX_PLAYERS; i++) {
        _ApplyPlayerRainbow(i, enable);
    }
    return 1;
}

stock TogglePlayerRainbow(playerid, enable) {
    if (playerid == -1) {
        return ToggleRainbow(enable);
    }
    return _ApplyPlayerRainbow(playerid, enable);
}

stock bool:IsPlayerRainbowEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return g_PlayerRainbowEnabled[playerid];
}

/**
 * Sets world water vertices for a player (or all players with -1), like MTA's
 * world-water controls. Outside-world water is unchanged unless requested.
 */
static stock _ApplyPlayerWaterLevel(playerid, Float:level, bool:includeWorldNonSeaLevel, bool:includeWorldSeaLevel, bool:includeOutsideWorldLevel) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerWaterLevelSet[playerid] = true;
    g_PlayerWaterLevel[playerid] = level;
    g_PlayerWaterIncludeNonSea[playerid] = includeWorldNonSeaLevel;
    g_PlayerWaterIncludeSea[playerid] = includeWorldSeaLevel;
    g_PlayerWaterIncludeOutside[playerid] = includeOutsideWorldLevel;
    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_WATER_LEVEL,
            PR_FLOAT, level,
            PR_UINT8, (includeWorldNonSeaLevel ? 1 : 0) | (includeWorldSeaLevel ? 2 : 0) | (includeOutsideWorldLevel ? 4 : 0)
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock SetPlayerWaterLevel(playerid, Float:level, bool:includeWorldNonSeaLevel = true, bool:includeWorldSeaLevel = true, bool:includeOutsideWorldLevel = true) {
    if (level != level) return 0;
    if (playerid == -1) {
        g_GlobalWaterLevelSet = true;
        g_GlobalWaterLevel = level;
        g_GlobalWaterIncludeNonSea = includeWorldNonSeaLevel;
        g_GlobalWaterIncludeSea = includeWorldSeaLevel;
        g_GlobalWaterIncludeOutside = includeOutsideWorldLevel;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyPlayerWaterLevel(i, level, includeWorldNonSeaLevel, includeWorldSeaLevel, includeOutsideWorldLevel);
        }
        return 1;
    }
    return _ApplyPlayerWaterLevel(playerid, level, includeWorldNonSeaLevel, includeWorldSeaLevel, includeOutsideWorldLevel);
}

stock Float:GetPlayerWaterLevel(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0.0;
    return g_PlayerWaterLevel[playerid];
}

static stock _ApplyResetPlayerWaterLevel(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerWaterLevelSet[playerid] = false;
    g_PlayerWaterLevel[playerid] = 0.0;
    g_PlayerWaterIncludeNonSea[playerid] = true;
    g_PlayerWaterIncludeSea[playerid] = true;
    g_PlayerWaterIncludeOutside[playerid] = true;
    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_RESET_WATER_LEVEL
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock ResetPlayerWaterLevel(playerid) {
    if (playerid == -1) {
        g_GlobalWaterLevelSet = false;
        g_GlobalWaterLevel = 0.0;
        g_GlobalWaterIncludeNonSea = true;
        g_GlobalWaterIncludeSea = true;
        g_GlobalWaterIncludeOutside = true;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyResetPlayerWaterLevel(i);
        }
        return 1;
    }
    return _ApplyResetPlayerWaterLevel(playerid);
}

static stock _ApplyPlayerWaterWaveHeight(playerid, Float:height) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerWaterWaveHeight[playerid] = height;
    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_WATER_WAVE_HEIGHT,
            PR_FLOAT, height
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock SetPlayerWaterWaveHeight(playerid, Float:height) {
    if (!(height >= -1.0 && height <= 100.0)) return 0;
    if (playerid == -1) {
        g_GlobalWaterWaveHeight = height;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyPlayerWaterWaveHeight(i, height);
        }
        return 1;
    }
    return _ApplyPlayerWaterWaveHeight(playerid, height);
}

stock Float:GetPlayerWaterWaveHeight(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0.0;
    if (g_PlayerWaterWaveHeight[playerid] < 0.0) return 0.6;
    return g_PlayerWaterWaveHeight[playerid];
}

static stock _ApplyTogglePlayerHUD(playerid, hudComponents) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    g_PlayerHUDComponents[playerid] = hudComponents;

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_HUD_COMPONENTS,
            PR_UINT32, hudComponents
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock TogglePlayerHUDComponents(playerid, hudComponents) {
    if (playerid == -1) {
        g_GlobalHUDComponents = hudComponents;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyTogglePlayerHUD(i, hudComponents);
        }
        return 1;
    }
    return _ApplyTogglePlayerHUD(playerid, hudComponents);
}

stock GetPlayerHUDComponents(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return HUD_COMPONENT_ALL;
    return g_PlayerHUDComponents[playerid];
}

static stock _ApplyPlayerHUDColour(playerid, hudComponents, intColour) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;

    if (hudComponents & HUD_COMPONENT_AMMO)       g_PlayerHUDColourAmmo[playerid]       = intColour;
    if (hudComponents & HUD_COMPONENT_WEAPONICON) g_PlayerHUDColourWeaponIcon[playerid] = intColour;
    if (hudComponents & HUD_COMPONENT_HEALTH)     g_PlayerHUDColourHealth[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_OXYGEN)     g_PlayerHUDColourOxygen[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_ARMOUR)     g_PlayerHUDColourArmour[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_RADAR)      g_PlayerHUDColourRadar[playerid]      = intColour;
    if (hudComponents & HUD_COMPONENT_CROSSHAIR)  g_PlayerHUDColourCrosshair[playerid]  = intColour;
    if (hudComponents & HUD_COMPONENT_MONEY)      g_PlayerHUDColourMoney[playerid]      = intColour;
    if (hudComponents & HUD_COMPONENT_LASERDOT)   g_PlayerHUDColourLaserDot[playerid]   = intColour;

    if (IsPlayerConnected(playerid)) {
        new BitStream:bs = BS_New();
        BS_WriteValue(bs,
            PR_UINT8, EXTENDEDGAME_PACKET_ID,
            PR_UINT16, playerid,
            PR_UINT8, EXTENDEDGAME_ACTION_SET_HUD_COMPONENTS_COLOUR,
            PR_UINT32, hudComponents,
            PR_UINT32, intColour
        );
        PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
        BS_Delete(bs);
    }
    return 1;
}

stock SetPlayerHUDComponentsColour(playerid, hudComponents, intColour) {
    if (playerid == -1) {
        for (new component = 0; component < 9; component++) {
            if (hudComponents & (1 << component))
                g_GlobalHUDColours[component] = intColour;
        }
        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (IsPlayerConnected(i))
                _ApplyPlayerHUDColour(i, hudComponents, intColour);
        }
        return 1;
    }
    return _ApplyPlayerHUDColour(playerid, hudComponents, intColour);
}

stock GetPlayerHUDComponentColour(playerid, hudComponent) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    switch (hudComponent) {
        case HUD_COMPONENT_AMMO:       return g_PlayerHUDColourAmmo[playerid];
        case HUD_COMPONENT_WEAPONICON: return g_PlayerHUDColourWeaponIcon[playerid];
        case HUD_COMPONENT_HEALTH:     return g_PlayerHUDColourHealth[playerid];
        case HUD_COMPONENT_OXYGEN:     return g_PlayerHUDColourOxygen[playerid];
        case HUD_COMPONENT_ARMOUR:     return g_PlayerHUDColourArmour[playerid];
        case HUD_COMPONENT_RADAR:      return g_PlayerHUDColourRadar[playerid];
        case HUD_COMPONENT_CROSSHAIR:  return g_PlayerHUDColourCrosshair[playerid];
        case HUD_COMPONENT_MONEY:      return g_PlayerHUDColourMoney[playerid];
        case HUD_COMPONENT_LASERDOT:   return g_PlayerHUDColourLaserDot[playerid];
    }
    return 0;
}

/**
 * Sets rotation and optional continuous spin animation for a global TextDraw for a specific player.
 *
 * @param playerid The player ID
 * @param textdrawid The global TextDraw ID (0 .. 2047)
 * @param angle Rotation angle in degrees (positive = clockwise)
 * @param pivotX X coordinate of rotation pivot (-1.0 = auto center, 0.0..1.0 = normalized, >1.0 = 640x480 space)
 * @param pivotY Y coordinate of rotation pivot (-1.0 = auto center, 0.0..1.0 = normalized, >1.0 = 640x480 space)
 * @param spinSpeed Continuous rotation speed in degrees per second (0.0 = static angle)
 */
stock SetTextDrawRotationForPlayer(playerid, Text:textdrawid, Float:angle, Float:pivotX = -1.0, Float:pivotY = -1.0, Float:spinSpeed = 0.0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION,
        PR_UINT16, _:textdrawid,
        PR_UINT8, 0, // isPlayerTextDraw = 0
        PR_FLOAT, angle,
        PR_FLOAT, pivotX,
        PR_FLOAT, pivotY,
        PR_FLOAT, spinSpeed
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Sets rotation and optional continuous spin animation for a global TextDraw for all players.
 */
stock SetTextDrawRotation(Text:textdrawid, Float:angle, Float:pivotX = -1.0, Float:pivotY = -1.0, Float:spinSpeed = 0.0) {
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;
    if (!g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled]) {
        g_ActiveTextDrawRotationsCount++;
    }
    g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] = true;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotAngle] = angle;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotX] = pivotX;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotY] = pivotY;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed] = spinSpeed;

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            SetTextDrawRotationForPlayer(i, textdrawid, angle, pivotX, pivotY, spinSpeed);
        }
    }
    return 1;
}

/**
 * Resets rotation of a global TextDraw for a specific player.
 */
stock ResetTextDrawRotationForPlayer(playerid, Text:textdrawid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION,
        PR_UINT16, _:textdrawid,
        PR_UINT8, 0 // isPlayerTextDraw = 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Resets rotation of a global TextDraw for all players.
 */
stock ResetTextDrawRotation(Text:textdrawid) {
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;
    if (g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled]) {
        g_ActiveTextDrawRotationsCount--;
    }
    g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] = false;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotAngle] = 0.0;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotX] = -1.0;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotY] = -1.0;
    g_GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed] = 0.0;

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            ResetTextDrawRotationForPlayer(i, textdrawid);
        }
    }
    return 1;
}

/**
 * Sets rotation and optional continuous spin animation for a Player TextDraw.
 */
stock SetPlayerTextDrawRotation(playerid, PlayerText:textdrawid, Float:angle, Float:pivotX = -1.0, Float:pivotY = -1.0, Float:spinSpeed = 0.0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return 0;

    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] = true;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle] = angle;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX] = pivotX;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY] = pivotY;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed] = spinSpeed;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION,
        PR_UINT16, _:textdrawid,
        PR_UINT8, 1, // isPlayerTextDraw = 1
        PR_FLOAT, angle,
        PR_FLOAT, pivotX,
        PR_FLOAT, pivotY,
        PR_FLOAT, spinSpeed
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Resets rotation of a Player TextDraw.
 */
stock ResetPlayerTextDrawRotation(playerid, PlayerText:textdrawid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return 0;

    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] = false;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle] = 0.0;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX] = -1.0;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY] = -1.0;
    g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed] = 0.0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION,
        PR_UINT16, _:textdrawid,
        PR_UINT8, 1 // isPlayerTextDraw = 1
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Returns true if a global TextDraw has active rotation.
 */
stock bool:IsTextDrawRotated(Text:textdrawid) {
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return false;
    return g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled];
}

/**
 * Retrieves the global TextDraw rotation parameters.
 */
stock GetTextDrawRotation(Text:textdrawid, &Float:angle, &Float:pivotX, &Float:pivotY, &Float:spinSpeed) {
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;
    angle = g_GlobalTextDrawRotation[_:textdrawid][tdRotAngle];
    pivotX = g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotX];
    pivotY = g_GlobalTextDrawRotation[_:textdrawid][tdRotPivotY];
    spinSpeed = g_GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed];
    return g_GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] ? 1 : 0;
}

/**
 * Returns true if a Player TextDraw has active rotation.
 */
stock bool:IsPlayerTextDrawRotated(playerid, PlayerText:textdrawid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return false;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return false;
    return g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled];
}

/**
 * Retrieves the Player TextDraw rotation parameters.
 */
stock GetPlayerTextDrawRotation(playerid, PlayerText:textdrawid, &Float:angle, &Float:pivotX, &Float:pivotY, &Float:spinSpeed) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return 0;
    angle = g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle];
    pivotX = g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX];
    pivotY = g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY];
    spinSpeed = g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed];
    return g_PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] ? 1 : 0;
}

/*
 * =============================================================================
 * Custom Water Zones Functions
 * =============================================================================
 */

/**
 * Creates or updates a custom water zone for a specific player.
 */
stock CreateWaterZoneForPlayer(playerid, zoneid, Float:minX, Float:minY, Float:maxX, Float:maxY, Float:level, Float:bottomZ = -100.0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_CREATE_WATER_ZONE,
        PR_UINT16, zoneid,
        PR_FLOAT, minX,
        PR_FLOAT, minY,
        PR_FLOAT, maxX,
        PR_FLOAT, maxY,
        PR_FLOAT, level,
        PR_FLOAT, bottomZ
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Creates a global custom water zone for all players.
 * Returns the created zone ID (0..MAX_WATER_ZONES-1), or INVALID_WATER_ZONE on failure.
 */
stock CreateWaterZone(Float:minX, Float:minY, Float:maxX, Float:maxY, Float:level, Float:bottomZ = -100.0) {
    new zoneid = INVALID_WATER_ZONE;
    for (new i = 0; i < MAX_WATER_ZONES; i++) {
        if (!g_GlobalWaterZones[i][wzActive]) {
            zoneid = i;
            break;
        }
    }
    if (zoneid == INVALID_WATER_ZONE) return INVALID_WATER_ZONE;

    g_ActiveWaterZonesCount++;
    g_GlobalWaterZones[zoneid][wzActive] = true;
    g_GlobalWaterZones[zoneid][wzMinX] = (minX < maxX) ? minX : maxX;
    g_GlobalWaterZones[zoneid][wzMaxX] = (minX > maxX) ? minX : maxX;
    g_GlobalWaterZones[zoneid][wzMinY] = (minY < maxY) ? minY : maxY;
    g_GlobalWaterZones[zoneid][wzMaxY] = (minY > maxY) ? minY : maxY;
    g_GlobalWaterZones[zoneid][wzLevel] = level;
    g_GlobalWaterZones[zoneid][wzBottomZ] = (bottomZ < level) ? bottomZ : (level - 5.0);

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            CreateWaterZoneForPlayer(i, zoneid, minX, minY, maxX, maxY, level, bottomZ);
        }
    }
    return zoneid;
}

/**
 * Changes the level of a custom water zone for a specific player.
 */
stock SetWaterZoneLevelForPlayer(playerid, zoneid, Float:level) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL,
        PR_UINT16, zoneid,
        PR_FLOAT, level
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Changes the level of a global custom water zone for all players.
 */
stock SetWaterZoneLevel(zoneid, Float:level) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !g_GlobalWaterZones[zoneid][wzActive]) return 0;
    g_GlobalWaterZones[zoneid][wzLevel] = level;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            SetWaterZoneLevelForPlayer(i, zoneid, level);
        }
    }
    return 1;
}

/**
 * Resizes / moves a custom water zone for a specific player.
 */
stock SetWaterZoneCoordsForPlayer(playerid, zoneid, Float:minX, Float:minY, Float:maxX, Float:maxY, Float:level, Float:bottomZ = -100.0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS,
        PR_UINT16, zoneid,
        PR_FLOAT, minX,
        PR_FLOAT, minY,
        PR_FLOAT, maxX,
        PR_FLOAT, maxY,
        PR_FLOAT, level,
        PR_FLOAT, bottomZ
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Resizes / moves a global custom water zone for all players.
 */
stock SetWaterZoneCoords(zoneid, Float:minX, Float:minY, Float:maxX, Float:maxY, Float:level, Float:bottomZ = -100.0) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !g_GlobalWaterZones[zoneid][wzActive]) return 0;
    g_GlobalWaterZones[zoneid][wzMinX] = (minX < maxX) ? minX : maxX;
    g_GlobalWaterZones[zoneid][wzMaxX] = (minX > maxX) ? minX : maxX;
    g_GlobalWaterZones[zoneid][wzMinY] = (minY < maxY) ? minY : maxY;
    g_GlobalWaterZones[zoneid][wzMaxY] = (minY > maxY) ? minY : maxY;
    g_GlobalWaterZones[zoneid][wzLevel] = level;
    g_GlobalWaterZones[zoneid][wzBottomZ] = (bottomZ < level) ? bottomZ : (level - 5.0);

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            SetWaterZoneCoordsForPlayer(i, zoneid, minX, minY, maxX, maxY, level, bottomZ);
        }
    }
    return 1;
}

/**
 * Destroys a custom water zone for a specific player.
 */
stock DestroyWaterZoneForPlayer(playerid, zoneid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES) return 0;

    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE,
        PR_UINT16, zoneid
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

/**
 * Destroys a global custom water zone for all players.
 */
stock DestroyWaterZone(zoneid) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !g_GlobalWaterZones[zoneid][wzActive]) return 0;
    g_ActiveWaterZonesCount--;
    g_GlobalWaterZones[zoneid][wzActive] = false;
    g_GlobalWaterZones[zoneid][wzMinX] = 0.0;
    g_GlobalWaterZones[zoneid][wzMinY] = 0.0;
    g_GlobalWaterZones[zoneid][wzMaxX] = 0.0;
    g_GlobalWaterZones[zoneid][wzMaxY] = 0.0;
    g_GlobalWaterZones[zoneid][wzLevel] = 0.0;
    g_GlobalWaterZones[zoneid][wzBottomZ] = -100.0;

    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            DestroyWaterZoneForPlayer(i, zoneid);
        }
    }
    return 1;
}

/**
 * Returns true if a water zone ID is active.
 */
stock bool:IsValidWaterZone(zoneid) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES) return false;
    return g_GlobalWaterZones[zoneid][wzActive];
}

/**
 * Gets the water level of a custom water zone.
 */
stock GetWaterZoneLevel(zoneid, &Float:level) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !g_GlobalWaterZones[zoneid][wzActive]) return 0;
    level = g_GlobalWaterZones[zoneid][wzLevel];
    return 1;
}

/**
 * Gets the coordinates and bounds of a custom water zone.
 */
stock GetWaterZoneCoords(zoneid, &Float:minX, &Float:minY, &Float:maxX, &Float:maxY, &Float:level, &Float:bottomZ) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !g_GlobalWaterZones[zoneid][wzActive]) return 0;
    minX = g_GlobalWaterZones[zoneid][wzMinX];
    minY = g_GlobalWaterZones[zoneid][wzMinY];
    maxX = g_GlobalWaterZones[zoneid][wzMaxX];
    maxY = g_GlobalWaterZones[zoneid][wzMaxY];
    level = g_GlobalWaterZones[zoneid][wzLevel];
    bottomZ = g_GlobalWaterZones[zoneid][wzBottomZ];
    return 1;
}

/*
 * =============================================================================
 * Automatic Server-Side Hooks (ALS Chaining)
 * =============================================================================
 */

// 1. Hook OnPlayerStreamIn: Auto-sync outfit and physics attributes when playerid streams in for forplayerid
public OnPlayerStreamIn(playerid, forplayerid) {
    if (playerid >= 0 && playerid < MAX_PLAYERS && forplayerid >= 0 && forplayerid < MAX_PLAYERS &&
        IsPlayerConnected(playerid) && IsPlayerConnected(forplayerid)) {
        _SyncExtGameActorState(playerid, forplayerid);
    }
    #if defined EXTG_OnPlayerStreamIn
        return EXTG_OnPlayerStreamIn(playerid, forplayerid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerStreamIn
    #undef OnPlayerStreamIn
#else
    #define _ALS_OnPlayerStreamIn
#endif
#define OnPlayerStreamIn EXTG_OnPlayerStreamIn
#if defined EXTG_OnPlayerStreamIn
    forward EXTG_OnPlayerStreamIn(playerid, forplayerid);
#endif

// 2. Hook OnPlayerSpawn: Auto-sync when playerid spawns with Model 0
public OnPlayerSpawn(playerid) {
    if (!g_PlayerHasPlugin[playerid]) {
        // Re-ping in case client was slow to load or connect
        CheckPlayerExtendedGame(playerid, 0);
    } else {
        _SyncPlayerExtendedGameSettings(playerid);
    }

    if (IsPlayerConnected(playerid)) {
        _SendPlayerBikeState(playerid, playerid);
        _SendPlayerFireProofState(playerid, playerid);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (i != playerid && IsPlayerConnected(i) && _IsPlayerStreamedIn(playerid, i)) {
                _SendPlayerBikeState(i, playerid);
                _SendPlayerFireProofState(i, playerid);
            }
        }
    }

    if (GetPlayerSkin(playerid) == 0) {
        if (g_PlayerCJActive[playerid]) {
            SyncPlayerCJ(playerid);
        } else {
            // Apply singleplayer default clothes
            ResetPlayerCJClothes(-1, playerid, true, true);
        }
    }
    #if defined EXTG_OnPlayerSpawn
        return EXTG_OnPlayerSpawn(playerid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerSpawn
    #undef OnPlayerSpawn
#else
    #define _ALS_OnPlayerSpawn
#endif
#define OnPlayerSpawn EXTG_OnPlayerSpawn
#if defined EXTG_OnPlayerSpawn
    forward EXTG_OnPlayerSpawn(playerid);
#endif

// 3. Hook OnPlayerDisconnect: Purge server state & notify clients
public OnPlayerDisconnect(playerid, reason) {
    ResetPlayerCJData(playerid);
    g_PlayerHasPlugin[playerid] = false;
    g_PlayerPluginVersion[playerid] = 0;
    g_PlayerPluginChecked[playerid] = false;

    g_PlayerAircraftHeightLimit[playerid] = g_GlobalAircraftHeightLimit;
    g_PlayerJetpackHeightLimit[playerid]  = g_GlobalJetpackHeightLimit;
    g_PlayerJetpackOffsetX[playerid]      = g_GlobalJetpackOffsetX;
    g_PlayerJetpackOffsetY[playerid]      = g_GlobalJetpackOffsetY;
    g_PlayerJetpackOffsetZ[playerid]      = g_GlobalJetpackOffsetZ;
    g_PlayerJetpackRotAngle[playerid]     = g_GlobalJetpackRotAngle;
    g_PlayerJetpackRotAxisX[playerid]     = g_GlobalJetpackRotAxisX;
    g_PlayerJetpackRotAxisY[playerid]     = g_GlobalJetpackRotAxisY;
    g_PlayerJetpackRotAxisZ[playerid]     = g_GlobalJetpackRotAxisZ;
    g_PlayerKnockOffBike[playerid]        = g_GlobalKnockOffBike;
    g_PlayerFireProof[playerid]           = g_GlobalFireProof;
    g_PlayerInfiniteSprint[playerid]      = g_GlobalInfiniteSprint;
    g_PlayerSunEnabled[playerid]          = g_GlobalSunEnabled;

    g_PlayerMoonEnabled[playerid]         = g_GlobalMoonEnabled;
    g_PlayerStarsEnabled[playerid]        = g_GlobalStarsEnabled;
    g_PlayerLowCloudsEnabled[playerid]    = g_GlobalLowCloudsEnabled;
    g_PlayerFluffyCloudsEnabled[playerid] = g_GlobalFluffyCloudsEnabled;
    g_PlayerRainbowEnabled[playerid]      = g_GlobalRainbowEnabled;
    g_PlayerWaterLevelSet[playerid]       = g_GlobalWaterLevelSet;
    g_PlayerWaterLevel[playerid]          = g_GlobalWaterLevel;
    g_PlayerWaterIncludeNonSea[playerid]  = g_GlobalWaterIncludeNonSea;
    g_PlayerWaterIncludeSea[playerid]     = g_GlobalWaterIncludeSea;
    g_PlayerWaterIncludeOutside[playerid] = g_GlobalWaterIncludeOutside;
    g_PlayerWaterWaveHeight[playerid]     = g_GlobalWaterWaveHeight;
    g_PlayerHUDComponents[playerid]       = g_GlobalHUDComponents;
    g_PlayerHUDColourAmmo[playerid]       = g_GlobalHUDColours[0];
    g_PlayerHUDColourWeaponIcon[playerid] = g_GlobalHUDColours[1];
    g_PlayerHUDColourHealth[playerid]      = g_GlobalHUDColours[2];
    g_PlayerHUDColourOxygen[playerid]      = g_GlobalHUDColours[3];
    g_PlayerHUDColourArmour[playerid]      = g_GlobalHUDColours[4];
    g_PlayerHUDColourRadar[playerid]       = g_GlobalHUDColours[5];
    g_PlayerHUDColourCrosshair[playerid]   = g_GlobalHUDColours[6];
    g_PlayerHUDColourMoney[playerid]       = g_GlobalHUDColours[7];
    g_PlayerHUDColourLaserDot[playerid]    = g_GlobalHUDColours[8];

    BroadcastClearPlayerCJCache(playerid);

    for (new _ptd = 0; _ptd < MAX_PLAYER_TEXT_DRAWS; _ptd++) {
        g_PlayerTextDrawRotation[playerid][_ptd][tdRotEnabled] = false;
        g_PlayerTextDrawRotation[playerid][_ptd][tdRotAngle] = 0.0;
        g_PlayerTextDrawRotation[playerid][_ptd][tdRotPivotX] = -1.0;
        g_PlayerTextDrawRotation[playerid][_ptd][tdRotPivotY] = -1.0;
        g_PlayerTextDrawRotation[playerid][_ptd][tdRotSpinSpeed] = 0.0;
    }
    #if defined EXTG_OnPlayerDisconnect
        return EXTG_OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect EXTG_OnPlayerDisconnect
#if defined EXTG_OnPlayerDisconnect
    forward EXTG_OnPlayerDisconnect(playerid, reason);
#endif

// 4. Hook OnIncomingPacket: Respond to client-side CJ_ACTION_REQUEST_SYNC fallback & HANDSHAKE_PONG
public OnIncomingPacket(playerid, packetid, BitStream:bs) {
    if (packetid == EXTENDEDGAME_PACKET_ID) {
        BS_SetReadOffset(bs, 8); // Ensure offset is at bit 8 (byte 1, payload after packetid)
        new targetid, actionid;
        BS_ReadValue(bs,
            PR_UINT16, targetid,
            PR_UINT8, actionid
        );
        if (actionid == CJ_ACTION_REQUEST_SYNC) {
            if (targetid >= 0 && targetid < MAX_PLAYERS && IsPlayerConnected(targetid)) {
                SyncPlayerCJForPlayer(targetid, playerid);
            }
            return 0; // Handled, discard packet
        } else if (actionid == CJ_ACTION_HANDSHAKE_PONG) {
            new magic, version, challenge;
            BS_ReadValue(bs,
                PR_UINT32, magic,
                PR_UINT16, version,
                PR_UINT32, challenge
            );
            if (magic == EXTENDEDGAME_HANDSHAKE_MAGIC) {
                new bool:wasInstalled = g_PlayerHasPlugin[playerid];
                g_PlayerHasPlugin[playerid] = true;
                g_PlayerPluginVersion[playerid] = version;
                g_PlayerPluginChecked[playerid] = true;

                // Send immediate Handshake ACK back to the client
                new BitStream:ack = BS_New();
                BS_WriteValue(ack,
                    PR_UINT8, EXTENDEDGAME_PACKET_ID,
                    PR_UINT16, playerid,
                    PR_UINT8, CJ_ACTION_HANDSHAKE_ACK,
                    PR_UINT32, challenge
                );
                PR_SendPacket(ack, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
                BS_Delete(ack);

                if (!wasInstalled) {
                    printf("[ExtendedGame] Player %d confirmed BrownTurbo-ExtendedGame ASI (version 0x%04x).", playerid, version);
                    new funcName[32] = "_DeferredSyncPlayerExtendedGame";
                    SetTimerEx(funcName, 50, false, "i", playerid);

                    #if defined OnPlayerExtendedGameCheck
                        OnPlayerExtendedGameCheck(playerid, true, version);
                    #endif
                }
            }
            return 0; // Handled, discard packet
        }
    }
    #if defined EXTG_OnIncomingPacket
        return EXTG_OnIncomingPacket(playerid, packetid, BitStream:bs);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnIncomingPacket
    #undef OnIncomingPacket
#else
    #define _ALS_OnIncomingPacket
#endif
#define OnIncomingPacket EXTG_OnIncomingPacket
#if defined EXTG_OnIncomingPacket
    forward EXTG_OnIncomingPacket(playerid, packetid, BitStream:bs);
#endif

// Deferred synchronization callback executed 50ms after handshake to keep OnIncomingPacket lightweight
forward _DeferredSyncPlayerExtendedGame(playerid);
public _DeferredSyncPlayerExtendedGame(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid) || !g_PlayerHasPlugin[playerid])
        return 0;

    _SyncPlayerExtendedGameSettings(playerid);
    _SyncExtGameActorState(playerid, playerid);
    for (new _targetid = 0; _targetid < MAX_PLAYERS; _targetid++) {
        if (_targetid == playerid || !_IsPlayerStreamedIn(_targetid, playerid))
            continue;
        _SyncExtGameActorState(_targetid, playerid);
    }
    return 1;
}

// 5. Hook OnPlayerConnect: Initialize player state and start presence handshake
forward EXTG_OnPlayerPluginTimeout(playerid);
public EXTG_OnPlayerPluginTimeout(playerid) {
    if (IsPlayerConnected(playerid) && !g_PlayerPluginChecked[playerid]) {
        g_PlayerPluginChecked[playerid] = true;
        g_PlayerHasPlugin[playerid] = false;
        #if defined OnPlayerExtendedGameCheck
            OnPlayerExtendedGameCheck(playerid, false, 0);
        #endif
    }
    return 1;
}

static stock _InitPlayerExtendedGame(playerid) {
    ResetPlayerCJData(playerid);
    g_PlayerHasPlugin[playerid] = false;
    g_PlayerPluginVersion[playerid] = 0;
    g_PlayerPluginChecked[playerid] = false;

    g_PlayerAircraftHeightLimit[playerid] = g_GlobalAircraftHeightLimit;
    g_PlayerJetpackHeightLimit[playerid]  = g_GlobalJetpackHeightLimit;
    g_PlayerJetpackOffsetX[playerid]      = g_GlobalJetpackOffsetX;
    g_PlayerJetpackOffsetY[playerid]      = g_GlobalJetpackOffsetY;
    g_PlayerJetpackOffsetZ[playerid]      = g_GlobalJetpackOffsetZ;
    g_PlayerJetpackRotAngle[playerid]     = g_GlobalJetpackRotAngle;
    g_PlayerJetpackRotAxisX[playerid]     = g_GlobalJetpackRotAxisX;
    g_PlayerJetpackRotAxisY[playerid]     = g_GlobalJetpackRotAxisY;
    g_PlayerJetpackRotAxisZ[playerid]     = g_GlobalJetpackRotAxisZ;
    g_PlayerKnockOffBike[playerid]        = g_GlobalKnockOffBike;
    g_PlayerFireProof[playerid]           = g_GlobalFireProof;
    g_PlayerInfiniteSprint[playerid]      = g_GlobalInfiniteSprint;
    g_PlayerSunEnabled[playerid]          = g_GlobalSunEnabled;

    g_PlayerMoonEnabled[playerid]         = g_GlobalMoonEnabled;
    g_PlayerStarsEnabled[playerid]        = g_GlobalStarsEnabled;
    g_PlayerLowCloudsEnabled[playerid]    = g_GlobalLowCloudsEnabled;
    g_PlayerFluffyCloudsEnabled[playerid] = g_GlobalFluffyCloudsEnabled;
    g_PlayerRainbowEnabled[playerid]      = g_GlobalRainbowEnabled;
    g_PlayerWaterLevelSet[playerid]       = g_GlobalWaterLevelSet;
    g_PlayerWaterLevel[playerid]          = g_GlobalWaterLevel;
    g_PlayerWaterIncludeNonSea[playerid]  = g_GlobalWaterIncludeNonSea;
    g_PlayerWaterIncludeSea[playerid]     = g_GlobalWaterIncludeSea;
    g_PlayerWaterIncludeOutside[playerid] = g_GlobalWaterIncludeOutside;
    g_PlayerWaterWaveHeight[playerid]     = g_GlobalWaterWaveHeight;
    g_PlayerHUDComponents[playerid]       = g_GlobalHUDComponents;
    g_PlayerHUDColourAmmo[playerid]       = g_GlobalHUDColours[0];
    g_PlayerHUDColourWeaponIcon[playerid] = g_GlobalHUDColours[1];
    g_PlayerHUDColourHealth[playerid]     = g_GlobalHUDColours[2];
    g_PlayerHUDColourOxygen[playerid]      = g_GlobalHUDColours[3];
    g_PlayerHUDColourArmour[playerid]      = g_GlobalHUDColours[4];
    g_PlayerHUDColourRadar[playerid]       = g_GlobalHUDColours[5];
    g_PlayerHUDColourCrosshair[playerid]   = g_GlobalHUDColours[6];
    g_PlayerHUDColourMoney[playerid]       = g_GlobalHUDColours[7];
    g_PlayerHUDColourLaserDot[playerid]    = g_GlobalHUDColours[8];

    // Send handshake ping to detect ASI plugin presence
    CheckPlayerExtendedGame(playerid, 0);
    new cbTimeout[] = "EXTG_OnPlayerPluginTimeout";
    new fmtI[] = "i";
    SetTimerEx(cbTimeout, 6000, false, fmtI, playerid);
}

public OnPlayerConnect(playerid) {
    _InitPlayerExtendedGame(playerid);

    #if defined EXTG_OnPlayerConnect
        return EXTG_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}
#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect EXTG_OnPlayerConnect
#if defined EXTG_OnPlayerConnect
    forward EXTG_OnPlayerConnect(playerid);
#endif

// 6. Hook OnFilterScriptInit: Ensure Pawn.RakNet registers this AMX and check existing connected players
public OnFilterScriptInit() {
    PR_Init();
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            _InitPlayerExtendedGame(i);
        }
    }
    #if defined EXTG_OnFilterScriptInit
        return EXTG_OnFilterScriptInit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnFilterScriptInit
    #undef OnFilterScriptInit
#else
    #define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit EXTG_OnFilterScriptInit
#if defined EXTG_OnFilterScriptInit
    forward EXTG_OnFilterScriptInit();
#endif

// 7. Hook OnGameModeInit: Ensure Pawn.RakNet registers this AMX and check existing connected players
public OnGameModeInit() {
    PR_Init();
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            _InitPlayerExtendedGame(i);
        }
    }
    #if defined EXTG_OnGameModeInit
        return EXTG_OnGameModeInit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit EXTG_OnGameModeInit
#if defined EXTG_OnGameModeInit
    forward EXTG_OnGameModeInit();
#endif

// 8. Hook TextDrawDestroy: Auto-reset rotation state when textdraw is destroyed
stock EXTG_TextDrawDestroy(Text:text) {
    if (_:text >= 0 && _:text < MAX_TEXT_DRAWS) {
        ResetTextDrawRotation(text);
    }
    return TextDrawDestroy(text);
}
#if defined _ALS_TextDrawDestroy
    #undef TextDrawDestroy
#else
    #define _ALS_TextDrawDestroy
#endif
#define TextDrawDestroy EXTG_TextDrawDestroy

// 9. Hook PlayerTextDrawDestroy: Auto-reset rotation state when player textdraw is destroyed
stock EXTG_PlayerTextDrawDestroy(playerid, PlayerText:text) {
    if (_:text >= 0 && _:text < MAX_PLAYER_TEXT_DRAWS) {
        ResetPlayerTextDrawRotation(playerid, text);
    }
    return PlayerTextDrawDestroy(playerid, text);
}
#if defined _ALS_PlayerTextDrawDestroy
    #undef PlayerTextDrawDestroy
#else
    #define _ALS_PlayerTextDrawDestroy
#endif
#define PlayerTextDrawDestroy EXTG_PlayerTextDrawDestroy

// 10. Hook TextDrawShowForPlayer: Auto-sync rotation when textdraw is shown
stock EXTG_TextDrawShowForPlayer(playerid, Text:text) {
    new ret = TextDrawShowForPlayer(playerid, text);
    if (_:text >= 0 && _:text < MAX_TEXT_DRAWS && g_GlobalTextDrawRotation[_:text][tdRotEnabled]) {
        SetTextDrawRotationForPlayer(playerid, text,
            g_GlobalTextDrawRotation[_:text][tdRotAngle],
            g_GlobalTextDrawRotation[_:text][tdRotPivotX],
            g_GlobalTextDrawRotation[_:text][tdRotPivotY],
            g_GlobalTextDrawRotation[_:text][tdRotSpinSpeed]
        );
    }
    return ret;
}
#if defined _ALS_TextDrawShowForPlayer
    #undef TextDrawShowForPlayer
#else
    #define _ALS_TextDrawShowForPlayer
#endif
#define TextDrawShowForPlayer EXTG_TextDrawShowForPlayer

// 11. Hook TextDrawShowForAll: Auto-sync rotation when textdraw is shown for all
stock EXTG_TextDrawShowForAll(Text:text) {
    new ret = TextDrawShowForAll(text);
    if (_:text >= 0 && _:text < MAX_TEXT_DRAWS && g_GlobalTextDrawRotation[_:text][tdRotEnabled]) {
        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (IsPlayerConnected(i)) {
                SetTextDrawRotationForPlayer(i, text,
                    g_GlobalTextDrawRotation[_:text][tdRotAngle],
                    g_GlobalTextDrawRotation[_:text][tdRotPivotX],
                    g_GlobalTextDrawRotation[_:text][tdRotPivotY],
                    g_GlobalTextDrawRotation[_:text][tdRotSpinSpeed]
                );
            }
        }
    }
    return ret;
}
#if defined _ALS_TextDrawShowForAll
    #undef TextDrawShowForAll
#else
    #define _ALS_TextDrawShowForAll
#endif
#define TextDrawShowForAll EXTG_TextDrawShowForAll

// 12. Hook PlayerTextDrawShow: Auto-sync rotation when player textdraw is shown
stock EXTG_PlayerTextDrawShow(playerid, PlayerText:text) {
    new ret = PlayerTextDrawShow(playerid, text);
    if (playerid >= 0 && playerid < MAX_PLAYERS && _:text >= 0 && _:text < MAX_PLAYER_TEXT_DRAWS) {
        if (g_PlayerTextDrawRotation[playerid][_:text][tdRotEnabled]) {
            SetPlayerTextDrawRotation(playerid, text,
                g_PlayerTextDrawRotation[playerid][_:text][tdRotAngle],
                g_PlayerTextDrawRotation[playerid][_:text][tdRotPivotX],
                g_PlayerTextDrawRotation[playerid][_:text][tdRotPivotY],
                g_PlayerTextDrawRotation[playerid][_:text][tdRotSpinSpeed]
            );
        }
    }
    return ret;
}
#if defined _ALS_PlayerTextDrawShow
    #undef PlayerTextDrawShow
#else
    #define _ALS_PlayerTextDrawShow
#endif
#define PlayerTextDrawShow EXTG_PlayerTextDrawShow
