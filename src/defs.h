#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Custom RakNet packet identifier
constexpr uint8_t EXTENDEDGAME_PACKET_ID        = 252;

// Protocol Magic & Version (v0.0.1 Alpha)
constexpr uint32_t EXTENDEDGAME_HANDSHAKE_MAGIC = 0x42544547; // 'BTEG' (BrownTurbo ExtendedGame)
constexpr uint16_t EXTENDEDGAME_PLUGIN_VERSION  = 0x0001;     // v0.0.1 (Alpha)

// Common SA-MP RPC Identifiers
constexpr uint16_t RPC_WorldPlayerAdd           = 137;
constexpr uint16_t RPC_WorldPlayerRemove        = 138;
constexpr uint16_t RPC_InitGame                 = 139;
constexpr uint16_t RPC_GameModeRestart          = 142;
constexpr uint16_t RPC_ServerQuit               = 166;
constexpr uint16_t RPC_Spawn                    = 52;
constexpr uint16_t RPC_ServerCommand            = 50;
constexpr uint16_t RPC_RequestClass             = 128;
constexpr uint16_t RPC_RequestSpawn             = 129;

// Special Player ID for Local Player
constexpr uint16_t CJ_LOCAL_PLAYER_ID           = 0xFFFF;

// Actions that can be sent in Packet 252
enum eExtendedGameAction : uint8_t {
	// Actions 1..13: Singleplayer CJ Skin & Clothing
	CJ_ACTION_SET_CLOTHES       = 1, // Set clothes/tattoo in a slot by string names
	CJ_ACTION_REMOVE_CLOTHES    = 2, // Remove clothes/tattoo from a slot
	CJ_ACTION_SET_BODY_STATS    = 3, // Set fat and muscle stats (0.0 to 1000.0)
	CJ_ACTION_RESET_CLOTHES     = 4, // Reset to default CJ clothes or empty
	CJ_ACTION_REBUILD           = 5, // Force rebuild player model
	CJ_ACTION_SET_FULL_OUTFIT   = 6, // Set all clothes items and body stats in one packet
	CJ_ACTION_SET_CLOTHES_HASH  = 7, // Set clothes/tattoo by CRC32 hash keys
	CJ_ACTION_SET_TATTOO        = 8, // Set a tattoo in one of the tattoo slots
	CJ_ACTION_CLEAR_CACHE       = 9, // Clear cached skin state on client
	CJ_ACTION_REQUEST_SYNC      = 10,// Client requests full outfit for a target player from server
	CJ_ACTION_HANDSHAKE_PING    = 11,// Server pings client to check if ASI plugin is installed
	CJ_ACTION_HANDSHAKE_PONG    = 12,// Client responds confirming plugin is installed & version
	CJ_ACTION_HANDSHAKE_ACK     = 13,// Server acknowledges handshake; confirms communication channel active

	// Actions 20..25: Parity Gameplay & Environment Controls
	EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT = 20, // float heightLimit
	EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT  = 21, // float heightLimit
	EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF        = 22, // uint8_t knockState (0=default, 1=never, 2=always)
	EXTENDEDGAME_ACTION_SET_FIRE_PROOF            = 23, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT       = 24, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_SET_SUN                   = 25, // uint8_t enable (0 or 1)
	// Actions 26 and 27 are reserved.

	// Actions 28..32: Sky & Environmental Toggles
	EXTENDEDGAME_ACTION_TOGGLE_MOON               = 28, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_TOGGLE_STARS              = 29, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_TOGGLE_LOW_CLOUDS         = 30, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_TOGGLE_FLUFFY_CLOUDS      = 31, // uint8_t enable (0 or 1)
	EXTENDEDGAME_ACTION_TOGGLE_RAINBOW            = 32, // uint8_t enable (0 or 1)

