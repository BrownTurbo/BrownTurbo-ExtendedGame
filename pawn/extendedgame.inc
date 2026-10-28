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

#define EXTGAME:: extg_

// Packet Identification
#define EXTENDEDGAME_PACKET_ID          (252)
#define EXTENDEDGAME_TARGET_LOCAL_PLAYER (0xFFFF)

// Versioning & Protocol Magic
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

// ExtendedGame Vision Actions
#define EXTENDEDGAME_ACTION_TOGGLE_NIGHT_VISION       (46)
#define EXTENDEDGAME_ACTION_TOGGLE_THERMAL_VISION     (47)

// ExtendedGame Weapon Reload Actions
#define EXTENDEDGAME_ACTION_SET_NO_RELOAD             (48)
#define EXTENDEDGAME_ACTION_TOGGLE_SWITCH_RELOAD      (49)
#define EXTENDEDGAME_ACTION_TOGGLE_QUICK_RELOAD       (50)

// ExtendedGame Physics & Movement Actions
#define EXTENDEDGAME_ACTION_TOGGLE_INFINITE_OXYGEN    (51)
#define EXTENDEDGAME_ACTION_SET_PLAYER_PED_ANIMS      (52)
#define EXTENDEDGAME_ACTION_TOGGLE_CURSOR             (53)

// ExtendedGame Plane Radar Altimeter Color (Hexadecimal RGBA)
#define EXTENDEDGAME_ACTION_SET_PLANE_RADAR_COLOR     (54)

// ExtendedGame Pause Menu Event (Client -> Server)
#define EXTENDEDGAME_ACTION_PAUSE_MENU_EVENT          (55)

// ExtendedGame Sun Color Actions (Hexadecimal RGBA)
#define EXTENDEDGAME_ACTION_SET_SUN_COLOR             (56)
#define EXTENDEDGAME_ACTION_RESET_SUN_COLOR           (57)

// ExtendedGame Moon Color Actions (Hexadecimal RGBA)
#define EXTENDEDGAME_ACTION_SET_MOON_COLOR            (58)
#define EXTENDEDGAME_ACTION_RESET_MOON_COLOR          (59)

// ExtendedGame Vehicle Speed Blur Motion Action
#define EXTENDEDGAME_ACTION_TOGGLE_VEHICLE_BLUR       (60)

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
static stock EXTGAME::GlobalWaterZones[MAX_WATER_ZONES][E_EXTG_WATER_ZONE];
static stock EXTGAME::ActiveWaterZonesCount = 0;

static stock bool:EXTGAME::GlobalWaterLevelSet;
static stock Float:EXTGAME::GlobalWaterLevel;
static stock bool:EXTGAME::GlobalWaterIncludeNonSea = true;
static stock bool:EXTGAME::GlobalWaterIncludeSea = true;
static stock bool:EXTGAME::GlobalWaterIncludeOutside = true;
static stock Float:EXTGAME::GlobalWaterWaveHeight;

enum E_EXTG_TD_ROTATION {
    bool:tdRotEnabled,
    Float:tdRotAngle,
    Float:tdRotPivotX,
    Float:tdRotPivotY,
    Float:tdRotSpinSpeed
};
static stock EXTGAME::GlobalTextDrawRotation[MAX_TEXT_DRAWS][E_EXTG_TD_ROTATION];
static stock EXTGAME::ActiveTextDrawRotationsCount = 0;
static stock EXTGAME::PlayerTextDrawRotation[MAX_PLAYERS][MAX_PLAYER_TEXT_DRAWS][E_EXTG_TD_ROTATION];

// HUD Component Defines (UG:MP Parity & ASCM Extended)
#define HUD_COMPONENT_AMMO                  1
#define HUD_COMPONENT_WEAPONICON            2
#define HUD_COMPONENT_HEALTH                4
#define HUD_COMPONENT_OXYGEN                8
#define HUD_COMPONENT_ARMOUR                16
#define HUD_COMPONENT_RADAR                 32
#define HUD_COMPONENT_CROSSHAIR             64
#define HUD_COMPONENT_MONEY                 128
#define HUD_COMPONENT_LASERDOT              256
#define HUD_COMPONENT_WANTED                512
#define HUD_COMPONENT_RADIO                 1024
#define HUD_COMPONENT_RADIO_TUNING          2048
#define HUD_COMPONENT_CLOCK                 4096
#define HUD_COMPONENT_ALL                   (8191)

// ASCM-Style HUD Component Colour Index Constants
#define HUD_COLOUR_MONEY                    (0)
#define HUD_COLOUR_MONEY_NEGATIVE           (1)
#define HUD_COLOUR_ARMOUR                   (2)
#define HUD_COLOUR_HEALTH                   (3)
#define HUD_COLOUR_BREATH                   (4)
#define HUD_COLOUR_AMMO                     (5)
#define HUD_COLOUR_WANTED                   (6)
#define HUD_COLOUR_RADIO                    (7)
#define HUD_COLOUR_RADIO_TUNING             (8)
#define HUD_COLOUR_CLOCK                    (9)

// Pause Menu Event Types
#define PAUSE_MENU_EVENT_OPEN               (1)
#define PAUSE_MENU_EVENT_CLOSE              (2)
#define PAUSE_MENU_EVENT_SUBMENU            (3)

