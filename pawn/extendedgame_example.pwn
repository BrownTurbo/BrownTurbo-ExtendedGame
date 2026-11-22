// this example is generated using AI.

#define FILTERSCRIPT
#include <a_samp>
#include <Pawn.RakNet>
#include "extendedgame.inc"

static stock Text:g_DemoTextDraw = Text:INVALID_TEXT_DRAW;
static stock Text:g_DemoSpriteTD = Text:INVALID_TEXT_DRAW;
static stock PlayerText:g_DemoPlayerTextDraw[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};
static stock PlayerText:g_DemoPlayerSpriteTD[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};
static stock g_DemoWaterZone = INVALID_WATER_ZONE;

// Demo custom roads and road side detection tracking
#define MAX_DEMO_ROAD_OBJECTS (16)
static stock g_DemoRoadObjects[MAX_PLAYERS][MAX_DEMO_ROAD_OBJECTS];
static stock g_DemoRoadCount[MAX_PLAYERS] = {0, ...};
static stock bool:g_DemoRoadCheck[MAX_PLAYERS] = {false, ...};

static stock CleanupPlayerDemoRoads(playerid) {
    for (new i = 0; i < g_DemoRoadCount[playerid]; i++) {
        if (g_DemoRoadObjects[playerid][i] != INVALID_OBJECT_ID) {
            DestroyObject(g_DemoRoadObjects[playerid][i]);
            g_DemoRoadObjects[playerid][i] = INVALID_OBJECT_ID;
        }
    }
    g_DemoRoadCount[playerid] = 0;
    g_DemoRoadCheck[playerid] = false;
}

public OnFilterScriptInit() {
    PR_Init();
    for (new i = 0; i < MAX_PLAYERS; i++) {
        for (new j = 0; j < MAX_DEMO_ROAD_OBJECTS; j++) {
            g_DemoRoadObjects[i][j] = INVALID_OBJECT_ID;
        }
    }
    print("==========================================================");
    print("   BrownTurbo-ExtendedGame FS Loaded      ");
    print("==========================================================");
    return 1;
}

public OnFilterScriptExit() {
    if (g_DemoTextDraw != Text:INVALID_TEXT_DRAW) {
        TextDrawDestroy(g_DemoTextDraw);
        g_DemoTextDraw = Text:INVALID_TEXT_DRAW;
    }
    if (g_DemoSpriteTD != Text:INVALID_TEXT_DRAW) {
        TextDrawDestroy(g_DemoSpriteTD);
        g_DemoSpriteTD = Text:INVALID_TEXT_DRAW;
    }
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (g_DemoPlayerTextDraw[i] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(i, g_DemoPlayerTextDraw[i]);
            g_DemoPlayerTextDraw[i] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (g_DemoPlayerSpriteTD[i] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(i, g_DemoPlayerSpriteTD[i]);
            g_DemoPlayerSpriteTD[i] = PlayerText:INVALID_TEXT_DRAW;
        }
        CleanupPlayerDemoRoads(i);
    }
    if (g_DemoWaterZone != INVALID_WATER_ZONE) {
        DestroyWaterZone(g_DemoWaterZone);
        g_DemoWaterZone = INVALID_WATER_ZONE;
    }
    return 1;
}