	// Actions 33..37: HUD Components Toggles & Coloring
	EXTENDEDGAME_ACTION_TOGGLE_HUD_COMPONENTS     = 33, // uint32_t hudComponents
	EXTENDEDGAME_ACTION_SET_HUD_COMPONENTS_COLOUR = 34, // uint32_t hudComponents, uint32_t intColour
	EXTENDEDGAME_ACTION_SET_WATER_LEVEL           = 35, // float level, uint8 flags (non-sea, sea, outside)
	EXTENDEDGAME_ACTION_SET_WATER_WAVE_HEIGHT     = 36, // float intensity percent (-1.0 or 0.0 to 100.0)
	EXTENDEDGAME_ACTION_RESET_WATER_LEVEL         = 37, // no payload

	// Actions 38..39: TextDraw Rotation & Spin
	EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION     = 38, // uint16_t textDrawId, uint8_t isPlayerTextDraw, float angle, float pivotX, float pivotY, float spinSpeed
	EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION   = 39, // uint16_t textDrawId, uint8_t isPlayerTextDraw

	// Actions 40..43: Custom Water Zones
	EXTENDEDGAME_ACTION_CREATE_WATER_ZONE         = 40, // uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ
	EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL      = 41, // uint16_t zoneId, float level
	EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS     = 42, // uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ
	EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE        = 43, // uint16_t zoneId

	// Actions 44..45: Jetpack Placement & Orientation
	EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET        = 44, // float posX, float posY, float posZ, float rotAngle, float rotAxisX, float rotAxisY, float rotAxisZ
	EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET      = 45, // reset to default offset and rotation
};

// HUD Components Bitmask
constexpr uint32_t HUD_COMPONENT_AMMO         = 1;
constexpr uint32_t HUD_COMPONENT_WEAPONICON   = 2;
constexpr uint32_t HUD_COMPONENT_HEALTH       = 4;
constexpr uint32_t HUD_COMPONENT_OXYGEN       = 8;
constexpr uint32_t HUD_COMPONENT_ARMOUR       = 16;
constexpr uint32_t HUD_COMPONENT_RADAR        = 32;
constexpr uint32_t HUD_COMPONENT_CROSSHAIR    = 64;
constexpr uint32_t HUD_COMPONENT_MONEY        = 128;
constexpr uint32_t HUD_COMPONENT_LASERDOT     = 256;
constexpr uint32_t HUD_COMPONENT_ALL          = 0x1FF; // 511

// Bike Knock Off States (CPed::CantBeKnockedOffBike)
constexpr uint8_t BIKE_KNOCK_OFF_DEFAULT = 0; // Default GTA:SA behavior
constexpr uint8_t BIKE_KNOCK_OFF_NEVER   = 1; // Rider never falls off bike
constexpr uint8_t BIKE_KNOCK_OFF_ALWAYS  = 2; // Rider always falls off bike on impact

// GTA:SA Clothes Texture Parts (Indices 0 to 18)
enum eCJSkinTextureSlot : uint8_t {
	CJ_SLOT_TORSO             = 0,  // Shirts, t-shirts, jackets, hoodies, vests
	CJ_SLOT_HEAD              = 1,  // Haircuts, bald, masks
	CJ_SLOT_LEGS              = 2,  // Jeans, trackpants, shorts
	CJ_SLOT_SHOES             = 3,  // Sneakers, boots, sandals
	CJ_SLOT_TATTOO_LOWER_LEFT = 4,  // Lower left arm tattoo
	CJ_SLOT_TATTOO_UPPER_LEFT = 5,  // Upper left arm tattoo
	CJ_SLOT_TATTOO_UPPER_RIGHT= 6,  // Upper right arm tattoo
	CJ_SLOT_TATTOO_LOWER_RIGHT= 7,  // Lower right arm tattoo
	CJ_SLOT_TATTOO_BACK_TOP   = 8,  // Upper back tattoo
	CJ_SLOT_TATTOO_LEFT_CHEST = 9,  // Left chest tattoo
	CJ_SLOT_TATTOO_RIGHT_CHEST= 10, // Right chest tattoo
	CJ_SLOT_TATTOO_STOMACH    = 11, // Stomach tattoo
	CJ_SLOT_TATTOO_LOWER_BACK = 12, // Lower back tattoo
	CJ_SLOT_NECKLACE          = 13, // Chains, necklaces, bandanas
	CJ_SLOT_WATCH             = 14, // Watches, bracelets
	CJ_SLOT_GLASSES           = 15, // Sunglasses, shades, glasses
	CJ_SLOT_HATS              = 16, // Baseball caps, helmets, beanies, berets
	CJ_SLOT_SPECIAL           = 17, // Special outfits & suits (cop, valet, gimp, etc.)
	CJ_SLOT_EXTRA             = 17, // Alias for special items
	CJ_SLOT_SUIT              = 17, // Full-body special outfits & suits (slot 17)
	CJ_SLOT_COUNT             = 18
};