// Pause Menu Pages (FrontEndMenuManager page IDs)
#define PAUSE_MENU_STATS                    (0)
#define PAUSE_MENU_START_GAME               (1)
#define PAUSE_MENU_BRIEF                    (2)
#define PAUSE_MENU_AUDIO                    (3)
#define PAUSE_MENU_DISPLAY                  (4)
#define PAUSE_MENU_MAP                      (5)
#define PAUSE_MENU_LANGUAGE                 (28)
#define PAUSE_MENU_OPTIONS                  (33)
#define PAUSE_MENU_QUIT                     (35)
#define PAUSE_MENU_CONTROLS                 (36)
#define PAUSE_MENU_REDEFINE                 (37)
#define PAUSE_MENU_MOUSE                    (39)
#define PAUSE_MENU_MAIN                     (41)

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
static stock bool:EXTGAME::PlayerCJActive[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerCJFat[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerCJMuscle[MAX_PLAYERS];
static stock EXTGAME::PlayerCJTexture[MAX_PLAYERS][CJ_SLOT_COUNT][32];
static stock EXTGAME::PlayerCJModel[MAX_PLAYERS][CJ_SLOT_COUNT][32];

// Server-side presence tracking for client ASI plugin
static stock bool:EXTGAME::PlayerHasPlugin[MAX_PLAYERS];
static stock EXTGAME::PlayerPluginVersion[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerPluginChecked[MAX_PLAYERS];

// ExtendedGame server-side authoritative state storage
static stock Float:EXTGAME::PlayerAircraftHeightLimit[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackHeightLimit[MAX_PLAYERS];
static stock Float:EXTGAME::GlobalAircraftHeightLimit = 800.0;
static stock Float:EXTGAME::GlobalJetpackHeightLimit = 100.0;

// ExtendedGame Jetpack Placement & Orientation storage
static stock Float:EXTGAME::PlayerJetpackOffsetX[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackOffsetY[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackOffsetZ[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackRotAngle[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackRotAxisX[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackRotAxisY[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerJetpackRotAxisZ[MAX_PLAYERS];

static stock Float:EXTGAME::GlobalJetpackOffsetX  = 0.10;
static stock Float:EXTGAME::GlobalJetpackOffsetY  = -0.38;
static stock Float:EXTGAME::GlobalJetpackOffsetZ  = 0.00;
static stock Float:EXTGAME::GlobalJetpackRotAngle = 90.0;
static stock Float:EXTGAME::GlobalJetpackRotAxisX = 0.0;
static stock Float:EXTGAME::GlobalJetpackRotAxisY = 1.0;
static stock Float:EXTGAME::GlobalJetpackRotAxisZ = 0.0;
static stock EXTGAME::PlayerKnockOffBike[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerFireProof[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerInfiniteSprint[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerSunEnabled[MAX_PLAYERS];
static stock EXTGAME::GlobalKnockOffBike;
static stock bool:EXTGAME::GlobalFireProof;
static stock bool:EXTGAME::GlobalInfiniteSprint;
static stock bool:EXTGAME::GlobalSunEnabled = true;

// Global Sky Defaults
static stock bool:EXTGAME::GlobalMoonEnabled         = true;
static stock bool:EXTGAME::GlobalStarsEnabled        = true;
static stock bool:EXTGAME::GlobalLowCloudsEnabled    = true;
static stock bool:EXTGAME::GlobalFluffyCloudsEnabled = true;
static stock bool:EXTGAME::GlobalRainbowEnabled      = true;

// ExtendedGame Sky and HUD storage
static stock bool:EXTGAME::PlayerMoonEnabled[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerStarsEnabled[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerLowCloudsEnabled[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerFluffyCloudsEnabled[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerRainbowEnabled[MAX_PLAYERS];

static stock EXTGAME::PlayerHUDComponents[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourAmmo[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourWeaponIcon[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourHealth[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourOxygen[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourArmour[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourRadar[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourCrosshair[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourMoney[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourLaserDot[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourWanted[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourRadio[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourRadioTuning[MAX_PLAYERS];
static stock EXTGAME::PlayerHUDColourClock[MAX_PLAYERS];
static stock EXTGAME::GlobalHUDComponents = 8191;
static stock EXTGAME::GlobalHUDColours[13];

// Vision storage
static stock bool:EXTGAME::PlayerNightVision[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerThermalVision[MAX_PLAYERS];
static stock bool:EXTGAME::GlobalNightVision;
static stock bool:EXTGAME::GlobalThermalVision;

// Weapon reload storage
static stock bool:EXTGAME::PlayerNoReload[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerSwitchReload[MAX_PLAYERS] = { true, ... };
static stock bool:EXTGAME::PlayerQuickReload[MAX_PLAYERS];
static stock bool:EXTGAME::GlobalNoReload;
static stock bool:EXTGAME::GlobalSwitchReload = true;
static stock bool:EXTGAME::GlobalQuickReload;

// Physics & movement storage
static stock bool:EXTGAME::PlayerInfiniteOxygen[MAX_PLAYERS];
static stock EXTGAME::PlayerPedAnims[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerCursor[MAX_PLAYERS];
static stock bool:EXTGAME::GlobalInfiniteOxygen;

// Plane radar altimeter storage (hexadecimal 0xRRGGBBAA)
static stock EXTGAME::PlayerPlaneRadarColor[MAX_PLAYERS] = { 0xFFFFFFFF, ... };
static stock EXTGAME::GlobalPlaneRadarColor = 0xFFFFFFFF;

// Sun color storage (hexadecimal 0xRRGGBBAA)
static stock bool:EXTGAME::PlayerSunColorSet[MAX_PLAYERS];
static stock EXTGAME::PlayerSunCoreColor[MAX_PLAYERS] = { 0xFFFFFFFF, ... };
static stock EXTGAME::PlayerSunCoronaColor[MAX_PLAYERS] = { 0xFFFFFFFF, ... };
static stock bool:EXTGAME::GlobalSunColorSet;
static stock EXTGAME::GlobalSunCoreColor = 0xFFFFFFFF;
static stock EXTGAME::GlobalSunCoronaColor = 0xFFFFFFFF;

// Moon color storage (hexadecimal 0xRRGGBBAA)
static stock bool:EXTGAME::PlayerMoonColorSet[MAX_PLAYERS];
static stock EXTGAME::PlayerMoonColor[MAX_PLAYERS] = { 0xFFFFFFFF, ... };
static stock bool:EXTGAME::GlobalMoonColorSet;
static stock EXTGAME::GlobalMoonColor = 0xFFFFFFFF;

// Vehicle speed blur storage
static stock bool:EXTGAME::PlayerVehicleBlur[MAX_PLAYERS];
static stock EXTGAME::PlayerVehicleBlurIntensity[MAX_PLAYERS] = { 36, ... };
static stock bool:EXTGAME::GlobalVehicleBlur;
static stock EXTGAME::GlobalVehicleBlurIntensity = 36;

// Pause menu tracking storage
static stock bool:EXTGAME::PlayerInPauseMenu[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerWaterLevelSet[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerWaterLevel[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerWaterIncludeNonSea[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerWaterIncludeSea[MAX_PLAYERS];
static stock bool:EXTGAME::PlayerWaterIncludeOutside[MAX_PLAYERS];
static stock Float:EXTGAME::PlayerWaterWaveHeight[MAX_PLAYERS];

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
        PR_UINT8, EXTGAME::PlayerKnockOffBike[targetid]
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
        PR_UINT8, EXTGAME::PlayerFireProof[targetid] ? 1 : 0
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

static stock _SyncExtGameActorState(targetid, recipient)
{
    if (targetid < 0 || targetid >= MAX_PLAYERS || !IsPlayerConnected(targetid) ||
        recipient < 0 || recipient >= MAX_PLAYERS || !IsPlayerConnected(recipient))
        return 0;

    if (EXTGAME::PlayerCJActive[targetid] || GetPlayerSkin(targetid) == 0)
        SyncPlayerCJForPlayer(targetid, recipient);

    _SendPlayerBikeState(recipient, targetid);
    _SendPlayerFireProofState(recipient, targetid);
    _SendPlayerPedAnimsPacket(recipient, targetid, EXTGAME::PlayerPedAnims[targetid]);
    return 1;
}

static stock _SyncPlayerExtendedGameSettings(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid))
        return 0;

    SetAircraftHeightLimitForPlayer(playerid, EXTGAME::PlayerAircraftHeightLimit[playerid]);
    SetJetpackHeightLimitForPlayer(playerid, EXTGAME::PlayerJetpackHeightLimit[playerid]);
    _SendJetpackOffsetPacket(playerid,
        EXTGAME::PlayerJetpackOffsetX[playerid],
        EXTGAME::PlayerJetpackOffsetY[playerid],
        EXTGAME::PlayerJetpackOffsetZ[playerid],
        EXTGAME::PlayerJetpackRotAngle[playerid],
        EXTGAME::PlayerJetpackRotAxisX[playerid],
        EXTGAME::PlayerJetpackRotAxisY[playerid],
        EXTGAME::PlayerJetpackRotAxisZ[playerid]
    );
    TogglePlayerInfiniteSprint(playerid, EXTGAME::PlayerInfiniteSprint[playerid]);
    TogglePlayerSun(playerid, EXTGAME::PlayerSunEnabled[playerid]);
    TogglePlayerMoon(playerid, EXTGAME::PlayerMoonEnabled[playerid]);
    TogglePlayerStars(playerid, EXTGAME::PlayerStarsEnabled[playerid]);
    TogglePlayerLowClouds(playerid, EXTGAME::PlayerLowCloudsEnabled[playerid]);
    TogglePlayerFluffyClouds(playerid, EXTGAME::PlayerFluffyCloudsEnabled[playerid]);
    TogglePlayerRainbow(playerid, EXTGAME::PlayerRainbowEnabled[playerid]);
    TogglePlayerHUDComponents(playerid, EXTGAME::PlayerHUDComponents[playerid]);

    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_AMMO, EXTGAME::PlayerHUDColourAmmo[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_WEAPONICON, EXTGAME::PlayerHUDColourWeaponIcon[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_HEALTH, EXTGAME::PlayerHUDColourHealth[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_OXYGEN, EXTGAME::PlayerHUDColourOxygen[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_ARMOUR, EXTGAME::PlayerHUDColourArmour[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_RADAR, EXTGAME::PlayerHUDColourRadar[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_CROSSHAIR, EXTGAME::PlayerHUDColourCrosshair[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_MONEY, EXTGAME::PlayerHUDColourMoney[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_LASERDOT, EXTGAME::PlayerHUDColourLaserDot[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_WANTED, EXTGAME::PlayerHUDColourWanted[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_RADIO, EXTGAME::PlayerHUDColourRadio[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_RADIO_TUNING, EXTGAME::PlayerHUDColourRadioTuning[playerid]);
    SetPlayerHUDComponentsColour(playerid, HUD_COMPONENT_CLOCK, EXTGAME::PlayerHUDColourClock[playerid]);

    ToggleNightVision(playerid, EXTGAME::PlayerNightVision[playerid]);
    ToggleThermalVision(playerid, EXTGAME::PlayerThermalVision[playerid]);
    SetPlayerNoReload(playerid, EXTGAME::PlayerNoReload[playerid]);
    TogglePlayerSwitchReload(playerid, EXTGAME::PlayerSwitchReload[playerid]);
    ToggleQuickReloadForPlayer(playerid, EXTGAME::PlayerQuickReload[playerid]);
    TogglePlayerInfiniteOxygen(playerid, EXTGAME::PlayerInfiniteOxygen[playerid]);
    TogglePlayerCursor(playerid, EXTGAME::PlayerCursor[playerid]);

    SetPlaneRadarColor(playerid, EXTGAME::PlayerPlaneRadarColor[playerid]);

    if (EXTGAME::PlayerSunColorSet[playerid]) {
        SetPlayerSunColor(playerid, EXTGAME::PlayerSunCoreColor[playerid], EXTGAME::PlayerSunCoronaColor[playerid]);
    } else {
        ResetPlayerSunColor(playerid);
    }

    if (EXTGAME::PlayerMoonColorSet[playerid]) {
        SetPlayerMoonColor(playerid, EXTGAME::PlayerMoonColor[playerid]);
    } else {
        ResetPlayerMoonColor(playerid);
    }

    TogglePlayerVehicleBlur(playerid, EXTGAME::PlayerVehicleBlur[playerid], EXTGAME::PlayerVehicleBlurIntensity[playerid]);

    if (EXTGAME::PlayerWaterLevelSet[playerid]) {
        SetPlayerWaterLevel(
            playerid,
            EXTGAME::PlayerWaterLevel[playerid],
            EXTGAME::PlayerWaterIncludeNonSea[playerid],
            EXTGAME::PlayerWaterIncludeSea[playerid],
            EXTGAME::PlayerWaterIncludeOutside[playerid]
        );
    } else {
        ResetPlayerWaterLevel(playerid);
    }
    SetPlayerWaterWaveHeight(playerid, EXTGAME::PlayerWaterWaveHeight[playerid]);

    if (EXTGAME::ActiveTextDrawRotationsCount > 0) {
        for (new _td = 0; _td < MAX_TEXT_DRAWS; _td++) {
            if (EXTGAME::GlobalTextDrawRotation[_td][tdRotEnabled]) {
                SetTextDrawRotationForPlayer(
                    playerid,
                    Text:_td,
                    EXTGAME::GlobalTextDrawRotation[_td][tdRotAngle],
                    EXTGAME::GlobalTextDrawRotation[_td][tdRotPivotX],
                    EXTGAME::GlobalTextDrawRotation[_td][tdRotPivotY],
                    EXTGAME::GlobalTextDrawRotation[_td][tdRotSpinSpeed]
                );
            }
        }
    }

    if (EXTGAME::ActiveWaterZonesCount > 0) {
        for (new _wz = 0; _wz < MAX_WATER_ZONES; _wz++) {
            if (EXTGAME::GlobalWaterZones[_wz][wzActive]) {
                CreateWaterZoneForPlayer(
                    playerid,
                    _wz,
                    EXTGAME::GlobalWaterZones[_wz][wzMinX],
                    EXTGAME::GlobalWaterZones[_wz][wzMinY],
                    EXTGAME::GlobalWaterZones[_wz][wzMaxX],
                    EXTGAME::GlobalWaterZones[_wz][wzMaxY],
                    EXTGAME::GlobalWaterZones[_wz][wzLevel],
                    EXTGAME::GlobalWaterZones[_wz][wzBottomZ]
                );
            }
        }
    }
    return 1;
}

// Forward callback triggered when a player's plugin status is resolved:
forward OnPlayerExtendedGameCheck(playerid, bool:installed, version);

/**
 * Checks if a player has the BrownTurbo-ExtendedGame ASI plugin installed
 */
stock bool:HasPlayerExtendedGame(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerHasPlugin[playerid];
}

/**
 * Returns the version of the BrownTurbo-ExtendedGame ASI plugin installed by the player
 * Returns 0 if not installed
 */
stock GetPlayerExtendedGameVersion(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    return EXTGAME::PlayerPluginVersion[playerid];
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
    EXTGAME::PlayerCJActive[playerid] = false;
    EXTGAME::PlayerCJFat[playerid] = 0.0;
    EXTGAME::PlayerCJMuscle[playerid] = 0.0;
    for (new s = 0; s < CJ_SLOT_COUNT; s++) {
        EXTGAME::PlayerCJTexture[playerid][s][0] = '\0';
        EXTGAME::PlayerCJModel[playerid][s][0] = '\0';
    }
    return 1;
}

/**
 * Checks whether a player currently has an active custom CJ skin on the server
 */
stock bool:IsPlayerUsingCJSkin(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerCJActive[playerid];
}

/**
 * Retrieves a player's current CJ body stats from server memory
 */
stock GetPlayerCJBodyStats(playerid, &Float:fat, &Float:muscle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    fat = EXTGAME::PlayerCJFat[playerid];
    muscle = EXTGAME::PlayerCJMuscle[playerid];
    return 1;
}

/**
 * Retrieves a player's current clothes item in a specific slot from server memory
 */
stock GetPlayerCJClothes(playerid, slot, texture[], model[], max_len = 32) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || slot < 0 || slot >= CJ_SLOT_COUNT) return 0;
    format(texture, max_len, "%s", EXTGAME::PlayerCJTexture[playerid][slot]);
    format(model, max_len, "%s", EXTGAME::PlayerCJModel[playerid][slot]);
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

    if (EXTGAME::PlayerCJActive[targetid]) {
        new slots[CJ_SLOT_COUNT];
        new textures[CJ_SLOT_COUNT][32];
        new models[CJ_SLOT_COUNT][32];
        new count = 0;

        for (new s = 0; s < CJ_SLOT_COUNT; s++) {
            if (EXTGAME::PlayerCJTexture[targetid][s][0] != '\0') {
                slots[count] = s;
                format(textures[count], 32, "%s", EXTGAME::PlayerCJTexture[targetid][s]);
                format(models[count], 32, "%s", EXTGAME::PlayerCJModel[targetid][s]);
                count++;
            }
        }

        _SendRawCJOutfitPacket(forplayerid, targetid,
            EXTGAME::PlayerCJFat[targetid],
            EXTGAME::PlayerCJMuscle[targetid],
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
        EXTGAME::PlayerCJActive[targetid] = true;
        format(EXTGAME::PlayerCJTexture[targetid][slot], 32, "%s", texture);
        format(EXTGAME::PlayerCJModel[targetid][slot], 32, "%s", model);
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
        EXTGAME::PlayerCJActive[targetid] = true;
        EXTGAME::PlayerCJFat[targetid] = fat;
        EXTGAME::PlayerCJMuscle[targetid] = muscle;
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
        EXTGAME::PlayerCJActive[targetid] = true;
        if (to_default) {
            format(EXTGAME::PlayerCJTexture[targetid][CJ_SLOT_TORSO], 32, "vest");
            format(EXTGAME::PlayerCJModel[targetid][CJ_SLOT_TORSO], 32, "vest");
            format(EXTGAME::PlayerCJTexture[targetid][CJ_SLOT_LEGS], 32, "jeansdenim");
            format(EXTGAME::PlayerCJModel[targetid][CJ_SLOT_LEGS], 32, "jeans");
            format(EXTGAME::PlayerCJTexture[targetid][CJ_SLOT_SHOES], 32, "sneakerbincblk");
            format(EXTGAME::PlayerCJModel[targetid][CJ_SLOT_SHOES], 32, "sneaker");
            format(EXTGAME::PlayerCJTexture[targetid][CJ_SLOT_HEAD], 32, "player_face");
            format(EXTGAME::PlayerCJModel[targetid][CJ_SLOT_HEAD], 32, "head");
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
        EXTGAME::PlayerCJActive[targetid] = true;
        EXTGAME::PlayerCJFat[targetid] = fat;
        EXTGAME::PlayerCJMuscle[targetid] = muscle;
        for (new i = 0; i < count; i++) {
            new s = slots[i];
            if (s >= 0 && s < CJ_SLOT_COUNT) {
                format(EXTGAME::PlayerCJTexture[targetid][s], 32, "%s", textures[i]);
                format(EXTGAME::PlayerCJModel[targetid][s], 32, "%s", models[i]);
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
        EXTGAME::GlobalAircraftHeightLimit = heightLimit;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            EXTGAME::PlayerAircraftHeightLimit[i] = heightLimit;
            if (IsPlayerConnected(i)) {
                _SendAircraftHeightLimitPacket(i, heightLimit);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerAircraftHeightLimit[playerid] = heightLimit;

    if (IsPlayerConnected(playerid)) {
        _SendAircraftHeightLimitPacket(playerid, heightLimit);
    }
    return 1;
}

stock Float:GetPlayerAircraftHeightLimit(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 800.0;
    return EXTGAME::PlayerAircraftHeightLimit[playerid];
}

/**
 * Sets the jetpack ceiling height limit for a player (or -1 for global broadcast)
 */
stock SetJetpackHeightLimitForPlayer(playerid, Float:heightLimit) {
    if (playerid == -1) {
        EXTGAME::GlobalJetpackHeightLimit = heightLimit;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            EXTGAME::PlayerJetpackHeightLimit[i] = heightLimit;
            if (IsPlayerConnected(i)) {
                _SendJetpackHeightLimitPacket(i, heightLimit);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerJetpackHeightLimit[playerid] = heightLimit;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackHeightLimitPacket(playerid, heightLimit);
    }
    return 1;
}

stock Float:GetPlayerJetpackHeightLimit(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 100.0;
    return EXTGAME::PlayerJetpackHeightLimit[playerid];
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
        EXTGAME::GlobalJetpackOffsetX = x;
        EXTGAME::GlobalJetpackOffsetY = y;
        EXTGAME::GlobalJetpackOffsetZ = z;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            EXTGAME::PlayerJetpackOffsetX[i] = x;
            EXTGAME::PlayerJetpackOffsetY[i] = y;
            EXTGAME::PlayerJetpackOffsetZ[i] = z;
            if (IsPlayerConnected(i)) {
                _SendJetpackOffsetPacket(i, x, y, z, EXTGAME::PlayerJetpackRotAngle[i], EXTGAME::PlayerJetpackRotAxisX[i], EXTGAME::PlayerJetpackRotAxisY[i], EXTGAME::PlayerJetpackRotAxisZ[i]);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerJetpackOffsetX[playerid] = x;
    EXTGAME::PlayerJetpackOffsetY[playerid] = y;
    EXTGAME::PlayerJetpackOffsetZ[playerid] = z;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackOffsetPacket(playerid, x, y, z, EXTGAME::PlayerJetpackRotAngle[playerid], EXTGAME::PlayerJetpackRotAxisX[playerid], EXTGAME::PlayerJetpackRotAxisY[playerid], EXTGAME::PlayerJetpackRotAxisZ[playerid]);
    }
    return 1;
}

stock GetPlayerJetpackOffset(playerid, &Float:x, &Float:y, &Float:z) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) {
        x = EXTGAME::GlobalJetpackOffsetX;
        y = EXTGAME::GlobalJetpackOffsetY;
        z = EXTGAME::GlobalJetpackOffsetZ;
        return 0;
    }
    x = EXTGAME::PlayerJetpackOffsetX[playerid];
    y = EXTGAME::PlayerJetpackOffsetY[playerid];
    z = EXTGAME::PlayerJetpackOffsetZ[playerid];
    return 1;
}

/**
 * Sets the jetpack clump rotation for a player (or -1 for global broadcast)
 */
stock SetJetpackRotationForPlayer(playerid, Float:angle, Float:axisX = 0.0, Float:axisY = 1.0, Float:axisZ = 0.0) {
    if (playerid == -1) {
        EXTGAME::GlobalJetpackRotAngle = angle;
        EXTGAME::GlobalJetpackRotAxisX = axisX;
        EXTGAME::GlobalJetpackRotAxisY = axisY;
        EXTGAME::GlobalJetpackRotAxisZ = axisZ;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            EXTGAME::PlayerJetpackRotAngle[i] = angle;
            EXTGAME::PlayerJetpackRotAxisX[i] = axisX;
            EXTGAME::PlayerJetpackRotAxisY[i] = axisY;
            EXTGAME::PlayerJetpackRotAxisZ[i] = axisZ;
            if (IsPlayerConnected(i)) {
                _SendJetpackOffsetPacket(i, EXTGAME::PlayerJetpackOffsetX[i], EXTGAME::PlayerJetpackOffsetY[i], EXTGAME::PlayerJetpackOffsetZ[i], angle, axisX, axisY, axisZ);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerJetpackRotAngle[playerid] = angle;
    EXTGAME::PlayerJetpackRotAxisX[playerid] = axisX;
    EXTGAME::PlayerJetpackRotAxisY[playerid] = axisY;
    EXTGAME::PlayerJetpackRotAxisZ[playerid] = axisZ;

    if (IsPlayerConnected(playerid)) {
        _SendJetpackOffsetPacket(playerid, EXTGAME::PlayerJetpackOffsetX[playerid], EXTGAME::PlayerJetpackOffsetY[playerid], EXTGAME::PlayerJetpackOffsetZ[playerid], angle, axisX, axisY, axisZ);
    }
    return 1;
}

stock GetPlayerJetpackRotation(playerid, &Float:angle, &Float:axisX, &Float:axisY, &Float:axisZ) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) {
        angle = EXTGAME::GlobalJetpackRotAngle;
        axisX = EXTGAME::GlobalJetpackRotAxisX;
        axisY = EXTGAME::GlobalJetpackRotAxisY;
        axisZ = EXTGAME::GlobalJetpackRotAxisZ;
        return 0;
    }
    angle = EXTGAME::PlayerJetpackRotAngle[playerid];
    axisX = EXTGAME::PlayerJetpackRotAxisX[playerid];
    axisY = EXTGAME::PlayerJetpackRotAxisY[playerid];
    axisZ = EXTGAME::PlayerJetpackRotAxisZ[playerid];
    return 1;
}

/**
 * Resets the jetpack clump placement and rotation to default
 */
stock ResetJetpackOffsetForPlayer(playerid) {
    if (playerid == -1) {
        EXTGAME::GlobalJetpackOffsetX  = 0.10;
        EXTGAME::GlobalJetpackOffsetY  = -0.38;
        EXTGAME::GlobalJetpackOffsetZ  = 0.00;
        EXTGAME::GlobalJetpackRotAngle = 90.0;
        EXTGAME::GlobalJetpackRotAxisX = 0.0;
        EXTGAME::GlobalJetpackRotAxisY = 1.0;
        EXTGAME::GlobalJetpackRotAxisZ = 0.0;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            EXTGAME::PlayerJetpackOffsetX[i]  = 0.10;
            EXTGAME::PlayerJetpackOffsetY[i]  = -0.38;
            EXTGAME::PlayerJetpackOffsetZ[i]  = 0.00;
            EXTGAME::PlayerJetpackRotAngle[i] = 90.0;
            EXTGAME::PlayerJetpackRotAxisX[i] = 0.0;
            EXTGAME::PlayerJetpackRotAxisY[i] = 1.0;
            EXTGAME::PlayerJetpackRotAxisZ[i] = 0.0;
            if (IsPlayerConnected(i)) {
                _SendResetJetpackOffsetPacket(i);
            }
        }
        return 1;
    }

    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerJetpackOffsetX[playerid]  = 0.10;
    EXTGAME::PlayerJetpackOffsetY[playerid]  = -0.38;
    EXTGAME::PlayerJetpackOffsetZ[playerid]  = 0.00;
    EXTGAME::PlayerJetpackRotAngle[playerid] = 90.0;
    EXTGAME::PlayerJetpackRotAxisX[playerid] = 0.0;
    EXTGAME::PlayerJetpackRotAxisY[playerid] = 1.0;
    EXTGAME::PlayerJetpackRotAxisZ[playerid] = 0.0;

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
    EXTGAME::PlayerKnockOffBike[playerid] = knockState;

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
        EXTGAME::GlobalKnockOffBike = knockState;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyKnockedOffBike(i, knockState);
        }
        return 1;
    }
    return _ApplyKnockedOffBike(playerid, knockState);
}

stock GetPlayerKnockedOffBikeState(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return BIKE_KNOCK_OFF_DEFAULT;
    return EXTGAME::PlayerKnockOffBike[playerid];
}

static stock _ApplyFireProof(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerFireProof[playerid] = (enable != 0);

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
        EXTGAME::GlobalFireProof = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyFireProof(i, enable);
        }
        return 1;
    }
    return _ApplyFireProof(playerid, enable);
}

stock bool:IsPlayerFireProof(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerFireProof[playerid];
}

static stock _ApplyInfiniteSprint(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerInfiniteSprint[playerid] = (enable != 0);

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
        EXTGAME::GlobalInfiniteSprint = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyInfiniteSprint(i, enable);
        }
        return 1;
    }
    return _ApplyInfiniteSprint(playerid, enable);
}

stock bool:IsPlayerInfiniteSprintEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerInfiniteSprint[playerid];
}

static stock _ApplySun(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerSunEnabled[playerid] = (enable != 0);

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
        EXTGAME::GlobalSunEnabled = (enable != 0);
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplySun(i, enable);
        }
        return 1;
    }
    return _ApplySun(playerid, enable);
}

stock bool:IsPlayerSunEnabled(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return true;
    return EXTGAME::PlayerSunEnabled[playerid];
}

/**
 * Toggles the visibility of the moon globally for all players
 */
static stock _ApplyPlayerMoon(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerMoonEnabled[playerid] = (enable != 0);

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
    EXTGAME::GlobalMoonEnabled = (enable != 0);
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
    return EXTGAME::PlayerMoonEnabled[playerid];
}

static stock _ApplyPlayerStars(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerStarsEnabled[playerid] = (enable != 0);

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
    EXTGAME::GlobalStarsEnabled = (enable != 0);
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
    return EXTGAME::PlayerStarsEnabled[playerid];
}

static stock _ApplyPlayerLowClouds(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerLowCloudsEnabled[playerid] = (enable != 0);

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
    EXTGAME::GlobalLowCloudsEnabled = (enable != 0);
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
    return EXTGAME::PlayerLowCloudsEnabled[playerid];
}

static stock _ApplyPlayerFluffyClouds(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerFluffyCloudsEnabled[playerid] = (enable != 0);

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
    EXTGAME::GlobalFluffyCloudsEnabled = (enable != 0);
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
    return EXTGAME::PlayerFluffyCloudsEnabled[playerid];
}

static stock _ApplyPlayerRainbow(playerid, enable) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerRainbowEnabled[playerid] = (enable != 0);

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
    EXTGAME::GlobalRainbowEnabled = (enable != 0);
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
    return EXTGAME::PlayerRainbowEnabled[playerid];
}

/**
 * Sets world water vertices for a player (or all players with -1), like MTA's
 * world-water controls. Outside-world water is unchanged unless requested.
 */
static stock _ApplyPlayerWaterLevel(playerid, Float:level, bool:includeWorldNonSeaLevel, bool:includeWorldSeaLevel, bool:includeOutsideWorldLevel) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerWaterLevelSet[playerid] = true;
    EXTGAME::PlayerWaterLevel[playerid] = level;
    EXTGAME::PlayerWaterIncludeNonSea[playerid] = includeWorldNonSeaLevel;
    EXTGAME::PlayerWaterIncludeSea[playerid] = includeWorldSeaLevel;
    EXTGAME::PlayerWaterIncludeOutside[playerid] = includeOutsideWorldLevel;
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
        EXTGAME::GlobalWaterLevelSet = true;
        EXTGAME::GlobalWaterLevel = level;
        EXTGAME::GlobalWaterIncludeNonSea = includeWorldNonSeaLevel;
        EXTGAME::GlobalWaterIncludeSea = includeWorldSeaLevel;
        EXTGAME::GlobalWaterIncludeOutside = includeOutsideWorldLevel;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyPlayerWaterLevel(i, level, includeWorldNonSeaLevel, includeWorldSeaLevel, includeOutsideWorldLevel);
        }
        return 1;
    }
    return _ApplyPlayerWaterLevel(playerid, level, includeWorldNonSeaLevel, includeWorldSeaLevel, includeOutsideWorldLevel);
}

stock Float:GetPlayerWaterLevel(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0.0;
    return EXTGAME::PlayerWaterLevel[playerid];
}

static stock _ApplyResetPlayerWaterLevel(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerWaterLevelSet[playerid] = false;
    EXTGAME::PlayerWaterLevel[playerid] = 0.0;
    EXTGAME::PlayerWaterIncludeNonSea[playerid] = true;
    EXTGAME::PlayerWaterIncludeSea[playerid] = true;
    EXTGAME::PlayerWaterIncludeOutside[playerid] = true;
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
        EXTGAME::GlobalWaterLevelSet = false;
        EXTGAME::GlobalWaterLevel = 0.0;
        EXTGAME::GlobalWaterIncludeNonSea = true;
        EXTGAME::GlobalWaterIncludeSea = true;
        EXTGAME::GlobalWaterIncludeOutside = true;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyResetPlayerWaterLevel(i);
        }
        return 1;
    }
    return _ApplyResetPlayerWaterLevel(playerid);
}

static stock _ApplyPlayerWaterWaveHeight(playerid, Float:height) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerWaterWaveHeight[playerid] = height;
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
        EXTGAME::GlobalWaterWaveHeight = height;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyPlayerWaterWaveHeight(i, height);
        }
        return 1;
    }
    return _ApplyPlayerWaterWaveHeight(playerid, height);
}

stock Float:GetPlayerWaterWaveHeight(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0.0;
    if (EXTGAME::PlayerWaterWaveHeight[playerid] < 0.0) return 0.6;
    return EXTGAME::PlayerWaterWaveHeight[playerid];
}

static stock _ApplyTogglePlayerHUD(playerid, hudComponents) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    EXTGAME::PlayerHUDComponents[playerid] = hudComponents;

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
        EXTGAME::GlobalHUDComponents = hudComponents;
        for (new i = 0; i < MAX_PLAYERS; i++) {
            _ApplyTogglePlayerHUD(i, hudComponents);
        }
        return 1;
    }
    return _ApplyTogglePlayerHUD(playerid, hudComponents);
}

stock GetPlayerHUDComponents(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return HUD_COMPONENT_ALL;
    return EXTGAME::PlayerHUDComponents[playerid];
}

static stock _ApplyPlayerHUDColour(playerid, hudComponents, intColour) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;

    if (hudComponents & HUD_COMPONENT_AMMO)       EXTGAME::PlayerHUDColourAmmo[playerid]       = intColour;
    if (hudComponents & HUD_COMPONENT_WEAPONICON) EXTGAME::PlayerHUDColourWeaponIcon[playerid] = intColour;
    if (hudComponents & HUD_COMPONENT_HEALTH)     EXTGAME::PlayerHUDColourHealth[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_OXYGEN)     EXTGAME::PlayerHUDColourOxygen[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_ARMOUR)     EXTGAME::PlayerHUDColourArmour[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_RADAR)      EXTGAME::PlayerHUDColourRadar[playerid]      = intColour;
    if (hudComponents & HUD_COMPONENT_CROSSHAIR)  EXTGAME::PlayerHUDColourCrosshair[playerid]  = intColour;
    if (hudComponents & HUD_COMPONENT_MONEY)      EXTGAME::PlayerHUDColourMoney[playerid]      = intColour;
    if (hudComponents & HUD_COMPONENT_LASERDOT)   EXTGAME::PlayerHUDColourLaserDot[playerid]   = intColour;
    if (hudComponents & HUD_COMPONENT_WANTED)     EXTGAME::PlayerHUDColourWanted[playerid]     = intColour;
    if (hudComponents & HUD_COMPONENT_RADIO)      EXTGAME::PlayerHUDColourRadio[playerid]      = intColour;
    if (hudComponents & HUD_COMPONENT_RADIO_TUNING) EXTGAME::PlayerHUDColourRadioTuning[playerid] = intColour;
    if (hudComponents & HUD_COMPONENT_CLOCK)      EXTGAME::PlayerHUDColourClock[playerid]      = intColour;

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
        for (new component = 0; component < 13; component++) {
            if (hudComponents & (1 << component))
                EXTGAME::GlobalHUDColours[component] = intColour;
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
        case HUD_COMPONENT_AMMO:         return EXTGAME::PlayerHUDColourAmmo[playerid];
        case HUD_COMPONENT_WEAPONICON:   return EXTGAME::PlayerHUDColourWeaponIcon[playerid];
        case HUD_COMPONENT_HEALTH:       return EXTGAME::PlayerHUDColourHealth[playerid];
        case HUD_COMPONENT_OXYGEN:       return EXTGAME::PlayerHUDColourOxygen[playerid];
        case HUD_COMPONENT_ARMOUR:       return EXTGAME::PlayerHUDColourArmour[playerid];
        case HUD_COMPONENT_RADAR:        return EXTGAME::PlayerHUDColourRadar[playerid];
        case HUD_COMPONENT_CROSSHAIR:    return EXTGAME::PlayerHUDColourCrosshair[playerid];
        case HUD_COMPONENT_MONEY:        return EXTGAME::PlayerHUDColourMoney[playerid];
        case HUD_COMPONENT_LASERDOT:     return EXTGAME::PlayerHUDColourLaserDot[playerid];
        case HUD_COMPONENT_WANTED:       return EXTGAME::PlayerHUDColourWanted[playerid];
        case HUD_COMPONENT_RADIO:        return EXTGAME::PlayerHUDColourRadio[playerid];
        case HUD_COMPONENT_RADIO_TUNING: return EXTGAME::PlayerHUDColourRadioTuning[playerid];
        case HUD_COMPONENT_CLOCK:        return EXTGAME::PlayerHUDColourClock[playerid];
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
    if (!EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled]) {
        EXTGAME::ActiveTextDrawRotationsCount++;
    }
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] = true;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotAngle] = angle;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotX] = pivotX;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotY] = pivotY;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed] = spinSpeed;

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
    if (EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled]) {
        EXTGAME::ActiveTextDrawRotationsCount--;
    }
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] = false;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotAngle] = 0.0;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotX] = -1.0;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotY] = -1.0;
    EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed] = 0.0;

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

    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] = true;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle] = angle;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX] = pivotX;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY] = pivotY;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed] = spinSpeed;

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

    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] = false;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle] = 0.0;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX] = -1.0;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY] = -1.0;
    EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed] = 0.0;

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
    return EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled];
}

/**
 * Retrieves the global TextDraw rotation parameters.
 */
stock GetTextDrawRotation(Text:textdrawid, &Float:angle, &Float:pivotX, &Float:pivotY, &Float:spinSpeed) {
    if (_:textdrawid < 0 || _:textdrawid >= MAX_TEXT_DRAWS) return 0;
    angle = EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotAngle];
    pivotX = EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotX];
    pivotY = EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotPivotY];
    spinSpeed = EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotSpinSpeed];
    return EXTGAME::GlobalTextDrawRotation[_:textdrawid][tdRotEnabled] ? 1 : 0;
}