public OnPlayerDisconnect(playerid, reason) {
    if (g_DemoPlayerTextDraw[playerid] != PlayerText:INVALID_TEXT_DRAW) {
        PlayerTextDrawDestroy(playerid, g_DemoPlayerTextDraw[playerid]);
        g_DemoPlayerTextDraw[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    if (g_DemoPlayerSpriteTD[playerid] != PlayerText:INVALID_TEXT_DRAW) {
        PlayerTextDrawDestroy(playerid, g_DemoPlayerSpriteTD[playerid]);
        g_DemoPlayerSpriteTD[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    CleanupPlayerDemoRoads(playerid);
    return 1;
}

// Callback triggered when plugin handshake resolves for a player
public OnPlayerExtendedGameCheck(playerid, bool:installed, version) {
    new msg[144];
    if (installed) {
        format(msg, sizeof(msg), "[ExtendedGame] BrownTurbo-ExtendedGame ASI detected! (v%d.%d.%d alpha)", (version >> 8) & 0xFF, (version >> 4) & 0x0F, version & 0x0F);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        SendClientMessage(playerid, 0x00FFCCAA, "  Type /eghelp for all ExtendedGame commands (GPS, Roads, Weather, etc.)");
    } else {
        SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Warning: BrownTurbo-ExtendedGame ASI is not installed!");
        SendClientMessage(playerid, 0xFFFF00AA, "  Please install brownturbo-extendedgame.asi into your GTA root for extended features.");
    }
    return 1;
}

public OnPlayerSpawn(playerid) {
    // When player spawns with skin 0 (CJ), broadcast default or saved clothes
    if (GetPlayerSkin(playerid) == 0) {
        // Reset player to default CJ clothes
        BroadcastResetPlayerCJClothes(playerid, true, true);
    }
    return 1;
}

// Simple parsing helpers for demo
stock sscanf_custom(const input[], cmd[], params[]) {
    new i = 0, j = 0;
    while (input[i] != '\0' && input[i] != ' ') {
        cmd[j++] = input[i++];
    }
    cmd[j] = '\0';
    while (input[i] == ' ') i++;
    j = 0;
    while (input[i] != '\0') {
        params[j++] = input[i++];
    }
    params[j] = '\0';
}

stock ParseColorHex(const str[]) {
    new i = 0;
    while (str[i] == ' ') i++;
    if (str[i] == '0' && (str[i+1] == 'x' || str[i+1] == 'X')) i += 2;
    else if (str[i] == '#') i++;
    new digits = 0, value = 0;
    while (str[i] != '\0' && str[i] != ' ') {
        new c = str[i++];
        if (c >= '0' && c <= '9') { value = (value << 4) | (c - '0'); digits++; }
        else if (c >= 'a' && c <= 'f') { value = (value << 4) | (c - 'a' + 10); digits++; }
        else if (c >= 'A' && c <= 'F') { value = (value << 4) | (c - 'A' + 10); digits++; }
        else break;
    }
    if (digits == 6) value = (value << 8) | 0xFF; // Auto append full alpha if 6 hex digits given
    return value;
}

stock bool:sscanf_two_strings(const params[], s1[], s2[]) {
    new i = 0, j = 0;
    while (params[i] == ' ') i++;
    if (params[i] == '\0') return false;
    while (params[i] != '\0' && params[i] != ' ') {
        s1[j++] = params[i++];
    }
    s1[j] = '\0';
    while (params[i] == ' ') i++;
    if (params[i] == '\0') return false;
    j = 0;
    while (params[i] != '\0' && params[i] != ' ') {
        s2[j++] = params[i++];
    }
    s2[j] = '\0';
    return true;
}

stock bool:sscanf_two_floats(const params[], &Float:f1, &Float:f2) {
    new s1[32], s2[32];
    if (!sscanf_two_strings(params, s1, s2)) return false;
    f1 = floatstr(s1);
    f2 = floatstr(s2);
    return true;
}

stock bool:sscanf_three_floats(const params[], &Float:f1, &Float:f2, &Float:f3) {
    new s1[32], s2[32], s3[32];
    new i = 0, j = 0;
    while (params[i] == ' ') i++;
    if (params[i] == '\0') return false;
    while (params[i] != '\0' && params[i] != ' ') s1[j++] = params[i++];
    s1[j] = '\0';
    while (params[i] == ' ') i++;
    if (params[i] == '\0') return false;
    j = 0;
    while (params[i] != '\0' && params[i] != ' ') s2[j++] = params[i++];
    s2[j] = '\0';
    while (params[i] == ' ') i++;
    if (params[i] == '\0') return false;
    j = 0;
    while (params[i] != '\0' && params[i] != ' ') s3[j++] = params[i++];
    s3[j] = '\0';
    f1 = floatstr(s1);
    f2 = floatstr(s2);
    f3 = floatstr(s3);
    return true;
}

stock bool:_ParseFloatStrict(const input[], &Float:value) {
    new i = 0, digits = 0, hasDecimal = 0;
    while (input[i] == ' ') i++;
    if (input[i] == '+' || input[i] == '-') i++;

    while (input[i] != '\0' && input[i] != ' ') {
        if (input[i] >= '0' && input[i] <= '9') {
            digits++;
        } else if (input[i] == '.' && !hasDecimal) {
            hasDecimal = 1;
        } else {
            return false;
        }
        i++;
    }
    while (input[i] == ' ') i++;
    if (digits == 0 || input[i] != '\0') return false;

    value = floatstr(input);
    return true;
}

stock bool:sscanf_int_string(const params[], &val, str[]) {
    new s1[32];
    if (!sscanf_two_strings(params, s1, str)) return false;
    val = strval(s1);
    return true;
}

// Command: /setcj - Gives skin 0 and default clothes
public OnPlayerCommandText(playerid, cmdtext[]) {
    new cmd[32], params[128];
    sscanf_custom(cmdtext, cmd, params);

    // Command: /eghelp or /extendedhelp - Comprehensive guide for all features
    if (!strcmp(cmd, "/eghelp", true) || !strcmp(cmd, "/extendedhelp", true)) {
        if (!strcmp(params, "gps", true) || !strcmp(params, "roads", true) || !strcmp(params, "road", true)) {
            SendClientMessage(playerid, 0x00FFCCAA, "==== [ExtendedGame GPS & Roads Commands] ====");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /gps <preset | x y z> - Start GPS navigation to coords or preset (ls, bank, pier, airport, sf, lv)");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /cancelgps (or /nogps) - Cancel active GPS route and clear radar line & marker");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /roadcheck [0/1] [autowarn: 0/1] - Toggle road side detection & wrong-way warnings");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /traffic [lookahead_dist] - Query live traffic & lane status (deviation, incline, target)");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /spawnroad [modelid] - Spawn & register custom road object at current facing angle");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /spawnsegment - Register a 30m custom virtual road segment in front of you");
            SendClientMessage(playerid, 0xFFFFFFFF, "  /clearroads - Clear all custom road objects and segments");
            return 1;
        }
        SendClientMessage(playerid, 0x00FFCCAA, "==== [BrownTurbo-ExtendedGame Commands] ====");
        SendClientMessage(playerid, 0xFFFF00AA, "  GPS & Roads: /gps, /cancelgps, /roadcheck, /traffic, /spawnroad, /spawnsegment, /clearroads");
        SendClientMessage(playerid, 0xFFFFFFFF, "  CJ & Clothes: /setcj, /shirt, /hair, /pants, /shoes, /glasses, /hat, /body, /tattoo, /resetcj, /bare, /outfit");
        SendClientMessage(playerid, 0xFFFFFFFF, "  Environment: /sun, /moon, /suncolor, /mooncolor, /stars, /lowclouds, /fluffyclouds, /rainbow, /time, /weather");
        SendClientMessage(playerid, 0xFFFFFFFF, "  Water: /waterlevel, /resetwater, /waterwaves, /wzcreate, /wzlevel, /wzdestroy");
        SendClientMessage(playerid, 0xFFFFFFFF, "  HUD & Radar: /hud, /hudcol, /hudwanted, /hudradio, /hudclock, /planeradar, /cursor, /ispause");
        SendClientMessage(playerid, 0xFFFFFFFF, "  Physics & Visuals: /jetpack, /jetpos, /jetrot, /vehicleblur, /nv, /tv, /noreload, /oxygen, /pedanims");
        SendClientMessage(playerid, 0xFFFFFFFF, "  TextDraws: /tdhelp, /tdcreate, /tdrotate, /ptdcreate, /ptdrotate");
        SendClientMessage(playerid, 0x00FFCCAA, "Type '/eghelp roads' for detailed GPS & Road Tracing commands.");
        return 1;
    }

    // Command: /setcj or /cjskin - Gives skin 0 and default clothes
    if (!strcmp(cmd, "/setcj", true) || !strcmp(cmd, "/cjskin", true)) {
        if (!HasPlayerExtendedGame(playerid)) {
            SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Error: You do not have the BrownTurbo-ExtendedGame ASI plugin installed!");
            SendClientMessage(playerid, 0xFFFF00AA, "  Setting skin to Model 0 without the plugin would crash your game.");
            return 1;
        }
        SetPlayerSkin(playerid, 0);
        BroadcastResetPlayerCJClothes(playerid, true, true);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] You have been transformed into Carl Johnson!");
        return 1;
    }

    // Command: /checkplugin [targetid] - Checks if player has ASI installed
    if (!strcmp(cmd, "/checkplugin", true)) {
        new target = playerid;
        if (params[0] != '\0') {
            target = strval(params);
        }
        if (!IsPlayerConnected(target)) {
            SendClientMessage(playerid, 0xFF0000AA, "Player not connected.");
            return 1;
        }
        new msg[144];
        if (HasPlayerExtendedGame(target)) {
            new ver = GetPlayerExtendedGameVersion(target);
            format(msg, sizeof(msg), "[Plugin Status] Player %d HAS the BrownTurbo-ExtendedGame ASI plugin installed (v0x%04x).", target, ver);
            SendClientMessage(playerid, 0x00FF00AA, msg);
        } else {
            format(msg, sizeof(msg), "[Plugin Status] Player %d does NOT have the BrownTurbo-ExtendedGame ASI plugin installed.", target);
            SendClientMessage(playerid, 0xFF0000AA, msg);
        }
        return 1;
    }

    // Command: /shirt [texture] [model]
    // Examples:
    // /shirt tshirtwhite tshirt
    // /shirt hoodyam hoody
    // /shirt shirtbplaid shirt
    // /shirt leather leather
    if (!strcmp(cmd, "/shirt", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            // Apply torso item and broadcast to all players
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_TORSO, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Torso clothing updated!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /shirt [texture] [model]");
            SendClientMessage(playerid, 0xCCCCCCAA, "Examples: /shirt vest vest | /shirt hoodyam hoody | /shirt leather leather");
        }
        return 1;
    }

    // Command: /hair [name] OR /hair [texture] [model]
    // Examples:
    // /hair afro
    // /hair cornrows
    // /hair bald
    // /hair default
    // /hair afro afro
    if (!strcmp(cmd, "/hair", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            if (!strcmp(tex, "dreads", true)) { format(tex, 32, "cornrows"); }
            if (!strcmp(mdl, "dreads", true)) { format(mdl, 32, "cornrows"); }
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_HEAD, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Haircut updated!");
        } else if (params[0] != '\0') {
            if (!strcmp(params, "afro", true)) {
                format(tex, 32, "afro"); format(mdl, 32, "afro");
            } else if (!strcmp(params, "highafro", true)) {
                format(tex, 32, "highafro"); format(mdl, 32, "highafro");
            } else if (!strcmp(params, "cornrows", true) || !strcmp(params, "dreads", true)) {
                format(tex, 32, "cornrows"); format(mdl, 32, "cornrows");
            } else if (!strcmp(params, "flattop", true)) {
                format(tex, 32, "flattop"); format(mdl, 32, "flattop");
            } else if (!strcmp(params, "elvishair", true)) {
                format(tex, 32, "elvishair"); format(mdl, 32, "elvishair");
            } else if (!strcmp(params, "wedge", true)) {
                format(tex, 32, "wedge"); format(mdl, 32, "wedge");
            } else if (!strcmp(params, "slope", true)) {
                format(tex, 32, "slope"); format(mdl, 32, "slope");
            } else if (!strcmp(params, "jheri", true) || !strcmp(params, "jhericurl", true)) {
                format(tex, 32, "jhericurl"); format(mdl, 32, "jheri");
            } else if (!strcmp(params, "mohawk", true)) {
                format(tex, 32, "mohawk"); format(mdl, 32, "mohawk");
            } else if (!strcmp(params, "bald", true)) {
                format(tex, 32, "bald"); format(mdl, 32, "head");
            } else if (!strcmp(params, "default", true) || !strcmp(params, "caesar", true) || !strcmp(params, "player_face", true)) {
                format(tex, 32, "player_face"); format(mdl, 32, "head");
            } else if (!strcmp(params, "blonde", true) || !strcmp(params, "hairblond", true)) {
                format(tex, 32, "hairblond"); format(mdl, 32, "head");
            } else if (!strcmp(params, "pink", true) || !strcmp(params, "hairpink", true)) {
                format(tex, 32, "hairpink"); format(mdl, 32, "head");
            } else if (!strcmp(params, "blue", true) || !strcmp(params, "hairblue", true)) {
                format(tex, 32, "hairblue"); format(mdl, 32, "head");
            } else if (!strcmp(params, "red", true) || !strcmp(params, "hairred", true)) {
                format(tex, 32, "hairred"); format(mdl, 32, "head");
            } else if (!strcmp(params, "green", true) || !strcmp(params, "hairgreen", true)) {
                format(tex, 32, "hairgreen"); format(mdl, 32, "head");
            } else {
                format(tex, 32, "%s", params);
                format(mdl, 32, "%s", params);
            }
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_HEAD, tex, mdl, true);
            new msg[128];
            format(msg, sizeof(msg), "[CJ Skin] Haircut '%s' applied (model: %s)!", tex, mdl);
            SendClientMessage(playerid, 0x00FF00AA, msg);
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /hair [name] OR /hair [texture] [model]");
            SendClientMessage(playerid, 0xCCCCCCAA, "Haircuts: afro, cornrows, flattop, elvishair, wedge, slope, jheri, mohawk, bald, default, blonde, pink, blue, red");
        }
        return 1;
    }

    // Command: /pants [texture] [model]
    // Examples:
    // /pants jeansdenim jeans
    // /pants tracktrblue tracktr
    // /pants boxer boxer
    if (!strcmp(cmd, "/pants", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_LEGS, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Pants updated!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /pants [texture] [model]");
        }
        return 1;
    }

    // Command: /shoes [texture] [model]
    // Examples:
    // /shoes sneakerbincblk sneaker
    // /shoes cowboyboot cowboy
    // /shoes flipflop flipflop
    if (!strcmp(cmd, "/shoes", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            if (!strcmp(tex, "sneakerbklc", true)) { format(tex, 32, "sneakerbincblk"); }
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_SHOES, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Shoes updated!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /shoes [texture] [model]");
        }
        return 1;
    }

    // Command: /glasses [texture] [model]
    // Examples:
    // /glasses shades glasses
    if (!strcmp(cmd, "/glasses", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_GLASSES, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Glasses updated!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /glasses [texture] [model] (or /removeglasses)");
        }
        return 1;
    }

    if (!strcmp(cmd, "/removeglasses", true)) {
        BroadcastRemovePlayerCJClothes(playerid, CJ_SLOT_GLASSES, true);
        SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Glasses removed!");
        return 1;
    }

    // Command: /hat [texture] [model]
    // Examples:
    // /hat capblktr hat
    if (!strcmp(cmd, "/hat", true)) {
        new tex[32], mdl[32];
        if (sscanf_two_strings(params, tex, mdl)) {
            BroadcastPlayerCJClothes(playerid, CJ_SLOT_HATS, tex, mdl, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Hat updated!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /hat [texture] [model] (or /removehat)");
        }
        return 1;
    }

    if (!strcmp(cmd, "/removehat", true)) {
        BroadcastRemovePlayerCJClothes(playerid, CJ_SLOT_HATS, true);
        SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Hat removed!");
        return 1;
    }

    // Command: /body [fat 0-1000] [muscle 0-1000]
    // Example: /body 0 1000  (Buff / bodybuilder)
    // Example: /body 1000 0  (Fat)
    // Example: /body 0 0     (Skinny)
    if (!strcmp(cmd, "/body", true)) {
        new Float:fat = 0.0, Float:muscle = 0.0;
        // Parse two floats
        if (sscanf_two_floats(params, fat, muscle)) {
            BroadcastPlayerCJBodyStats(playerid, fat, muscle, true);
            new str[128];
            format(str, sizeof(str), "[CJ Skin] Body stats updated! Fat: %.1f, Muscle: %.1f", fat, muscle);
            SendClientMessage(playerid, 0x00FF00AA, str);
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /body [fat: 0-1000] [muscle: 0-1000]");
        }
        return 1;
    }

    // Command: /tattoo [slot 0-8] [texture]
    // Examples: /tattoo 5 weed (Upper left arm weed tattoo)
    if (!strcmp(cmd, "/tattoo", true)) {
        new slot = 0, tex[32];
        if (sscanf_int_string(params, slot, tex)) {
            BroadcastPlayerCJTattoo(playerid, slot, tex, true);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Tattoo applied!");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /tattoo [slot 0-8] [texture]");
        }
        return 1;
    }

    // Command: /resetcj - Resets back to default singleplayer clothes
    if (!strcmp(cmd, "/resetcj", true)) {
        BroadcastResetPlayerCJClothes(playerid, true, true);
        SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Reset to default singleplayer Carl Johnson!");
        return 1;
    }

    // Command: /bare - Strips all clothes down to bare boxers
    if (!strcmp(cmd, "/bare", true)) {
        BroadcastResetPlayerCJClothes(playerid, false, true);
        SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Stripped down to bare underwear!");
        return 1;
    }

    // Command: /outfit [biker|gangster|casual] - Sets a complete coordinated outfit
    if (!strcmp(cmd, "/outfit", true)) {
        if (!strcmp(params, "biker", true)) {
            new slots[4] = { CJ_SLOT_TORSO, CJ_SLOT_LEGS, CJ_SLOT_SHOES, CJ_SLOT_HEAD };
            new textures[4][32] = { "leather", "jeansdenim", "cowboyboot", "bald" };
            new models[4][32] = { "leather", "jeans", "cowboy", "head" };
            BroadcastPlayerCJFullOutfit(playerid, 200.0, 800.0, slots, textures, models, 4);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Full Biker outfit applied!");
        } else if (!strcmp(params, "gangster", true)) {
            new slots[4] = { CJ_SLOT_TORSO, CJ_SLOT_LEGS, CJ_SLOT_SHOES, CJ_SLOT_HEAD };
            new textures[4][32] = { "hoodyablack", "jeansdenim", "sneakerbincblk", "cornrows" };
            new models[4][32] = { "hoodya", "jeans", "sneaker", "cornrows" };
            BroadcastPlayerCJFullOutfit(playerid, 100.0, 600.0, slots, textures, models, 4);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Full Gangster outfit applied!");
        } else {
            // Casual
            new slots[4] = { CJ_SLOT_TORSO, CJ_SLOT_LEGS, CJ_SLOT_SHOES, CJ_SLOT_HEAD };
            new textures[4][32] = { "tshirtwhite", "tracktrblue", "sneakerbincblk", "afro" };
            new models[4][32] = { "tshirt", "tracktr", "sneaker", "afro" };
            BroadcastPlayerCJFullOutfit(playerid, 0.0, 400.0, slots, textures, models, 4);
            SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Full Casual outfit applied!");
        }
        return 1;
    }

    // Command: /clearcache - Purges client-side cached clothing state
    if (!strcmp(cmd, "/clearcache", true)) {
        BroadcastClearPlayerCJCache(playerid);
        SendClientMessage(playerid, 0x00FF00AA, "[CJ Skin] Client cache cleared!");
        return 1;
    }

    // Command: /myoutfit - Displays server-stored CJ clothing items
    if (!strcmp(cmd, "/myoutfit", true)) {
        if (!IsPlayerUsingCJSkin(playerid)) {
            SendClientMessage(playerid, 0xFF0000AA, "[CJ Skin] You are not currently wearing a custom CJ outfit!");
            return 1;
        }
        new Float:fat, Float:muscle;
        GetPlayerCJBodyStats(playerid, fat, muscle);
        new msg[144], tex[32], mdl[32];
        format(msg, sizeof(msg), "[CJ Outfit] Body Stats - Fat: %.1f | Muscle: %.1f", fat, muscle);
        SendClientMessage(playerid, 0x00FF00AA, msg);

        GetPlayerCJClothes(playerid, CJ_SLOT_TORSO, tex, mdl);
        if (tex[0] != '\0') {
            format(msg, sizeof(msg), "  Torso: %s (model: %s)", tex, mdl);
            SendClientMessage(playerid, 0xFFFFFFFF, msg);
        }
        GetPlayerCJClothes(playerid, CJ_SLOT_LEGS, tex, mdl);
        if (tex[0] != '\0') {
            format(msg, sizeof(msg), "  Legs: %s (model: %s)", tex, mdl);
            SendClientMessage(playerid, 0xFFFFFFFF, msg);
        }
        GetPlayerCJClothes(playerid, CJ_SLOT_SHOES, tex, mdl);
        if (tex[0] != '\0') {
            format(msg, sizeof(msg), "  Shoes: %s (model: %s)", tex, mdl);
            SendClientMessage(playerid, 0xFFFFFFFF, msg);
        }
        GetPlayerCJClothes(playerid, CJ_SLOT_HEAD, tex, mdl);
        if (tex[0] != '\0') {
            format(msg, sizeof(msg), "  Head: %s (model: %s)", tex, mdl);
            SendClientMessage(playerid, 0xFFFFFFFF, msg);
        }
        return 1;
    }

    // Command: /airlimit [height]
    if (!strcmp(cmd, "/airlimit", true)) {
        if (params[0] == '\0') {
            new Float:cur = GetPlayerAircraftHeightLimit(playerid);
            new msg[128];
            format(msg, sizeof(msg), "Usage: /airlimit [height] (current: %.1f)", cur);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }
        new Float:limit = floatstr(params);
        SetAircraftHeightLimitForPlayer(playerid, limit);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Aircraft height limit set to %.1f m.", limit);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /jetlimit [height]
    if (!strcmp(cmd, "/jetlimit", true)) {
        if (params[0] == '\0') {
            new Float:cur = GetPlayerJetpackHeightLimit(playerid);
            new msg[128];
            format(msg, sizeof(msg), "Usage: /jetlimit [height] (current: %.1f)", cur);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }
        new Float:limit = floatstr(params);
        SetJetpackHeightLimitForPlayer(playerid, limit);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Jetpack height limit set to %.1f m.", limit);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /jetpack [optional_height] - Gives or removes a jetpack; optionally sets height limit
    if (!strcmp(cmd, "/jetpack", true)) {
        if (GetPlayerSpecialAction(playerid) == SPECIAL_ACTION_USEJETPACK) {
            SetPlayerSpecialAction(playerid, SPECIAL_ACTION_NONE);
            SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Jetpack removed.");
        } else {
            SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
            if (params[0] != '\0') {
                // Optional: set height limit at the same time (e.g. /jetpack 500, /jetpack 0 for infinite)
                new Float:limit = floatstr(params);
                SetJetpackHeightLimitForPlayer(playerid, limit);
                new msg[144];
                if (limit <= 0.0) {
                    format(msg, sizeof(msg), "[ExtendedGame] Jetpack equipped! Height limit: INFINITE.");
                } else {
                    format(msg, sizeof(msg), "[ExtendedGame] Jetpack equipped! Height limit: %.1f m. Use /jetlimit to adjust.", limit);
                }
                SendClientMessage(playerid, 0x00FF00AA, msg);
            } else {
                SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Jetpack equipped! Use /jetlimit [height] to set a ceiling (e.g. /jetlimit 500 or /jetlimit 0 for infinite).");
            }
        }
        return 1;
    }

    // Command: /jetoffset [x] [y] [z] - Set custom translation offset for jetpack
    if (!strcmp(cmd, "/jetoffset", true) || !strcmp(cmd, "/jetpos", true)) {
        new Float:x, Float:y, Float:z;
        GetPlayerJetpackOffset(playerid, x, y, z);
        if (params[0] == '\0') {
            new msg[144];
            format(msg, sizeof(msg), "[Jetpack Offset] Current: X=%.3f, Y=%.3f, Z=%.3f", x, y, z);
            SendClientMessage(playerid, 0xFFFF00AA, msg);
            SendClientMessage(playerid, 0xFFFFFFFF, "Usage: /jetoffset [x] [y] [z]  (e.g. /jetoffset 0.10 -0.38 0.00)");
            return 1;
        }
        new endX = -1, endY = -1;
        for (new i = 0; params[i] != '\0'; i++) {
            if (params[i] == ' ') {
                if (endX == -1) endX = i;
                else if (endY == -1) { endY = i; break; }
            }
        }
        if (endX == -1 || endY == -1) {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /jetoffset [x] [y] [z]  (e.g. /jetoffset 0.10 -0.38 0.00)");
            return 1;
        }
        x = floatstr(params);
        y = floatstr(params[endX + 1]);
        z = floatstr(params[endY + 1]);
        SetJetpackOffsetForPlayer(playerid, x, y, z);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Jetpack offset set to X=%.3f, Y=%.3f, Z=%.3f.", x, y, z);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /jetrot [angle] [axisX] [axisY] [axisZ] - Set custom rotation angle and axis
    if (!strcmp(cmd, "/jetrot", true)) {
        new Float:angle, Float:axisX, Float:axisY, Float:axisZ;
        GetPlayerJetpackRotation(playerid, angle, axisX, axisY, axisZ);
        if (params[0] == '\0') {
            new msg[144];
            format(msg, sizeof(msg), "[Jetpack Rotation] Current: Angle=%.1f deg, Axis=(%.2f, %.2f, %.2f)", angle, axisX, axisY, axisZ);
            SendClientMessage(playerid, 0xFFFF00AA, msg);
            SendClientMessage(playerid, 0xFFFFFFFF, "Usage: /jetrot [angle] [optional: axisX axisY axisZ] (e.g. /jetrot 90.0 or /jetrot 90.0 0.0 1.0 0.0)");
            return 1;
        }
        angle = floatstr(params);
        new firstSpace = -1;
        for (new i = 0; params[i] != '\0'; i++) {
            if (params[i] == ' ') { firstSpace = i; break; }
        }
        if (firstSpace != -1) {
            new endAxX = -1;
            for (new i = firstSpace + 1; params[i] != '\0'; i++) {
                if (params[i] == ' ') { endAxX = i; break; }
            }
            if (endAxX != -1) {
                new endAxY = -1;
                for (new i = endAxX + 1; params[i] != '\0'; i++) {
                    if (params[i] == ' ') { endAxY = i; break; }
                }
                if (endAxY != -1) {
                    axisX = floatstr(params[firstSpace + 1]);
                    axisY = floatstr(params[endAxX + 1]);
                    axisZ = floatstr(params[endAxY + 1]);
                }
            }
        }
        SetJetpackRotationForPlayer(playerid, angle, axisX, axisY, axisZ);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Jetpack rotation set: Angle=%.1f deg, Axis=(%.2f, %.2f, %.2f).", angle, axisX, axisY, axisZ);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /jetreset - Reset jetpack placement and rotation
    if (!strcmp(cmd, "/jetreset", true)) {
        ResetJetpackOffsetForPlayer(playerid);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Jetpack offset and rotation reset to umbrella-tuned default.");
        return 1;
    }

    // Command: /bikerule [0=default, 1=never, 2=always]
    if (!strcmp(cmd, "/bikerule", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /bikerule [0=Default, 1=Never knock off, 2=Always knock off]");
            return 1;
        }
        new knockState = strval(params);
        SetPlayerKnockedOffBikeState(playerid, knockState);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Bike knock-off state set to %d.", knockState);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /fireproof [0/1]
    if (!strcmp(cmd, "/fireproof", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /fireproof [0=Disabled, 1=Enabled]");
            return 1;
        }
        new enable = strval(params);
        SetPlayerFireProof(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] You are now fireproof!" : "[ExtendedGame] Fireproof disabled.");
        return 1;
    }

    // Command: /sprint [0/1]
    if (!strcmp(cmd, "/sprint", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /sprint [0=Normal, 1=Infinite sprint]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerInfiniteSprint(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Infinite sprint enabled!" : "[ExtendedGame] Infinite sprint disabled.");
        return 1;
    }

    // Command: /sun [0/1]
    if (!strcmp(cmd, "/sun", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /sun [0=Hide sun, 1=Show sun]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerSun(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Sun visible." : "[ExtendedGame] Sun hidden.");
        return 1;
    }

    // Command: /moon [0/1]
    if (!strcmp(cmd, "/moon", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /moon [0=Hide moon, 1=Show moon]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerMoon(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Moon visible." : "[ExtendedGame] Moon hidden.");
        return 1;
    }

    // Command: /stars [0/1]
    if (!strcmp(cmd, "/stars", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /stars [0=Hide stars, 1=Show stars]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerStars(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Stars visible." : "[ExtendedGame] Stars hidden.");
        return 1;
    }

    // Command: /lowclouds [0/1]
    if (!strcmp(cmd, "/lowclouds", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /lowclouds [0=Hide low clouds, 1=Show low clouds]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerLowClouds(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Low clouds visible." : "[ExtendedGame] Low clouds hidden.");
        return 1;
    }

    // Command: /fluffyclouds [0/1]
    if (!strcmp(cmd, "/fluffyclouds", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /fluffyclouds [0=Hide fluffy clouds, 1=Show fluffy clouds]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerFluffyClouds(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Fluffy clouds visible." : "[ExtendedGame] Fluffy clouds hidden.");
        return 1;
    }

    // Command: /rainbow [0/1]
    if (!strcmp(cmd, "/rainbow", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /rainbow [0=Hide rainbow, 1=Show rainbow]");
            return 1;
        }
        new enable = strval(params);
        TogglePlayerRainbow(playerid, enable);
        SendClientMessage(playerid, 0x00FF00AA, enable ? "[ExtendedGame] Rainbow enabled." : "[ExtendedGame] Rainbow disabled.");
        return 1;
    }

    // Command: /waterlevel [height] - Sets world sea, lakes, and ocean water heights
    if (!strcmp(cmd, "/waterlevel", true)) {
        if (params[0] == '\0') {
            new Float:current = GetPlayerWaterLevel(playerid);
            new msg[144];
            format(msg, sizeof(msg), "Usage: /waterlevel [height] (current: %.2f m)", current);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }

        new Float:level;
        if (!_ParseFloatStrict(params, level)) {
            SendClientMessage(playerid, 0xFF0000AA, "Please provide a numeric water height.");
            return 1;
        }
        if (!SetPlayerWaterLevel(playerid, level, true, true, true)) {
            SendClientMessage(playerid, 0xFF0000AA, "Water level could not be applied. Check the value and client initialization.");
            return 1;
        }
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Water level set to %.2f m (including ocean & coast).", level);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /resetwater - Restores GTA:SA's original world water heights
    if (!strcmp(cmd, "/resetwater", true)) {
        ResetPlayerWaterLevel(playerid);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] World water levels reset to GTA:SA defaults.");
        return 1;
    }

    // Command: /waterwaves [percent] - 0..100% intensity; -1 restores native weather-driven waves
    if (!strcmp(cmd, "/waterwaves", true) || !strcmp(cmd, "/waveheight", true)) {
        if (params[0] == '\0') {
            new Float:current = GetPlayerWaterWaveHeight(playerid);
            new msg[128];
            format(msg, sizeof(msg), "Usage: /waterwaves [-1.0 to 100.0] percent (current: %.2f%%; -1 restores GTA default)", current);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }

        new Float:height;
        if (!_ParseFloatStrict(params, height)) {
            SendClientMessage(playerid, 0xFF0000AA, "Please provide a numeric wave intensity percent.");
            return 1;
        }
        if (!SetPlayerWaterWaveHeight(playerid, height)) {
            SendClientMessage(playerid, 0xFF0000AA, "Water wave intensity must be between -1.0 and 100.0 percent.");
            return 1;
        }
        new msg[128];
        if (height < 0.0) {
            format(msg, sizeof(msg), "[ExtendedGame] GTA:SA's native water wave behavior restored.");
        } else {
            format(msg, sizeof(msg), "[ExtendedGame] Water wave intensity set to %.2f%% of normal.", height);
        }
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /hud [components_mask]
    if (!strcmp(cmd, "/hud", true)) {
        if (params[0] == '\0') {
            new cur = GetPlayerHUDComponents(playerid);
            new msg[144];
            format(msg, sizeof(msg), "Usage: /hud [mask] (current: %d, All=511, Ammo=1, Icon=2, Health=4, Oxygen=8, Armour=16, Radar=32, Crosshair=64, Money=128, Laser=256)", cur);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }
        new mask = strval(params);
        TogglePlayerHUDComponents(playerid, mask);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] HUD components mask set to %d.", mask);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /hudcol [component_id] [hex_colour]
    // Example: /hudcol 4 0xFF0000FF (Health red)
    if (!strcmp(cmd, "/hudcol", true)) {
        new sComp[32], sCol[32];
        if (!sscanf_two_strings(params, sComp, sCol)) {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /hudcol [component_id] [hex_colour]");
            SendClientMessage(playerid, 0xFFFF00AA, "  Components: 1=Ammo, 2=Icon, 4=Health, 8=Oxygen, 16=Armour, 32=Radar, 64=Crosshair, 128=Money, 256=LaserDot");
            SendClientMessage(playerid, 0xFFFF00AA, "  Example: /hudcol 4 0xFF0000FF (sets health bar red)");
            return 1;
        }
        new comp = strval(sComp);
        new col = 0;
        // Parse hex color 0xRRGGBBAA or RRGGBBAA
        new startIdx = 0;
        if (sCol[0] == '0' && (sCol[1] == 'x' || sCol[1] == 'X')) startIdx = 2;
        for (new i = startIdx; sCol[i] != '\0'; i++) {
            new c = sCol[i];
            new digit = 0;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            col = (col << 4) | digit;
        }
        SetPlayerHUDComponentsColour(playerid, comp, col);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Set HUD component %d color to 0x%08X.", comp, col);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /time [0-23]
    if (!strcmp(cmd, "/time", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /time [0-23] (e.g. 0 or 2 for Midnight/Moon/Stars, 12 for Noon/Sun)");
            return 1;
        }
        new hour = strval(params);
        SetPlayerTime(playerid, hour, 0);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Clock set to %02d:00.", hour);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /weather [0-45]
    if (!strcmp(cmd, "/weather", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /weather [0-45] (e.g. 1=Sunny, 8=Rainy, 9=Foggy)");
            return 1;
        }
        new wId = strval(params);
        SetPlayerWeather(playerid, wId);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Weather set to ID %d.", wId);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /tdhelp - Lists all Global and Per-Player TextDraw rotation commands
    if (!strcmp(cmd, "/tdhelp", true)) {
        SendClientMessage(playerid, 0x00FF00AA, "=== [ExtendedGame] TextDraw Rotation Commands ===");
        SendClientMessage(playerid, 0xFFFFFFFF, "Global: /tdcreate, /tdrotate [angle] [spin], /tdspin [spin], /tdreset, /tddestroy");
        SendClientMessage(playerid, 0xFFFFFFFF, "Per-Player: /ptdcreate, /ptdrotate [angle] [spin], /ptdspin [spin], /ptdreset, /ptddestroy");
        return 1;
    }

    // Command: /tdcreate - Creates demo textdraws to test rotation
    if (!strcmp(cmd, "/tdcreate", true)) {
        if (g_DemoTextDraw != Text:INVALID_TEXT_DRAW) {
            TextDrawDestroy(g_DemoTextDraw);
        }
        if (g_DemoSpriteTD != Text:INVALID_TEXT_DRAW) {
            TextDrawDestroy(g_DemoSpriteTD);
        }

        // 1. Text TextDraw
        new textStr[] = "BrownTurbo TextDraw Rotation";
        g_DemoTextDraw = TextDrawCreate(320.0, 200.0, textStr);
        TextDrawAlignment(g_DemoTextDraw, 2); // Center
        TextDrawFont(g_DemoTextDraw, 1);
        TextDrawLetterSize(g_DemoTextDraw, 0.45, 1.8);
        TextDrawColor(g_DemoTextDraw, 0x00FF00FF);
        TextDrawSetOutline(g_DemoTextDraw, 1);
        TextDrawUseBox(g_DemoTextDraw, 1);
        TextDrawBoxColor(g_DemoTextDraw, 0x00000088);
        TextDrawTextSize(g_DemoTextDraw, 20.0, 260.0);
        TextDrawShowForPlayer(playerid, g_DemoTextDraw);

        // 2. Sprite TextDraw (LD_BEAT:chit)
        new spriteStr[] = "LD_BEAT:chit";
        g_DemoSpriteTD = TextDrawCreate(320.0, 260.0, spriteStr);
        TextDrawFont(g_DemoSpriteTD, 4); // TXD Sprite
        TextDrawColor(g_DemoSpriteTD, 0xFFFFFFFF);
        TextDrawTextSize(g_DemoSpriteTD, 50.0, 50.0);
        TextDrawAlignment(g_DemoSpriteTD, 2);
        TextDrawShowForPlayer(playerid, g_DemoSpriteTD);

        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Created demo TextDraws! Use /tdrotate, /tdspin, /tdreset, /tddestroy.");
        return 1;
    }

    // Command: /tdrotate [angle] [optional: spinspeed]
    if (!strcmp(cmd, "/tdrotate", true)) {
        if (g_DemoTextDraw == Text:INVALID_TEXT_DRAW) {
            SendClientMessage(playerid, 0xFF0000AA, "Please create demo textdraws first with /tdcreate!");
            return 1;
        }
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /tdrotate [angle] (e.g. /tdrotate 45) or /tdrotate [angle] [spinspeed]");
            return 1;
        }

        new Float:angle = 0.0, Float:spin = 0.0;
        new part1[32], part2[32];
        new s = 0;
        while (params[s] != '\0' && params[s] != ' ') {
            part1[s] = params[s];
            s++;
        }
        part1[s] = '\0';
        while (params[s] == ' ') s++;
        if (params[s] != '\0') {
            new p = 0;
            while (params[s] != '\0' && params[s] != ' ') {
                part2[p++] = params[s++];
            }
            part2[p] = '\0';
            _ParseFloatStrict(part2, spin);
        }
        _ParseFloatStrict(part1, angle);

        SetTextDrawRotation(g_DemoTextDraw, angle, -1.0, -1.0, spin);
        SetTextDrawRotation(g_DemoSpriteTD, angle, -1.0, -1.0, spin);

        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] TextDraw rotation set to %.1f deg (spin: %.1f deg/s).", angle, spin);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /tdspin [spinspeed]
    if (!strcmp(cmd, "/tdspin", true)) {
        if (g_DemoTextDraw == Text:INVALID_TEXT_DRAW) {
            SendClientMessage(playerid, 0xFF0000AA, "Please create demo textdraws first with /tdcreate!");
            return 1;
        }
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /tdspin [spinspeed deg/s] (e.g. /tdspin 90, /tdspin -180)");
            return 1;
        }
        new Float:spin;
        if (!_ParseFloatStrict(params, spin)) {
            SendClientMessage(playerid, 0xFF0000AA, "Invalid spin speed.");
            return 1;
        }
        SetTextDrawRotation(g_DemoTextDraw, 0.0, -1.0, -1.0, spin);
        SetTextDrawRotation(g_DemoSpriteTD, 0.0, -1.0, -1.0, spin);

        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Continuous spin set to %.1f deg/s.", spin);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /tdreset
    if (!strcmp(cmd, "/tdreset", true)) {
        if (g_DemoTextDraw != Text:INVALID_TEXT_DRAW) {
            ResetTextDrawRotation(g_DemoTextDraw);
        }
        if (g_DemoSpriteTD != Text:INVALID_TEXT_DRAW) {
            ResetTextDrawRotation(g_DemoSpriteTD);
        }
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] TextDraw rotations reset to 0.");
        return 1;
    }

    // Command: /tddestroy
    if (!strcmp(cmd, "/tddestroy", true)) {
        if (g_DemoTextDraw != Text:INVALID_TEXT_DRAW) {
            TextDrawDestroy(g_DemoTextDraw);
            g_DemoTextDraw = Text:INVALID_TEXT_DRAW;
        }
        if (g_DemoSpriteTD != Text:INVALID_TEXT_DRAW) {
            TextDrawDestroy(g_DemoSpriteTD);
            g_DemoSpriteTD = Text:INVALID_TEXT_DRAW;
        }
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Demo TextDraws destroyed.");
        return 1;
    }

    // Command: /ptdcreate - Creates demo per-player textdraws to test per-player rotation
    if (!strcmp(cmd, "/ptdcreate", true)) {
        if (g_DemoPlayerTextDraw[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(playerid, g_DemoPlayerTextDraw[playerid]);
            g_DemoPlayerTextDraw[playerid] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (g_DemoPlayerSpriteTD[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(playerid, g_DemoPlayerSpriteTD[playerid]);
            g_DemoPlayerSpriteTD[playerid] = PlayerText:INVALID_TEXT_DRAW;
        }

        // 1. Per-Player Text TextDraw
        new textStr[] = "BrownTurbo Player TextDraw";
        g_DemoPlayerTextDraw[playerid] = CreatePlayerTextDraw(playerid, 320.0, 140.0, textStr);
        PlayerTextDrawAlignment(playerid, g_DemoPlayerTextDraw[playerid], 2); // Center
        PlayerTextDrawFont(playerid, g_DemoPlayerTextDraw[playerid], 1);
        PlayerTextDrawLetterSize(playerid, g_DemoPlayerTextDraw[playerid], 0.45, 1.8);
        PlayerTextDrawColor(playerid, g_DemoPlayerTextDraw[playerid], 0xFFFF00FF); // Yellow
        PlayerTextDrawSetOutline(playerid, g_DemoPlayerTextDraw[playerid], 1);
        PlayerTextDrawUseBox(playerid, g_DemoPlayerTextDraw[playerid], 1);
        PlayerTextDrawBoxColor(playerid, g_DemoPlayerTextDraw[playerid], 0x00000088);
        PlayerTextDrawTextSize(playerid, g_DemoPlayerTextDraw[playerid], 20.0, 260.0);
        PlayerTextDrawShow(playerid, g_DemoPlayerTextDraw[playerid]);

        // 2. Per-Player Sprite TextDraw (LD_BEAT:chit)
        new spriteStr[] = "LD_BEAT:chit";
        g_DemoPlayerSpriteTD[playerid] = CreatePlayerTextDraw(playerid, 320.0, 80.0, spriteStr);
        PlayerTextDrawFont(playerid, g_DemoPlayerSpriteTD[playerid], 4); // TXD Sprite
        PlayerTextDrawColor(playerid, g_DemoPlayerSpriteTD[playerid], 0x00FFFFFF); // Cyan
        PlayerTextDrawTextSize(playerid, g_DemoPlayerSpriteTD[playerid], 50.0, 50.0);
        PlayerTextDrawAlignment(playerid, g_DemoPlayerSpriteTD[playerid], 2);
        PlayerTextDrawShow(playerid, g_DemoPlayerSpriteTD[playerid]);

        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Created demo Per-Player TextDraws! Use /ptdrotate, /ptdspin, /ptdreset, /ptddestroy.");
        return 1;
    }

    // Command: /ptdrotate [angle] [optional: spinspeed]
    if (!strcmp(cmd, "/ptdrotate", true)) {
        if (g_DemoPlayerTextDraw[playerid] == PlayerText:INVALID_TEXT_DRAW) {
            SendClientMessage(playerid, 0xFF0000AA, "Please create demo player textdraws first with /ptdcreate!");
            return 1;
        }
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /ptdrotate [angle] (e.g. /ptdrotate 45) or /ptdrotate [angle] [spinspeed]");
            return 1;
        }

        new Float:angle = 0.0, Float:spin = 0.0;
        new part1[32], part2[32];
        new s = 0;
        while (params[s] != '\0' && params[s] != ' ') {
            part1[s] = params[s];
            s++;
        }
        part1[s] = '\0';
        while (params[s] == ' ') s++;
        if (params[s] != '\0') {
            new p = 0;
            while (params[s] != '\0' && params[s] != ' ') {
                part2[p++] = params[s++];
            }
            part2[p] = '\0';
            _ParseFloatStrict(part2, spin);
        }
        _ParseFloatStrict(part1, angle);

        SetPlayerTextDrawRotation(playerid, g_DemoPlayerTextDraw[playerid], angle, -1.0, -1.0, spin);
        SetPlayerTextDrawRotation(playerid, g_DemoPlayerSpriteTD[playerid], angle, -1.0, -1.0, spin);

        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Per-Player TextDraw rotation set to %.1f deg (spin: %.1f deg/s).", angle, spin);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /ptdspin [spinspeed]
    if (!strcmp(cmd, "/ptdspin", true)) {
        if (g_DemoPlayerTextDraw[playerid] == PlayerText:INVALID_TEXT_DRAW) {
            SendClientMessage(playerid, 0xFF0000AA, "Please create demo player textdraws first with /ptdcreate!");
            return 1;
        }
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFF0000AA, "Usage: /ptdspin [spinspeed deg/s] (e.g. /ptdspin 90, /ptdspin -180)");
            return 1;
        }
        new Float:spin;
        if (!_ParseFloatStrict(params, spin)) {
            SendClientMessage(playerid, 0xFF0000AA, "Invalid spin speed.");
            return 1;
        }
        SetPlayerTextDrawRotation(playerid, g_DemoPlayerTextDraw[playerid], 0.0, -1.0, -1.0, spin);
        SetPlayerTextDrawRotation(playerid, g_DemoPlayerSpriteTD[playerid], 0.0, -1.0, -1.0, spin);

        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Continuous per-player spin set to %.1f deg/s.", spin);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /ptdreset
    if (!strcmp(cmd, "/ptdreset", true)) {
        if (g_DemoPlayerTextDraw[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            ResetPlayerTextDrawRotation(playerid, g_DemoPlayerTextDraw[playerid]);
        }
        if (g_DemoPlayerSpriteTD[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            ResetPlayerTextDrawRotation(playerid, g_DemoPlayerSpriteTD[playerid]);
        }
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Per-Player TextDraw rotations reset to 0.");
        return 1;
    }

    // Command: /ptddestroy
    if (!strcmp(cmd, "/ptddestroy", true)) {
        if (g_DemoPlayerTextDraw[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(playerid, g_DemoPlayerTextDraw[playerid]);
            g_DemoPlayerTextDraw[playerid] = PlayerText:INVALID_TEXT_DRAW;
        }
        if (g_DemoPlayerSpriteTD[playerid] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(playerid, g_DemoPlayerSpriteTD[playerid]);
            g_DemoPlayerSpriteTD[playerid] = PlayerText:INVALID_TEXT_DRAW;
        }
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Demo Per-Player TextDraws destroyed.");
        return 1;
    }

    // Command: /wzcreate [optional: size] [optional: height_offset]
    // Creates a custom swimmable water pool around the player
    if (!strcmp(cmd, "/wzcreate", true)) {
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);

        new Float:size = 40.0;
        new Float:offset = 0.5;

        if (params[0] != '\0') {
            new part1[32], part2[32];
            new s = 0;
            while (params[s] != '\0' && params[s] != ' ') {
                part1[s] = params[s];
                s++;
            }
            part1[s] = '\0';
            while (params[s] == ' ') s++;
            if (params[s] != '\0') {
                new p = 0;
                while (params[s] != '\0' && params[s] != ' ') {
                    part2[p++] = params[s++];
                }
                part2[p] = '\0';
                _ParseFloatStrict(part2, offset);
            }
            _ParseFloatStrict(part1, size);
        }

        if (g_DemoWaterZone != INVALID_WATER_ZONE) {
            DestroyWaterZone(g_DemoWaterZone);
            g_DemoWaterZone = INVALID_WATER_ZONE;
        }

        new Float:half = size / 2.0;
        new Float:minX = x - half;
        new Float:maxX = x + half;
        new Float:minY = y - half;
        new Float:maxY = y + half;
        new Float:waterZ = z + offset;
        new Float:bottomZ = z - 20.0;

        g_DemoWaterZone = CreateWaterZone(minX, minY, maxX, maxY, waterZ, bottomZ);
        if (g_DemoWaterZone == INVALID_WATER_ZONE) {
            SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Failed to create water zone (limit reached or invalid params).");
            return 1;
        }

        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Water zone #%d created! Size: %.1fx%.1f m, Surface Z: %.2f m. Use /wzlevel, /wzdestroy.", g_DemoWaterZone, size, size, waterZ);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /wzlevel [height] - Change custom water zone height
    if (!strcmp(cmd, "/wzlevel", true)) {
        if (g_DemoWaterZone == INVALID_WATER_ZONE) {
            SendClientMessage(playerid, 0xFF0000AA, "Please create a demo water zone first with /wzcreate!");
            return 1;
        }
        if (params[0] == '\0') {
            new Float:current = 0.0;
            GetWaterZoneLevel(g_DemoWaterZone, current);
            new msg[128];
            format(msg, sizeof(msg), "Usage: /wzlevel [height] (current: %.2f m)", current);
            SendClientMessage(playerid, 0xFF0000AA, msg);
            return 1;
        }
        new Float:newLevel;
        if (!_ParseFloatStrict(params, newLevel)) {
            SendClientMessage(playerid, 0xFF0000AA, "Please provide a numeric water height.");
            return 1;
        }
        SetWaterZoneLevel(g_DemoWaterZone, newLevel);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Water zone #%d level set to %.2f m.", g_DemoWaterZone, newLevel);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // Command: /wzdestroy - Destroy custom water zone
    if (!strcmp(cmd, "/wzdestroy", true)) {
        if (g_DemoWaterZone == INVALID_WATER_ZONE) {
            SendClientMessage(playerid, 0xFF0000AA, "No demo water zone is active.");
            return 1;
        }
        DestroyWaterZone(g_DemoWaterZone);
        g_DemoWaterZone = INVALID_WATER_ZONE;
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Demo water zone destroyed.");
        return 1;
    }

    // --- Vision Commands ---
    if (!strcmp(cmd, "/nv", true)) {
        new bool:toggle = !GetPlayerNightVision(playerid);
        ToggleNightVision(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Night Vision %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/tv", true)) {
        new bool:toggle = !GetPlayerThermalVision(playerid);
        ToggleThermalVision(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Thermal Vision %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // --- Weapon Reload Commands ---
    if (!strcmp(cmd, "/noreload", true)) {
        new bool:toggle = !GetPlayerNoReload(playerid);
        SetPlayerNoReload(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Weapon No-Reload %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/switchreload", true)) {
        new bool:toggle = !GetPlayerSwitchReload(playerid);
        TogglePlayerSwitchReload(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Switch Reload (auto-reload on weapon switch) %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/quickreload", true)) {
        new bool:toggle = !GetPlayerQuickReload(playerid);
        ToggleQuickReloadForPlayer(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Quick Reload %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // --- Physics & Movement Commands ---
    if (!strcmp(cmd, "/oxygen", true)) {
        new bool:toggle = !GetPlayerInfiniteOxygen(playerid);
        TogglePlayerInfiniteOxygen(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Infinite Oxygen %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/pedanims", true)) {
        new curr = GetPlayerPedAnims(playerid);
        new bool:toggle = (curr != 1);
        SetPlayerPedAnims(playerid, toggle);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] CJ Ped Walk/Run Animations %s for all players.", toggle ? "Enabled (CJ moves like CJ)" : "Disabled (CJ moves like normal ped)");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/cursor", true)) {
        new bool:toggle = !GetCursorState(playerid);
        TogglePlayerCursor(playerid, toggle);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] Player Cursor %s.", toggle ? "Enabled" : "Disabled");
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    // --- Plane Radar Color Command (Hexadecimal 0xRRGGBBAA) ---
    if (!strcmp(cmd, "/planeradar", true)) {
        new color = 0xFFFFFFFF;
        if (params[0] != '\0') {
            if (!strcmp(params, "red", true)) {
                color = 0xFF0000FF;
            } else if (!strcmp(params, "green", true)) {
                color = 0x00FF00FF;
            } else if (!strcmp(params, "blue", true)) {
                color = 0x0080FFFF;
            } else if (!strcmp(params, "yellow", true)) {
                color = 0xFFFF00FF;
            } else if (!strcmp(params, "reset", true)) {
                color = 0xFFFFFFFF;
            } else {
                color = ParseColorHex(params);
            }
        }
        SetPlaneRadarColor(playerid, color);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Plane radar altimeter color set to 0x%08x.", color);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        SendClientMessage(playerid, 0xFFFF00AA, "Usage: /planeradar <0xRRGGBBAA | red | green | blue | yellow | reset>");
        return 1;
    }

    // --- Sun Color Commands ---
    if (!strcmp(cmd, "/suncolor", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFFFF00AA, "Usage: /suncolor <hexCore> [hexCorona]  (e.g. /suncolor 0xFF0000FF, /suncolor red, blood, gold)");
            return 1;
        }
        new p1[32], p2[32];
        new coreColor = 0xFFFFFFFF, coronaColor = 0xFFFFFFFF;
        if (sscanf_two_strings(params, p1, p2)) {
            coreColor = ParseColorHex(p1);
            coronaColor = ParseColorHex(p2);
        } else {
            if (!strcmp(params, "red", true)) {
                coreColor = 0xFF2020FF; coronaColor = 0xFF6000AA;
            } else if (!strcmp(params, "blood", true)) {
                coreColor = 0xFF0000FF; coronaColor = 0x880000FF;
            } else if (!strcmp(params, "gold", true)) {
                coreColor = 0xFFFF00FF; coronaColor = 0xFFA500FF;
            } else if (!strcmp(params, "blue", true)) {
                coreColor = 0x0088FFFF; coronaColor = 0x00FFFF88;
            } else if (!strcmp(params, "green", true)) {
                coreColor = 0x00FF00FF; coronaColor = 0x88FF0088;
            } else if (!strcmp(params, "purple", true)) {
                coreColor = 0xAA00FFAA; coronaColor = 0xFF00FFAA;
            } else {
                coreColor = ParseColorHex(params);
                coronaColor = coreColor;
            }
        }
        SetPlayerSunColor(playerid, coreColor, coronaColor);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Sun color set to Core: 0x%08x, Corona: 0x%08x.", coreColor, coronaColor);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/resetsuncolor", true)) {
        ResetPlayerSunColor(playerid);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Sun color restored to default game timecycle.");
        return 1;
    }

    // --- Moon Color Commands ---
    if (!strcmp(cmd, "/mooncolor", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFFFF00AA, "Usage: /mooncolor <0xRRGGBBAA | red | blood | blue | green | yellow | white>");
            return 1;
        }
        new color = 0xFFFFFFFF;
        if (!strcmp(params, "red", true) || !strcmp(params, "blood", true)) {
            color = 0xFF2020FF;
        } else if (!strcmp(params, "blue", true)) {
            color = 0x4080FFFF;
        } else if (!strcmp(params, "green", true)) {
            color = 0x40FF40FF;
        } else if (!strcmp(params, "yellow", true)) {
            color = 0xFFFF40FF;
        } else if (!strcmp(params, "white", true)) {
            color = 0xFFFFFFFF;
        } else {
            color = ParseColorHex(params);
        }
        SetPlayerMoonColor(playerid, color);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Moon color set to 0x%08x.", color);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/resetmooncolor", true)) {
        ResetPlayerMoonColor(playerid);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Moon color restored to default.");
        return 1;
    }

    // --- Vehicle Motion Blur Command ---
    if (!strcmp(cmd, "/vehicleblur", true)) {
        new currentIntensity = 36;
        new bool:enabled = GetPlayerVehicleBlur(playerid, currentIntensity);
        new bool:newToggle = !enabled;
        new intensity = (params[0] != '\0') ? strval(params) : 36;
        if (intensity <= 0 || intensity > 255) intensity = 36;
        TogglePlayerVehicleBlur(playerid, newToggle, intensity);
        new msg[144];
        if (newToggle) {
            format(msg, sizeof(msg), "[ExtendedGame] Vehicle motion blur ENABLED (intensity: %d). Drive fast in a vehicle to see the effect!", intensity);
            SendClientMessage(playerid, 0x00FF00AA, msg);
        } else {
            SendClientMessage(playerid, 0xFFFF00AA, "[ExtendedGame] Vehicle motion blur DISABLED.");
        }
        return 1;
    }

    // --- Extended HUD Component Colors Command ---
    if (!strcmp(cmd, "/hudwanted", true)) {
        SetPlayerHUDComponentColor(playerid, HUD_COLOUR_WANTED, 0xFF00FFFF);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Wanted stars color set to Purple.");
        return 1;
    }

    if (!strcmp(cmd, "/hudradio", true)) {
        SetPlayerHUDComponentColor(playerid, HUD_COLOUR_RADIO, 0x00FFFFFF);
        SetPlayerHUDComponentColor(playerid, HUD_COLOUR_RADIO_TUNING, 0xFFA500FF);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Radio text set to Cyan, tuning to Orange.");
        return 1;
    }

    if (!strcmp(cmd, "/hudclock", true)) {
        SetPlayerHUDComponentColor(playerid, HUD_COLOUR_CLOCK, 0x00FF00FF);
        SendClientMessage(playerid, 0x00FF00AA, "[ExtendedGame] Clock text color set to Lime.");
        return 1;
    }

    // --- Pause Menu Status Check Command ---
    if (!strcmp(cmd, "/ispause", true)) {
        new bool:inMenu = IsPlayerInPauseMenu(playerid);
        new msg[128];
        format(msg, sizeof(msg), "[ExtendedGame] You are currently %s the pause menu.", inMenu ? "INSIDE" : "NOT IN");
        SendClientMessage(playerid, inMenu ? 0xFFFF00AA : 0x00FF00AA, msg);
        return 1;
    }

    // --- GPS Navigation Commands ---
    if (!strcmp(cmd, "/gps", true)) {
        if (params[0] == '\0') {
            SendClientMessage(playerid, 0xFFFF00AA, "Usage: /gps <preset | x y z>");
            SendClientMessage(playerid, 0xAAAAAAFF, "  Presets: ls, grove, bank, cityhall, pier, airport, sf, lv, chilliad");
            SendClientMessage(playerid, 0xAAAAAAFF, "  Example: /gps bank   OR   /gps 1543.2 -1675.8 13.5");
            return 1;
        }

        new Float:destX = 0.0, Float:destY = 0.0, Float:destZ = 0.0;
        new presetName[32];
        presetName[0] = '\0';

        if (!strcmp(params, "ls", true) || !strcmp(params, "grove", true)) {
            destX = 2492.0; destY = -1668.0; destZ = 13.3;
            presetName = "Grove Street (LS)";
        } else if (!strcmp(params, "bank", true) || !strcmp(params, "cityhall", true)) {
            destX = 1481.0; destY = -1771.0; destZ = 18.7;
            presetName = "City Hall / LSPD";
        } else if (!strcmp(params, "pier", true)) {
            destX = 380.0; destY = -2072.0; destZ = 7.8;
            presetName = "Santa Maria Pier";
        } else if (!strcmp(params, "airport", true) || !strcmp(params, "lsairport", true)) {
            destX = 1975.0; destY = -2180.0; destZ = 13.5;
            presetName = "Los Santos Airport";
        } else if (!strcmp(params, "sf", true)) {
            destX = -1420.0; destY = -287.0; destZ = 14.1;
            presetName = "San Fierro Station";
        } else if (!strcmp(params, "lv", true)) {
            destX = 2036.0; destY = 1007.0; destZ = 10.8;
            presetName = "The Strip (Las Venturas)";
        } else if (!strcmp(params, "chilliad", true)) {
            destX = -2312.0; destY = -1645.0; destZ = 483.0;
            presetName = "Mount Chiliad Summit";
        } else {
            if (!sscanf_three_floats(params, destX, destY, destZ)) {
                SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Invalid coordinates. Usage: /gps <x y z> or preset name.");
                return 1;
            }
        }

        EXTGAME::SetGPSDestination(playerid, destX, destY, destZ, GPS_FLAG_ALL);
        new msg[144];
        if (presetName[0] != '\0') {
            format(msg, sizeof(msg), "[ExtendedGame] GPS route activated -> %s at (%.1f, %.1f, %.1f). Radar path & 3D marker enabled!", presetName, destX, destY, destZ);
        } else {
            format(msg, sizeof(msg), "[ExtendedGame] GPS route activated -> (%.1f, %.1f, %.1f). Radar path & 3D marker enabled!", destX, destY, destZ);
        }
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    if (!strcmp(cmd, "/cancelgps", true) || !strcmp(cmd, "/nogps", true)) {
        EXTGAME::CancelGPS(playerid);
        SendClientMessage(playerid, 0xFFFF00AA, "[ExtendedGame] GPS Navigation cancelled and radar path cleared.");
        return 1;
    }

    // --- Road Side Check & Wrong-Way Detection ---
    if (!strcmp(cmd, "/roadcheck", true)) {
        new bool:newToggle = !g_DemoRoadCheck[playerid];
        new bool:autoWarn = true;
        new interval = 3000;

        if (params[0] != '\0') {
            new s1[32], s2[32];
            if (sscanf_two_strings(params, s1, s2)) {
                newToggle = (strval(s1) != 0);
                autoWarn = (strval(s2) != 0);
            } else {
                newToggle = (strval(params) != 0);
            }
        }

        g_DemoRoadCheck[playerid] = newToggle;
        EXTGAME::ToggleRoadSideCheck(playerid, newToggle, autoWarn, interval);
        new msg[144];
        if (newToggle) {
            format(msg, sizeof(msg), "[ExtendedGame] Road side detection ENABLED (Auto-warn: %s, Interval: %dms).", autoWarn ? "YES" : "NO", interval);
            SendClientMessage(playerid, 0x00FF00AA, msg);
            SendClientMessage(playerid, 0xAAAAAAFF, "  Driving against traffic will notify the server and trigger alerts!");
        } else {
            SendClientMessage(playerid, 0xFFFF00AA, "[ExtendedGame] Road side detection DISABLED.");
        }
        return 1;
    }

    // --- Traffic Tracing Query ---
    if (!strcmp(cmd, "/traffic", true)) {
        new Float:lookahead = 12.0;
        if (params[0] != '\0') {
            lookahead = floatstr(params);
            if (lookahead < 1.0) lookahead = 1.0;
            if (lookahead > 100.0) lookahead = 100.0;
        }

        EXTGAME::RequestTrafficQuery(playerid, lookahead);
        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Querying traffic & lane status (lookahead: %.1fm)...", lookahead);
        SendClientMessage(playerid, 0x00FFCCAA, msg);
        return 1;
    }

    // --- Custom Road Objects & Virtual Segments ---
    if (!strcmp(cmd, "/spawnroad", true)) {
        if (g_DemoRoadCount[playerid] >= MAX_DEMO_ROAD_OBJECTS) {
            SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Maximum custom demo road objects reached (16). Use /clearroads first.");
            return 1;
        }

        new modelid = 18450; // Road asphalt section model
        if (params[0] != '\0') {
            modelid = strval(params);
            if (modelid <= 0) modelid = 18450;
        }

        new Float:px, Float:py, Float:pz, Float:pa;
        GetPlayerPos(playerid, px, py, pz);
        GetPlayerFacingAngle(playerid, pa);

        // Place object 10m directly ahead of player
        new Float:spawnX = px - (10.0 * floatsin(pa, degrees));
        new Float:spawnY = py + (10.0 * floatcos(pa, degrees));
        new Float:spawnZ = pz - 0.5;

        new objId = CreateObject(modelid, spawnX, spawnY, spawnZ, 0.0, 0.0, pa);
        if (objId == INVALID_OBJECT_ID) {
            SendClientMessage(playerid, 0xFF0000AA, "[ExtendedGame] Failed to create object (invalid model or limit).");
            return 1;
        }

        g_DemoRoadObjects[playerid][g_DemoRoadCount[playerid]++] = objId;

        // Register custom road object with client plugin
        EXTGAME::RegisterCustomRoadObject(playerid, objId, modelid, spawnX, spawnY, spawnZ, 0.0, 0.0, pa, 1, 1, 10.0);

        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Spawned custom road object #%d (Model %d, ObjID %d) at (%.1f, %.1f, %.1f)!",
            g_DemoRoadCount[playerid], modelid, objId, spawnX, spawnY, spawnZ);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        SendClientMessage(playerid, 0xAAAAAAFF, "  Traffic & GPS systems now recognize this object as a 2-lane road!");
        return 1;
    }

    if (!strcmp(cmd, "/spawnsegment", true)) {
        new Float:px, Float:py, Float:pz, Float:pa;
        GetPlayerPos(playerid, px, py, pz);
        GetPlayerFacingAngle(playerid, pa);

        // Start at player pos, extend 30m forward
        new Float:startX = px;
        new Float:startY = py;
        new Float:startZ = pz;
        new Float:endX = px - (30.0 * floatsin(pa, degrees));
        new Float:endY = py + (30.0 * floatcos(pa, degrees));
        new Float:endZ = pz;

        // Register 2-lane virtual road segment
        EXTGAME::RegisterCustomRoadSegment(playerid, startX, startY, startZ, endX, endY, endZ, 1, 1, 4.0);

        new msg[144];
        format(msg, sizeof(msg), "[ExtendedGame] Registered 30m virtual road segment: (%.1f, %.1f) -> (%.1f, %.1f).",
            startX, startY, endX, endY);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        SendClientMessage(playerid, 0xAAAAAAFF, "  Segment added to traffic tracer (1 left lane, 1 right lane, width: 4m).");
        return 1;
    }

    if (!strcmp(cmd, "/clearroads", true)) {
        CleanupPlayerDemoRoads(playerid);
        EXTGAME::ClearCustomRoads(playerid);
        SendClientMessage(playerid, 0xFFFF00AA, "[ExtendedGame] All custom road objects and segments cleared.");
        return 1;
    }

    return 0;
}

public OnPlayerOpenPauseMenu(playerid) {
    printf("[ExtendedGame] Player %d opened the pause menu.", playerid);
    return 1;
}

public OnPlayerClosePauseMenu(playerid) {
    printf("[ExtendedGame] Player %d closed the pause menu.", playerid);
    return 1;
}

public OnPlayerEnterPauseSubmenu(playerid, from, to) {
    printf("[ExtendedGame] Player %d entered pause submenu (from %d to %d).", playerid, from, to);
    return 1;
}

// ==============================================================================
// GPS & Roads Event Callbacks
// ==============================================================================

public OnPlayerGPSEvent(playerid, eventType, Float:remainingDistance) {
    new msg[144];
    switch (eventType) {
        case GPS_EVENT_DESTINATION_REACHED: {
            format(msg, sizeof(msg), "[GPS] You have reached your destination! (Remaining distance: %.1fm)", remainingDistance);
            SendClientMessage(playerid, 0x00FF00AA, msg);
            GameTextForPlayer(playerid, "~g~DESTINATION REACHED!", 4000, 1);
        }
        case GPS_EVENT_REROUTED: {
            format(msg, sizeof(msg), "[GPS] Route recalculating... New remaining distance: %.1fm", remainingDistance);
            SendClientMessage(playerid, 0xFFFF00AA, msg);
        }
        case GPS_EVENT_PATH_NOT_FOUND: {
            SendClientMessage(playerid, 0xFF0000AA, "[GPS] Error: No viable road/path found to destination!");
        }
        default: {
            format(msg, sizeof(msg), "[GPS] Navigation event %d (Distance: %.1fm)", eventType, remainingDistance);
            SendClientMessage(playerid, 0x00FFFFFF, msg);
        }
    }
    return 1;
}

public OnPlayerRoadStatusChange(playerid, status, closestNode, Float:distanceToRoad, Float:roadAngle, Float:vehicleAngle) {
    new msg[144];
    switch (status) {
        case ROAD_STATUS_ON_RIGHT_SIDE: {
            format(msg, sizeof(msg), "[Traffic] Correct lane | Node: %d | Dist: %.1fm | Angle diff: %.1f deg",
                closestNode, distanceToRoad, floatabs(vehicleAngle - roadAngle));
            SendClientMessage(playerid, 0x33CC33AA, msg);
        }
        case ROAD_STATUS_ON_WRONG_SIDE: {
            format(msg, sizeof(msg), "[Traffic] WRONG WAY WARNING! Against traffic flow | Node: %d | Dist: %.1fm",
                closestNode, distanceToRoad);
            SendClientMessage(playerid, 0xFF3333AA, msg);
            GameTextForPlayer(playerid, "~r~WRONG WAY!", 2000, 5);
        }
        case ROAD_STATUS_ON_INTERSECTION: {
            format(msg, sizeof(msg), "[Traffic] Entering/Inside INTERSECTION | Node: %d | Dist: %.1fm",
                closestNode, distanceToRoad);
            SendClientMessage(playerid, 0xFFFF00AA, msg);
        }
        case ROAD_STATUS_NO_CONNECTING_NODE: {
            format(msg, sizeof(msg), "[Traffic] Isolated road node (%d) - No connecting link found.", closestNode);
            SendClientMessage(playerid, 0xAAAAAAFF, msg);
        }
        case ROAD_STATUS_NO_NAVI_FOUND: {
            format(msg, sizeof(msg), "[Traffic] Off-road or no navigation node nearby (Dist: %.1fm).", distanceToRoad);
            SendClientMessage(playerid, 0x888888FF, msg);
        }
        case ROAD_STATUS_OFF_ROAD: {
            format(msg, sizeof(msg), "[Traffic] Off-road / Parking / Garage (Dist: %.1fm).", distanceToRoad);
            SendClientMessage(playerid, 0x888888FF, msg);
        }
        default: {
            format(msg, sizeof(msg), "[Traffic] Road status change: %d (Node: %d, Dist: %.1fm)", status, closestNode, distanceToRoad);
            SendClientMessage(playerid, 0xCCCCCCFF, msg);
        }
    }
    return 1;
}

public OnPlayerTrafficQuery(playerid, status, side, laneIndex, totalLanes, Float:lateralDeviation, Float:targetX, Float:targetY, Float:targetZ, Float:inclineDeg, bool:isCustomRoad, customObjectId) {
    new msg[144];
    new sideStr[24];
    if (side == ROAD_SIDE_RIGHT) sideStr = "Right (Correct)";
    else if (side == ROAD_SIDE_LEFT) sideStr = "Left (Wrong)";
    else sideStr = "Unknown/Middle";

    format(msg, sizeof(msg), "[Traffic Query] Side: %s | Lane: %d/%d | Deviation: %.2fm | Incline: %.1f deg",
        sideStr, laneIndex, totalLanes, lateralDeviation, inclineDeg);
    SendClientMessage(playerid, 0x00FFCCAA, msg);

    if (isCustomRoad) {
        format(msg, sizeof(msg), "  Custom Road: YES (ObjID: %d) | Target: (%.1f, %.1f, %.1f)",
            customObjectId, targetX, targetY, targetZ);
    } else {
        format(msg, sizeof(msg), "  Official Road: YES | Target: (%.1f, %.1f, %.1f)",
            targetX, targetY, targetZ);
    }
    SendClientMessage(playerid, 0x00FFCCAA, msg);
    return 1;
}