// Represents a single clothes/tattoo item
struct CJSkinItem {
	uint8_t slot = 0;
	std::string textureName;
	std::string modelName;
	uint32_t textureHash = 0;
	uint32_t modelHash = 0;
	std::string model;
	std::string texture;
	bool active = false;
};

// Full state of a CJ skin
struct CJSkinState {
	bool hasCustomSkin = false;
	float fat = 0.0f;
	float muscle = 0.0f;
	uint32_t textureKeys[CJ_SLOT_COUNT] = { 0 };
	uint32_t modelKeys[10] = { 0 };
	uint32_t version = 1;
	uint32_t appliedVersion = 0;
	void* lastPed = nullptr;
};

// Complete CJ outfit including all slots and body fat/muscle stats
struct CJSkinOutfit {
	CJSkinItem items[CJ_SLOT_COUNT];
	float fat    = 0.0f; // 0.0 to 1000.0 (GTA:SA stat 21)
	float muscle = 0.0f; // 0.0 to 1000.0 (GTA:SA stat 23)
	bool custom  = false;// Has this player ever received custom clothes data?
};

// State tracker for a player's CJ outfit
struct CJSyncState {
	uint16_t playerId = 0;
	CJSkinOutfit currentOutfit;
	CJSkinOutfit targetOutfit;
	bool dirty = false;
	uint32_t lastRebuildTime = 0;
};

// Known item definition lookup structure
struct CJItemDef {
	const char* name;
	const char* model;
	const char* texture;
	uint8_t slot;
};