/**
 * Returns true if a Player TextDraw has active rotation.
 */
stock bool:IsPlayerTextDrawRotated(playerid, PlayerText:textdrawid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return false;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return false;
    return EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled];
}

/**
 * Retrieves the Player TextDraw rotation parameters.
 */
stock GetPlayerTextDrawRotation(playerid, PlayerText:textdrawid, &Float:angle, &Float:pivotX, &Float:pivotY, &Float:spinSpeed) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (_:textdrawid < 0 || _:textdrawid >= MAX_PLAYER_TEXT_DRAWS) return 0;
    angle = EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotAngle];
    pivotX = EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotX];
    pivotY = EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotPivotY];
    spinSpeed = EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotSpinSpeed];
    return EXTGAME::PlayerTextDrawRotation[playerid][_:textdrawid][tdRotEnabled] ? 1 : 0;
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
        if (!EXTGAME::GlobalWaterZones[i][wzActive]) {
            zoneid = i;
            break;
        }
    }
    if (zoneid == INVALID_WATER_ZONE) return INVALID_WATER_ZONE;

    EXTGAME::ActiveWaterZonesCount++;
    EXTGAME::GlobalWaterZones[zoneid][wzActive] = true;
    EXTGAME::GlobalWaterZones[zoneid][wzMinX] = (minX < maxX) ? minX : maxX;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxX] = (minX > maxX) ? minX : maxX;
    EXTGAME::GlobalWaterZones[zoneid][wzMinY] = (minY < maxY) ? minY : maxY;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxY] = (minY > maxY) ? minY : maxY;
    EXTGAME::GlobalWaterZones[zoneid][wzLevel] = level;
    EXTGAME::GlobalWaterZones[zoneid][wzBottomZ] = (bottomZ < level) ? bottomZ : (level - 5.0);

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
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !EXTGAME::GlobalWaterZones[zoneid][wzActive]) return 0;
    EXTGAME::GlobalWaterZones[zoneid][wzLevel] = level;
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
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !EXTGAME::GlobalWaterZones[zoneid][wzActive]) return 0;
    EXTGAME::GlobalWaterZones[zoneid][wzMinX] = (minX < maxX) ? minX : maxX;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxX] = (minX > maxX) ? minX : maxX;
    EXTGAME::GlobalWaterZones[zoneid][wzMinY] = (minY < maxY) ? minY : maxY;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxY] = (minY > maxY) ? minY : maxY;
    EXTGAME::GlobalWaterZones[zoneid][wzLevel] = level;
    EXTGAME::GlobalWaterZones[zoneid][wzBottomZ] = (bottomZ < level) ? bottomZ : (level - 5.0);

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
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !EXTGAME::GlobalWaterZones[zoneid][wzActive]) return 0;
    EXTGAME::ActiveWaterZonesCount--;
    EXTGAME::GlobalWaterZones[zoneid][wzActive] = false;
    EXTGAME::GlobalWaterZones[zoneid][wzMinX] = 0.0;
    EXTGAME::GlobalWaterZones[zoneid][wzMinY] = 0.0;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxX] = 0.0;
    EXTGAME::GlobalWaterZones[zoneid][wzMaxY] = 0.0;
    EXTGAME::GlobalWaterZones[zoneid][wzLevel] = 0.0;
    EXTGAME::GlobalWaterZones[zoneid][wzBottomZ] = -100.0;

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
    return EXTGAME::GlobalWaterZones[zoneid][wzActive];
}