// Static table of standard CJ clothing items
constexpr CJItemDef CJ_ALL_ITEMS[] = {
	// Torso (Slot 0)
	{ "White Tank",       "vest",          "vest",          CJ_SLOT_TORSO },
	{ "Black Tank",       "vestblack",     "vestblack",     CJ_SLOT_TORSO },
	{ "Green Hoody",      "hoodya",        "hoodyaGreen",   CJ_SLOT_TORSO },
	{ "Black Hoody",      "hoodya",        "hoodyaBlack",   CJ_SLOT_TORSO },
	{ "Blue Hoody",       "hoodya",        "hoodyaBlue",    CJ_SLOT_TORSO },
	{ "Brown Shirt",      "shirtb",        "shirtbBrown",   CJ_SLOT_TORSO },
	{ "Checkered Shirt",  "shirtb",        "shirtbCheck",   CJ_SLOT_TORSO },
	{ "Blue Shirt",       "shirtb",        "shirtbBlue",    CJ_SLOT_TORSO },
	{ "Red T-Shirt",      "tshirt",        "tshirtRed",     CJ_SLOT_TORSO },
	{ "Green T-Shirt",    "tshirt",        "tshirtGreen",   CJ_SLOT_TORSO },
	{ "White T-Shirt",    "tshirt",        "tshirtWhite",   CJ_SLOT_TORSO },
	{ "Black T-Shirt",    "tshirt",        "tshirtBlack",   CJ_SLOT_TORSO },
	{ "Leather Jacket",   "leather",       "leather",       CJ_SLOT_TORSO },
	{ "Tuxedo",           "suit1",         "suit1tr",       CJ_SLOT_TORSO },
	{ "Track Top Green",  "tracktr",       "tracktrGreen",  CJ_SLOT_TORSO },
	{ "Track Top Blue",   "tracktr",       "tracktrBlue",   CJ_SLOT_TORSO },
	{ "Sweat Top",        "sweat",         "sweat",         CJ_SLOT_TORSO },
	{ "Denim Jacket",     "denim",         "denim",         CJ_SLOT_TORSO },
	{ "Biker Vest",       "biker",         "biker",         CJ_SLOT_TORSO },
	{ "Hawaiian Shirt",   "hawaii",        "hawaii",        CJ_SLOT_TORSO },
	// Legs (Slot 2)
	{ "Blue Jeans",       "jeans",         "jeansdenim",    CJ_SLOT_LEGS },
	{ "Green Trackpants", "tracktr",       "tracktrgrn",    CJ_SLOT_LEGS },
	{ "Blue Trackpants",  "tracktr",       "tracktrblue",   CJ_SLOT_LEGS },
	{ "Black Boxers",     "boxers",        "boxers",        CJ_SLOT_LEGS },
	{ "Heart Boxers",     "boxers",        "boxersheart",   CJ_SLOT_LEGS },
	{ "Khaki Pants",      "chinos",        "chinoskhaki",   CJ_SLOT_LEGS },
	{ "Green Shorts",     "shorts",        "shortsgrn",     CJ_SLOT_LEGS },
	{ "Grey Shorts",      "shorts",        "shortsgrey",    CJ_SLOT_LEGS },
	{ "Denim Shorts",     "shorts",        "shortsdenim",   CJ_SLOT_LEGS },
	{ "Beige Pants",      "suit1tr",       "suit1trbeige",  CJ_SLOT_LEGS },
	{ "Black Pants",      "suit1tr",       "suit1trblk",    CJ_SLOT_LEGS },
	{ "Blue Pants",       "suit1tr",       "suit1trblue",   CJ_SLOT_LEGS },
	{ "Camouflage Pants", "camo",          "camogrn",       CJ_SLOT_LEGS },
	// Shoes (Slot 3)
	{ "Sneakers",         "sneaker",       "sneakerbincblk",CJ_SLOT_SHOES },
	{ "Blue Sneakers",    "sneaker",       "sneakerbincblue",CJ_SLOT_SHOES },
	{ "Black Hi-Tops",    "bask1",         "bask1blk",      CJ_SLOT_SHOES },
	{ "White Hi-Tops",    "bask1",         "bask1wht",      CJ_SLOT_SHOES },
	{ "Blue Hi-Tops",     "bask1",         "bask1blue",     CJ_SLOT_SHOES },
	{ "Cowboy Boots",     "cowboy",        "cowboy",        CJ_SLOT_SHOES },
	{ "Biker Boots",      "biker",         "bikerboot",     CJ_SLOT_SHOES },
	{ "Sandals",          "sandal",        "sandal",        CJ_SLOT_SHOES },
	{ "Flip-Flops",       "flipflop",      "flipflop",      CJ_SLOT_SHOES },
	// Head / Hair (Slot 1)
	{ "Bald",             "bald",          "bald",          CJ_SLOT_HEAD },
	{ "Afro",             "afro",          "afro",          CJ_SLOT_HEAD },
	{ "Cornrows",         "cornrows",      "cornrows",      CJ_SLOT_HEAD },
	{ "High Fade",        "fade",          "fade",          CJ_SLOT_HEAD },
	{ "Caesar",           "caesar",        "caesar",        CJ_SLOT_HEAD },
	{ "Wedge",            "wedge",         "wedge",         CJ_SLOT_HEAD },
	{ "Jheri Curl",       "jheri",         "jheri",         CJ_SLOT_HEAD },
	// Hats / Headwear (Slot 16)
	{ "Black Bandana",    "bandana",       "bandblack",     CJ_SLOT_HATS },
	{ "Green Bandana",    "bandana",       "bandgreen",     CJ_SLOT_HATS },
	{ "Blue Bandana",     "bandana",       "bandblue",      CJ_SLOT_HATS },
	{ "Red Bandana",      "bandana",       "bandred",       CJ_SLOT_HATS },
	{ "Green Cap Back",   "capback",       "capbackgrn",    CJ_SLOT_HATS },
	{ "Black Cap Back",   "capback",       "capbackblk",    CJ_SLOT_HATS },
	{ "Green Cap Front",  "cap",           "capgrn",        CJ_SLOT_HATS },
	{ "Black Cap Front",  "cap",           "capblk",        CJ_SLOT_HATS },
	{ "Motorcycle Helmet","helmet",        "helmet",        CJ_SLOT_HATS },
	{ "Full-Face Helmet", "moto",          "moto",          CJ_SLOT_HATS },
	{ "Boxing Helmet",    "boxingt",       "boxingt",       CJ_SLOT_HATS },
	// Glasses (Slot 15)
	{ "Aviator Shades",   "glasses01",     "glasses01dark", CJ_SLOT_GLASSES },
	{ "Sun Glasses",      "glasses02",     "glasses02dark", CJ_SLOT_GLASSES },
	{ "Square Glasses",   "glasses03",     "glasses03dark", CJ_SLOT_GLASSES },
	{ "Eye Patch",        "eyepatch",      "eyepatch",      CJ_SLOT_GLASSES },
	// Watches (Slot 14)
	{ "Gold Watch",       "watch",         "watchgold",     CJ_SLOT_WATCH },
	{ "Silver Watch",     "watch",         "watchsilver",   CJ_SLOT_WATCH },
	// Necklaces (Slot 13)
	{ "Silver Chain",     "neck",          "necksilver",    CJ_SLOT_NECKLACE },
	{ "Gold Chain",       "neck",          "neckgold",      CJ_SLOT_NECKLACE },
	{ "Dog Tags",         "neck",          "dogtag",        CJ_SLOT_NECKLACE },
	// Special Outfits & Suits (Slot 18)
	{ "Gimp Suit",        "gimp",          "gimp",          CJ_SLOT_SUIT },
	{ "Medic Uniform",    "medic",         "medic",         CJ_SLOT_SUIT },
	{ "Police Uniform",   "cop",           "cop",           CJ_SLOT_SUIT },
	{ "Valet Uniform",    "valet",         "valet",         CJ_SLOT_SUIT },
	{ "Croupier Uniform", "croupier",      "croupier",      CJ_SLOT_SUIT },
	{ "Country Outfit",   "country",       "country",       CJ_SLOT_SUIT },
};