/**
 * Gets the water level of a custom water zone.
 */
stock GetWaterZoneLevel(zoneid, &Float:level) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !EXTGAME::GlobalWaterZones[zoneid][wzActive]) return 0;
    level = EXTGAME::GlobalWaterZones[zoneid][wzLevel];
    return 1;
}

/**
 * Gets the coordinates and bounds of a custom water zone.
 */
stock GetWaterZoneCoords(zoneid, &Float:minX, &Float:minY, &Float:maxX, &Float:maxY, &Float:level, &Float:bottomZ) {
    if (zoneid < 0 || zoneid >= MAX_WATER_ZONES || !EXTGAME::GlobalWaterZones[zoneid][wzActive]) return 0;
    minX = EXTGAME::GlobalWaterZones[zoneid][wzMinX];
    minY = EXTGAME::GlobalWaterZones[zoneid][wzMinY];
    maxX = EXTGAME::GlobalWaterZones[zoneid][wzMaxX];
    maxY = EXTGAME::GlobalWaterZones[zoneid][wzMaxY];
    level = EXTGAME::GlobalWaterZones[zoneid][wzLevel];
    bottomZ = EXTGAME::GlobalWaterZones[zoneid][wzBottomZ];
    return 1;
}

/*
 * =============================================================================
 * Vision Controls
 * =============================================================================
 */