constexpr size_t CJ_ALL_ITEMS_COUNT = sizeof(CJ_ALL_ITEMS) / sizeof(CJ_ALL_ITEMS[0]);

// Find item definition by name
inline const CJItemDef* FindCJItem(const char* name, uint8_t slot = 0xFF) {
	if (!name) return nullptr;
	for (size_t i = 0; i < CJ_ALL_ITEMS_COUNT; ++i) {
		if (slot != 0xFF && CJ_ALL_ITEMS[i].slot != slot) continue;
		if (_stricmp(CJ_ALL_ITEMS[i].name, name) == 0 ||
			_stricmp(CJ_ALL_ITEMS[i].texture, name) == 0) {
			return &CJ_ALL_ITEMS[i];
		}
	}
	return nullptr;
}

// Find item definition by hash keys
inline const CJItemDef* FindCJItemByHash(uint32_t modelHash, uint32_t txdHash, uint8_t slot = 0xFF) {
	for (size_t i = 0; i < CJ_ALL_ITEMS_COUNT; ++i) {
		if (slot != 0xFF && CJ_ALL_ITEMS[i].slot != slot) continue;
		if (modelHash != 0 && txdHash != 0) {
			return &CJ_ALL_ITEMS[i];
		}
	}
	return nullptr;
}