stock ToggleNightVision(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerNightVision[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_NIGHT_VISION,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ToggleNightVisionForAll(bool:toggle) {
    EXTGAME::GlobalNightVision = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) ToggleNightVision(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerNightVision(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerNightVision[playerid];
}

stock ToggleThermalVision(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerThermalVision[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_THERMAL_VISION,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ToggleThermalVisionForAll(bool:toggle) {
    EXTGAME::GlobalThermalVision = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) ToggleThermalVision(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerThermalVision(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerThermalVision[playerid];
}

/*
 * =============================================================================
 * Weapon Reload Mechanics
 * =============================================================================
 */
stock SetPlayerNoReload(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerNoReload[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_NO_RELOAD,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock SetPlayerNoReloadForAll(bool:toggle) {
    EXTGAME::GlobalNoReload = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) SetPlayerNoReload(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerNoReload(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerNoReload[playerid];
}

stock TogglePlayerSwitchReload(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerSwitchReload[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_SWITCH_RELOAD,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock TogglePlayerSwitchReloadForAll(bool:toggle) {
    EXTGAME::GlobalSwitchReload = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) TogglePlayerSwitchReload(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerSwitchReload(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerSwitchReload[playerid];
}

stock ToggleQuickReloadForPlayer(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerQuickReload[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_QUICK_RELOAD,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ToggleQuickReloadForAll(bool:toggle) {
    EXTGAME::GlobalQuickReload = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) ToggleQuickReloadForPlayer(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerQuickReload(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerQuickReload[playerid];
}

/*
 * =============================================================================
 * Physics & Movement Controls
 * =============================================================================
 */
stock TogglePlayerInfiniteOxygen(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerInfiniteOxygen[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_INFINITE_OXYGEN,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock TogglePlayerInfiniteOxygenForAll(bool:toggle) {
    EXTGAME::GlobalInfiniteOxygen = toggle;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) TogglePlayerInfiniteOxygen(i, toggle);
    }
    return 1;
}

stock bool:GetPlayerInfiniteOxygen(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerInfiniteOxygen[playerid];
}

static stock _SendPlayerPedAnimsPacket(recipient, targetid, mode) {
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, recipient,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_PLAYER_PED_ANIMS,
        PR_UINT16, targetid,
        PR_UINT8, mode
    );
    PR_SendPacket(bs, recipient, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
}

stock SetPlayerPedAnims(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    new mode = toggle ? 1 : 2;
    EXTGAME::PlayerPedAnims[playerid] = mode;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) {
            _SendPlayerPedAnimsPacket(i, playerid, mode);
        }
    }
    return 1;
}

stock GetPlayerPedAnims(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    return EXTGAME::PlayerPedAnims[playerid];
}

stock TogglePlayerCursor(playerid, bool:toggle) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerCursor[playerid] = toggle;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_CURSOR,
        PR_UINT8, toggle ? 1 : 0
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock bool:GetCursorState(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerCursor[playerid];
}

/*
 * =============================================================================
 * Plane Radar Altimeter Color (Hexadecimal RGBA)
 * =============================================================================
 */
stock SetPlaneRadarColor(playerid, color, g = -1, b = -1, a = -1) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    new hexColor = color;
    if (g != -1 && b != -1 && a != -1) {
        hexColor = ((color & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);
    }
    EXTGAME::PlayerPlaneRadarColor[playerid] = hexColor;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_PLANE_RADAR_COLOR,
        PR_UINT32, hexColor
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock SetPlaneRadarColorForAll(color, g = -1, b = -1, a = -1) {
    new hexColor = color;
    if (g != -1 && b != -1 && a != -1) {
        hexColor = ((color & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);
    }
    EXTGAME::GlobalPlaneRadarColor = hexColor;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) SetPlaneRadarColor(i, hexColor);
    }
    return 1;
}

stock GetPlaneRadarColor(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0xFFFFFFFF;
    return EXTGAME::PlayerPlaneRadarColor[playerid];
}

/*
 * =============================================================================
 * Sun Color Controls (Hexadecimal RGBA)
 * =============================================================================
 */
stock SetPlayerSunColor(playerid, coreColor, coronaColor = -1) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    if (coronaColor == -1) coronaColor = coreColor;
    EXTGAME::PlayerSunColorSet[playerid] = true;
    EXTGAME::PlayerSunCoreColor[playerid] = coreColor;
    EXTGAME::PlayerSunCoronaColor[playerid] = coronaColor;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_SUN_COLOR,
        PR_UINT32, coreColor,
        PR_UINT32, coronaColor
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock SetSunColorForAll(coreColor, coronaColor = -1) {
    if (coronaColor == -1) coronaColor = coreColor;
    EXTGAME::GlobalSunColorSet = true;
    EXTGAME::GlobalSunCoreColor = coreColor;
    EXTGAME::GlobalSunCoronaColor = coronaColor;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) SetPlayerSunColor(i, coreColor, coronaColor);
    }
    return 1;
}

stock ResetPlayerSunColor(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerSunColorSet[playerid] = false;
    EXTGAME::PlayerSunCoreColor[playerid] = 0xFFFFFFFF;
    EXTGAME::PlayerSunCoronaColor[playerid] = 0xFFFFFFFF;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_RESET_SUN_COLOR
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ResetSunColorForAll() {
    EXTGAME::GlobalSunColorSet = false;
    EXTGAME::GlobalSunCoreColor = 0xFFFFFFFF;
    EXTGAME::GlobalSunCoronaColor = 0xFFFFFFFF;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) ResetPlayerSunColor(i);
    }
    return 1;
}

stock bool:GetPlayerSunColor(playerid, &coreColor, &coronaColor) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    coreColor = EXTGAME::PlayerSunCoreColor[playerid];
    coronaColor = EXTGAME::PlayerSunCoronaColor[playerid];
    return EXTGAME::PlayerSunColorSet[playerid];
}

/*
 * =============================================================================
 * Moon Color Controls (Hexadecimal RGBA)
 * =============================================================================
 */
stock SetPlayerMoonColor(playerid, color) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerMoonColorSet[playerid] = true;
    EXTGAME::PlayerMoonColor[playerid] = color;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_SET_MOON_COLOR,
        PR_UINT32, color
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock SetMoonColorForAll(color) {
    EXTGAME::GlobalMoonColorSet = true;
    EXTGAME::GlobalMoonColor = color;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) SetPlayerMoonColor(i, color);
    }
    return 1;
}

stock ResetPlayerMoonColor(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerMoonColorSet[playerid] = false;
    EXTGAME::PlayerMoonColor[playerid] = 0xFFFFFFFF;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_RESET_MOON_COLOR
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ResetMoonColorForAll() {
    EXTGAME::GlobalMoonColorSet = false;
    EXTGAME::GlobalMoonColor = 0xFFFFFFFF;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) ResetPlayerMoonColor(i);
    }
    return 1;
}

stock bool:GetPlayerMoonColor(playerid, &color) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    color = EXTGAME::PlayerMoonColor[playerid];
    return EXTGAME::PlayerMoonColorSet[playerid];
}

/*
 * =============================================================================
 * Vehicle Blur Motion Controls
 * =============================================================================
 */
stock TogglePlayerVehicleBlur(playerid, bool:toggle, intensity = 36) {
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 0;
    EXTGAME::PlayerVehicleBlur[playerid] = toggle;
    EXTGAME::PlayerVehicleBlurIntensity[playerid] = intensity;
    new BitStream:bs = BS_New();
    BS_WriteValue(bs,
        PR_UINT8, EXTENDEDGAME_PACKET_ID,
        PR_UINT16, playerid,
        PR_UINT8, EXTENDEDGAME_ACTION_TOGGLE_VEHICLE_BLUR,
        PR_UINT8, toggle ? 1 : 0,
        PR_UINT8, intensity
    );
    PR_SendPacket(bs, playerid, PR_HIGH_PRIORITY, PR_RELIABLE_ORDERED);
    BS_Delete(bs);
    return 1;
}

stock ToggleVehicleBlurForAll(bool:toggle, intensity = 36) {
    EXTGAME::GlobalVehicleBlur = toggle;
    EXTGAME::GlobalVehicleBlurIntensity = intensity;
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (IsPlayerConnected(i)) TogglePlayerVehicleBlur(i, toggle, intensity);
    }
    return 1;
}

stock bool:GetPlayerVehicleBlur(playerid, &intensity = 0) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    intensity = EXTGAME::PlayerVehicleBlurIntensity[playerid];
    return EXTGAME::PlayerVehicleBlur[playerid];
}

/*
 * =============================================================================
 * Pause Menu Tracking
 * =============================================================================
 */
forward OnPlayerOpenPauseMenu(playerid);
forward OnPlayerClosePauseMenu(playerid);
forward OnPlayerEnterPauseSubmenu(playerid, from, to);

stock bool:IsPlayerInPauseMenu(playerid) {
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return EXTGAME::PlayerInPauseMenu[playerid];
}

/*
 * =============================================================================
 * ASCM-Compatible HUD Colour Functions
 * =============================================================================
 */
stock SetPlayerHUDComponentColor(playerid, componentid, colour) {
    new hudComp = 0;
    switch (componentid) {
        case HUD_COLOUR_MONEY:          hudComp = HUD_COMPONENT_MONEY;
        case HUD_COLOUR_ARMOUR:         hudComp = HUD_COMPONENT_ARMOUR;
        case HUD_COLOUR_HEALTH:         hudComp = HUD_COMPONENT_HEALTH;
        case HUD_COLOUR_BREATH:         hudComp = HUD_COMPONENT_OXYGEN;
        case HUD_COLOUR_AMMO:           hudComp = HUD_COMPONENT_AMMO;
        case HUD_COLOUR_WANTED:         hudComp = HUD_COMPONENT_WANTED;
        case HUD_COLOUR_RADIO:          hudComp = HUD_COMPONENT_RADIO;
        case HUD_COLOUR_RADIO_TUNING:   hudComp = HUD_COMPONENT_RADIO_TUNING;
        case HUD_COLOUR_CLOCK:          hudComp = HUD_COMPONENT_CLOCK;
        default: return 0;
    }
    return SetPlayerHUDComponentsColour(playerid, hudComp, colour);
}

stock GetPlayerHUDComponentColor(playerid, componentid) {
    new hudComp = 0;
    switch (componentid) {
        case HUD_COLOUR_MONEY:          hudComp = HUD_COMPONENT_MONEY;
        case HUD_COLOUR_ARMOUR:         hudComp = HUD_COMPONENT_ARMOUR;
        case HUD_COLOUR_HEALTH:         hudComp = HUD_COMPONENT_HEALTH;
        case HUD_COLOUR_BREATH:         hudComp = HUD_COMPONENT_OXYGEN;
        case HUD_COLOUR_AMMO:           hudComp = HUD_COMPONENT_AMMO;
        case HUD_COLOUR_WANTED:         hudComp = HUD_COMPONENT_WANTED;
        case HUD_COLOUR_RADIO:          hudComp = HUD_COMPONENT_RADIO;
        case HUD_COLOUR_RADIO_TUNING:   hudComp = HUD_COMPONENT_RADIO_TUNING;
        case HUD_COLOUR_CLOCK:          hudComp = HUD_COMPONENT_CLOCK;
        default: return 0;
    }
    return GetPlayerHUDComponentColour(playerid, hudComp);
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
    if (!EXTGAME::PlayerHasPlugin[playerid]) {
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
        if (EXTGAME::PlayerCJActive[playerid]) {
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
    EXTGAME::PlayerHasPlugin[playerid] = false;
    EXTGAME::PlayerPluginVersion[playerid] = 0;
    EXTGAME::PlayerPluginChecked[playerid] = false;

    EXTGAME::PlayerAircraftHeightLimit[playerid] = EXTGAME::GlobalAircraftHeightLimit;
    EXTGAME::PlayerJetpackHeightLimit[playerid]  = EXTGAME::GlobalJetpackHeightLimit;
    EXTGAME::PlayerJetpackOffsetX[playerid]      = EXTGAME::GlobalJetpackOffsetX;
    EXTGAME::PlayerJetpackOffsetY[playerid]      = EXTGAME::GlobalJetpackOffsetY;
    EXTGAME::PlayerJetpackOffsetZ[playerid]      = EXTGAME::GlobalJetpackOffsetZ;
    EXTGAME::PlayerJetpackRotAngle[playerid]     = EXTGAME::GlobalJetpackRotAngle;
    EXTGAME::PlayerJetpackRotAxisX[playerid]     = EXTGAME::GlobalJetpackRotAxisX;
    EXTGAME::PlayerJetpackRotAxisY[playerid]     = EXTGAME::GlobalJetpackRotAxisY;
    EXTGAME::PlayerJetpackRotAxisZ[playerid]     = EXTGAME::GlobalJetpackRotAxisZ;
    EXTGAME::PlayerKnockOffBike[playerid]        = EXTGAME::GlobalKnockOffBike;
    EXTGAME::PlayerFireProof[playerid]           = EXTGAME::GlobalFireProof;
    EXTGAME::PlayerInfiniteSprint[playerid]      = EXTGAME::GlobalInfiniteSprint;
    EXTGAME::PlayerSunEnabled[playerid]          = EXTGAME::GlobalSunEnabled;

    EXTGAME::PlayerMoonEnabled[playerid]         = EXTGAME::GlobalMoonEnabled;
    EXTGAME::PlayerStarsEnabled[playerid]        = EXTGAME::GlobalStarsEnabled;
    EXTGAME::PlayerLowCloudsEnabled[playerid]    = EXTGAME::GlobalLowCloudsEnabled;
    EXTGAME::PlayerFluffyCloudsEnabled[playerid] = EXTGAME::GlobalFluffyCloudsEnabled;
    EXTGAME::PlayerRainbowEnabled[playerid]      = EXTGAME::GlobalRainbowEnabled;
    EXTGAME::PlayerWaterLevelSet[playerid]       = EXTGAME::GlobalWaterLevelSet;
    EXTGAME::PlayerWaterLevel[playerid]          = EXTGAME::GlobalWaterLevel;
    EXTGAME::PlayerWaterIncludeNonSea[playerid]  = EXTGAME::GlobalWaterIncludeNonSea;
    EXTGAME::PlayerWaterIncludeSea[playerid]     = EXTGAME::GlobalWaterIncludeSea;
    EXTGAME::PlayerWaterIncludeOutside[playerid] = EXTGAME::GlobalWaterIncludeOutside;
    EXTGAME::PlayerWaterWaveHeight[playerid]     = EXTGAME::GlobalWaterWaveHeight;
    EXTGAME::PlayerHUDComponents[playerid]       = EXTGAME::GlobalHUDComponents;
    EXTGAME::PlayerHUDColourAmmo[playerid]       = EXTGAME::GlobalHUDColours[0];
    EXTGAME::PlayerHUDColourWeaponIcon[playerid] = EXTGAME::GlobalHUDColours[1];
    EXTGAME::PlayerHUDColourHealth[playerid]      = EXTGAME::GlobalHUDColours[2];
    EXTGAME::PlayerHUDColourOxygen[playerid]      = EXTGAME::GlobalHUDColours[3];
    EXTGAME::PlayerHUDColourArmour[playerid]      = EXTGAME::GlobalHUDColours[4];
    EXTGAME::PlayerHUDColourRadar[playerid]       = EXTGAME::GlobalHUDColours[5];
    EXTGAME::PlayerHUDColourCrosshair[playerid]   = EXTGAME::GlobalHUDColours[6];
    EXTGAME::PlayerHUDColourMoney[playerid]       = EXTGAME::GlobalHUDColours[7];
    EXTGAME::PlayerHUDColourLaserDot[playerid]    = EXTGAME::GlobalHUDColours[8];
    EXTGAME::PlayerHUDColourWanted[playerid]      = EXTGAME::GlobalHUDColours[9];
    EXTGAME::PlayerHUDColourRadio[playerid]       = EXTGAME::GlobalHUDColours[10];
    EXTGAME::PlayerHUDColourRadioTuning[playerid] = EXTGAME::GlobalHUDColours[11];
    EXTGAME::PlayerHUDColourClock[playerid]       = EXTGAME::GlobalHUDColours[12];

    EXTGAME::PlayerNightVision[playerid]          = EXTGAME::GlobalNightVision;
    EXTGAME::PlayerThermalVision[playerid]        = EXTGAME::GlobalThermalVision;
    EXTGAME::PlayerNoReload[playerid]             = EXTGAME::GlobalNoReload;
    EXTGAME::PlayerSwitchReload[playerid]         = EXTGAME::GlobalSwitchReload;
    EXTGAME::PlayerQuickReload[playerid]          = EXTGAME::GlobalQuickReload;
    EXTGAME::PlayerInfiniteOxygen[playerid]       = EXTGAME::GlobalInfiniteOxygen;
    EXTGAME::PlayerPedAnims[playerid]             = 0;
    EXTGAME::PlayerCursor[playerid]               = false;
    EXTGAME::PlayerPlaneRadarColor[playerid]      = EXTGAME::GlobalPlaneRadarColor;
    EXTGAME::PlayerSunColorSet[playerid]          = EXTGAME::GlobalSunColorSet;
    EXTGAME::PlayerSunCoreColor[playerid]         = EXTGAME::GlobalSunCoreColor;
    EXTGAME::PlayerSunCoronaColor[playerid]       = EXTGAME::GlobalSunCoronaColor;
    EXTGAME::PlayerMoonColorSet[playerid]         = EXTGAME::GlobalMoonColorSet;
    EXTGAME::PlayerMoonColor[playerid]            = EXTGAME::GlobalMoonColor;
    EXTGAME::PlayerVehicleBlur[playerid]          = EXTGAME::GlobalVehicleBlur;
    EXTGAME::PlayerVehicleBlurIntensity[playerid] = EXTGAME::GlobalVehicleBlurIntensity;
    EXTGAME::PlayerInPauseMenu[playerid]          = false;

    BroadcastClearPlayerCJCache(playerid);

    for (new _ptd = 0; _ptd < MAX_PLAYER_TEXT_DRAWS; _ptd++) {
        EXTGAME::PlayerTextDrawRotation[playerid][_ptd][tdRotEnabled] = false;
        EXTGAME::PlayerTextDrawRotation[playerid][_ptd][tdRotAngle] = 0.0;
        EXTGAME::PlayerTextDrawRotation[playerid][_ptd][tdRotPivotX] = -1.0;
        EXTGAME::PlayerTextDrawRotation[playerid][_ptd][tdRotPivotY] = -1.0;
        EXTGAME::PlayerTextDrawRotation[playerid][_ptd][tdRotSpinSpeed] = 0.0;
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
                new bool:wasInstalled = EXTGAME::PlayerHasPlugin[playerid];
                EXTGAME::PlayerHasPlugin[playerid] = true;
                EXTGAME::PlayerPluginVersion[playerid] = version;
                EXTGAME::PlayerPluginChecked[playerid] = true;

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
                    SetTimerEx("_DeferredSyncPlayerExtendedGame", 50, false, "i", playerid);

                    CallLocalFunction("OnPlayerExtendedGameCheck", "iii", playerid, true, version);
                }
            }
            return 0; // Handled, discard packet
        } else if (actionid == EXTENDEDGAME_ACTION_PAUSE_MENU_EVENT) {
            new eventType, fromPage, toPage;
            BS_ReadValue(bs,
                PR_UINT8, eventType,
                PR_UINT8, fromPage,
                PR_UINT8, toPage
            );
            if (eventType == PAUSE_MENU_EVENT_OPEN) {
                EXTGAME::PlayerInPauseMenu[playerid] = true;
                CallLocalFunction("OnPlayerOpenPauseMenu", "i", playerid);
            } else if (eventType == PAUSE_MENU_EVENT_CLOSE) {
                EXTGAME::PlayerInPauseMenu[playerid] = false;
                CallLocalFunction("OnPlayerClosePauseMenu", "i", playerid);
            } else if (eventType == PAUSE_MENU_EVENT_SUBMENU) {
                CallLocalFunction("OnPlayerEnterPauseSubmenu", "iii", playerid, fromPage, toPage);
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
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid) || !EXTGAME::PlayerHasPlugin[playerid])
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
    if (IsPlayerConnected(playerid) && !EXTGAME::PlayerPluginChecked[playerid]) {
        EXTGAME::PlayerPluginChecked[playerid] = true;
        EXTGAME::PlayerHasPlugin[playerid] = false;
        CallLocalFunction("OnPlayerExtendedGameCheck", "iii", playerid, false, 0);
    }
    return 1;
}

static stock _InitPlayerExtendedGame(playerid) {
    ResetPlayerCJData(playerid);
    EXTGAME::PlayerHasPlugin[playerid] = false;
    EXTGAME::PlayerPluginVersion[playerid] = 0;
    EXTGAME::PlayerPluginChecked[playerid] = false;

    EXTGAME::PlayerAircraftHeightLimit[playerid] = EXTGAME::GlobalAircraftHeightLimit;
    EXTGAME::PlayerJetpackHeightLimit[playerid]  = EXTGAME::GlobalJetpackHeightLimit;
    EXTGAME::PlayerJetpackOffsetX[playerid]      = EXTGAME::GlobalJetpackOffsetX;
    EXTGAME::PlayerJetpackOffsetY[playerid]      = EXTGAME::GlobalJetpackOffsetY;
    EXTGAME::PlayerJetpackOffsetZ[playerid]      = EXTGAME::GlobalJetpackOffsetZ;
    EXTGAME::PlayerJetpackRotAngle[playerid]     = EXTGAME::GlobalJetpackRotAngle;
    EXTGAME::PlayerJetpackRotAxisX[playerid]     = EXTGAME::GlobalJetpackRotAxisX;
    EXTGAME::PlayerJetpackRotAxisY[playerid]     = EXTGAME::GlobalJetpackRotAxisY;
    EXTGAME::PlayerJetpackRotAxisZ[playerid]     = EXTGAME::GlobalJetpackRotAxisZ;
    EXTGAME::PlayerKnockOffBike[playerid]        = EXTGAME::GlobalKnockOffBike;
    EXTGAME::PlayerFireProof[playerid]           = EXTGAME::GlobalFireProof;
    EXTGAME::PlayerInfiniteSprint[playerid]      = EXTGAME::GlobalInfiniteSprint;
    EXTGAME::PlayerSunEnabled[playerid]          = EXTGAME::GlobalSunEnabled;

    EXTGAME::PlayerMoonEnabled[playerid]         = EXTGAME::GlobalMoonEnabled;
    EXTGAME::PlayerStarsEnabled[playerid]        = EXTGAME::GlobalStarsEnabled;
    EXTGAME::PlayerLowCloudsEnabled[playerid]    = EXTGAME::GlobalLowCloudsEnabled;
    EXTGAME::PlayerFluffyCloudsEnabled[playerid] = EXTGAME::GlobalFluffyCloudsEnabled;
    EXTGAME::PlayerRainbowEnabled[playerid]      = EXTGAME::GlobalRainbowEnabled;
    EXTGAME::PlayerWaterLevelSet[playerid]       = EXTGAME::GlobalWaterLevelSet;
    EXTGAME::PlayerWaterLevel[playerid]          = EXTGAME::GlobalWaterLevel;
    EXTGAME::PlayerWaterIncludeNonSea[playerid]  = EXTGAME::GlobalWaterIncludeNonSea;
    EXTGAME::PlayerWaterIncludeSea[playerid]     = EXTGAME::GlobalWaterIncludeSea;
    EXTGAME::PlayerWaterIncludeOutside[playerid] = EXTGAME::GlobalWaterIncludeOutside;
    EXTGAME::PlayerWaterWaveHeight[playerid]     = EXTGAME::GlobalWaterWaveHeight;
    EXTGAME::PlayerHUDComponents[playerid]       = EXTGAME::GlobalHUDComponents;
    EXTGAME::PlayerHUDColourAmmo[playerid]       = EXTGAME::GlobalHUDColours[0];
    EXTGAME::PlayerHUDColourWeaponIcon[playerid] = EXTGAME::GlobalHUDColours[1];
    EXTGAME::PlayerHUDColourHealth[playerid]     = EXTGAME::GlobalHUDColours[2];
    EXTGAME::PlayerHUDColourOxygen[playerid]      = EXTGAME::GlobalHUDColours[3];
    EXTGAME::PlayerHUDColourArmour[playerid]      = EXTGAME::GlobalHUDColours[4];
    EXTGAME::PlayerHUDColourRadar[playerid]       = EXTGAME::GlobalHUDColours[5];
    EXTGAME::PlayerHUDColourCrosshair[playerid]   = EXTGAME::GlobalHUDColours[6];
    EXTGAME::PlayerHUDColourMoney[playerid]       = EXTGAME::GlobalHUDColours[7];
    EXTGAME::PlayerHUDColourLaserDot[playerid]    = EXTGAME::GlobalHUDColours[8];
    EXTGAME::PlayerHUDColourWanted[playerid]      = EXTGAME::GlobalHUDColours[9];
    EXTGAME::PlayerHUDColourRadio[playerid]       = EXTGAME::GlobalHUDColours[10];
    EXTGAME::PlayerHUDColourRadioTuning[playerid] = EXTGAME::GlobalHUDColours[11];
    EXTGAME::PlayerHUDColourClock[playerid]       = EXTGAME::GlobalHUDColours[12];

    EXTGAME::PlayerNightVision[playerid]          = EXTGAME::GlobalNightVision;
    EXTGAME::PlayerThermalVision[playerid]        = EXTGAME::GlobalThermalVision;
    EXTGAME::PlayerNoReload[playerid]             = EXTGAME::GlobalNoReload;
    EXTGAME::PlayerSwitchReload[playerid]         = EXTGAME::GlobalSwitchReload;
    EXTGAME::PlayerQuickReload[playerid]          = EXTGAME::GlobalQuickReload;
    EXTGAME::PlayerInfiniteOxygen[playerid]       = EXTGAME::GlobalInfiniteOxygen;
    EXTGAME::PlayerPedAnims[playerid]             = 0;
    EXTGAME::PlayerCursor[playerid]               = false;
    EXTGAME::PlayerPlaneRadarColor[playerid]      = EXTGAME::GlobalPlaneRadarColor;
    EXTGAME::PlayerSunColorSet[playerid]          = EXTGAME::GlobalSunColorSet;
    EXTGAME::PlayerSunCoreColor[playerid]         = EXTGAME::GlobalSunCoreColor;
    EXTGAME::PlayerSunCoronaColor[playerid]       = EXTGAME::GlobalSunCoronaColor;
    EXTGAME::PlayerMoonColorSet[playerid]         = EXTGAME::GlobalMoonColorSet;
    EXTGAME::PlayerMoonColor[playerid]            = EXTGAME::GlobalMoonColor;
    EXTGAME::PlayerVehicleBlur[playerid]          = EXTGAME::GlobalVehicleBlur;
    EXTGAME::PlayerVehicleBlurIntensity[playerid] = EXTGAME::GlobalVehicleBlurIntensity;
    EXTGAME::PlayerInPauseMenu[playerid]          = false;

    // Send handshake ping to detect ASI plugin presence
    CheckPlayerExtendedGame(playerid, 0);
    SetTimerEx("EXTG_OnPlayerPluginTimeout", 6000, false, "i", playerid);
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
    if (_:text >= 0 && _:text < MAX_TEXT_DRAWS && EXTGAME::GlobalTextDrawRotation[_:text][tdRotEnabled]) {
        SetTextDrawRotationForPlayer(playerid, text,
            EXTGAME::GlobalTextDrawRotation[_:text][tdRotAngle],
            EXTGAME::GlobalTextDrawRotation[_:text][tdRotPivotX],
            EXTGAME::GlobalTextDrawRotation[_:text][tdRotPivotY],
            EXTGAME::GlobalTextDrawRotation[_:text][tdRotSpinSpeed]
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
    if (_:text >= 0 && _:text < MAX_TEXT_DRAWS && EXTGAME::GlobalTextDrawRotation[_:text][tdRotEnabled]) {
        for (new i = 0; i < MAX_PLAYERS; i++) {
            if (IsPlayerConnected(i)) {
                SetTextDrawRotationForPlayer(i, text,
                    EXTGAME::GlobalTextDrawRotation[_:text][tdRotAngle],
                    EXTGAME::GlobalTextDrawRotation[_:text][tdRotPivotX],
                    EXTGAME::GlobalTextDrawRotation[_:text][tdRotPivotY],
                    EXTGAME::GlobalTextDrawRotation[_:text][tdRotSpinSpeed]
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
        if (EXTGAME::PlayerTextDrawRotation[playerid][_:text][tdRotEnabled]) {
            SetPlayerTextDrawRotation(playerid, text,
                EXTGAME::PlayerTextDrawRotation[playerid][_:text][tdRotAngle],
                EXTGAME::PlayerTextDrawRotation[playerid][_:text][tdRotPivotX],
                EXTGAME::PlayerTextDrawRotation[playerid][_:text][tdRotPivotY],
                EXTGAME::PlayerTextDrawRotation[playerid][_:text][tdRotSpinSpeed]
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
