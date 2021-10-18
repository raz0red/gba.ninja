#import <emscripten.h>
#include <stdarg.h>
#include <string.h>
#include <cstddef>
#import "../System.h"
#import "../Util.h"
#import "../common/ConfigManager.h"
#import "../gba/GBA.h"
#import "../gba/RTC.h"
#import "../gba/Sound.h"
#import "../gba/agbprint.h"
#import "EmscriptenSoundDriver.h"

#define ENTRY_FN extern "C" int EMSCRIPTEN_KEEPALIVE

bool enableRtc;

typedef struct {
    char romtitle[256];
    char romid[5];
    int flashSize;
    int saveType;
    int rtcEnabled;
    int mirroringEnabled;
    int useBios;
} ini_t;

static const ini_t gbaover[256] = {
			//romtitle,							    	romid	flash	save	rtc	mirror	bios
			{"2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)",	"BLFE",	0,	1,	0,	0,	0},
			{"2 Games in 1 - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA)", "BUFE", 0, 1, 0, 0, 0},
			{"Boktai - The Sun Is in Your Hand (Europe)(En,Fr,De,Es,It)",		"U3IP",	0,	0,	1,	0,	0},
			{"Boktai - The Sun Is in Your Hand (USA)",				"U3IE",	0,	0,	1,	0,	0},
			{"Boktai 2 - Solar Boy Django (USA)",					"U32E",	0,	0,	1,	0,	0},
			{"Boktai 2 - Solar Boy Django (Europe)(En,Fr,De,Es,It)",		"U32P",	0,	0,	1,	0,	0},
			{"Bokura no Taiyou - Taiyou Action RPG (Japan)",			"U3IJ",	0,	0,	1,	0,	0},
			{"Card e-Reader+ (Japan)",						"PSAJ",	131072,	0,	0,	0,	0},
			{"Classic NES Series - Bomberman (USA, Europe)",			"FBME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Castlevania (USA, Europe)",			"FADE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Donkey Kong (USA, Europe)",			"FDKE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Dr. Mario (USA, Europe)",			"FDME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Excitebike (USA, Europe)",			"FEBE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Legend of Zelda (USA, Europe)",			"FZLE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Ice Climber (USA, Europe)",			"FICE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Metroid (USA, Europe)",				"FMRE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Pac-Man (USA, Europe)",				"FP7E",	0,	1,	0,	1,	0},
			{"Classic NES Series - Super Mario Bros. (USA, Europe)",		"FSME",	0,	1,	0,	1,	0},
			{"Classic NES Series - Xevious (USA, Europe)",				"FXVE",	0,	1,	0,	1,	0},
			{"Classic NES Series - Zelda II - The Adventure of Link (USA, Europe)",	"FLBE",	0,	1,	0,	1,	0},
			{"Digi Communication 2 - Datou! Black Gemagema Dan (Japan)",		"BDKJ",	0,	1,	0,	0,	0},
			{"e-Reader (USA)",							"PSAE",	131072,	0,	0,	0,	0},
			{"Dragon Ball GT - Transformation (USA)",				"BT4E",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Buu's Fury (USA)",					"BG3E",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Taiketsu (Europe)(En,Fr,De,Es,It)",			"BDBP",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - Taiketsu (USA)",					"BDBE",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II International (Japan)",		"ALFJ",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II (Europe)(En,Fr,De,Es,It)",	"ALFP", 0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku II (USA)",				"ALFE",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy Of Goku (Europe)(En,Fr,De,Es,It)",		"ALGP",	0,	1,	0,	0,	0},
			{"Dragon Ball Z - The Legacy of Goku (USA)",				"ALGE",	131072,	1,	0,	0,	0},
			{"F-Zero - Climax (Japan)",						"BFTJ",	131072,	0,	0,	0,	0},
			{"Famicom Mini Vol. 01 - Super Mario Bros. (Japan)",			"FMBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 12 - Clu Clu Land (Japan)",				"FCLJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 13 - Balloon Fight (Japan)",			"FBFJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 14 - Wrecking Crew (Japan)",			"FWCJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 15 - Dr. Mario (Japan)",				"FDMJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 16 - Dig Dug (Japan)",				"FTBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 17 - Takahashi Meijin no Boukenjima (Japan)",	"FTBJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 18 - Makaimura (Japan)",				"FMKJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 19 - Twin Bee (Japan)",				"FTWJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 20 - Ganbare Goemon! Karakuri Douchuu (Japan)",	"FGGJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 21 - Super Mario Bros. 2 (Japan)",			"FM2J",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 22 - Nazo no Murasame Jou (Japan)",			"FNMJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 23 - Metroid (Japan)",				"FMRJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 24 - Hikari Shinwa - Palthena no Kagami (Japan)",	"FPTJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 25 - The Legend of Zelda 2 - Link no Bouken (Japan)","FLBJ",0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 26 - Famicom Mukashi Banashi - Shin Onigashima - Zen Kou Hen (Japan)","FFMJ",0,1,0,	1,	0},
			{"Famicom Mini Vol. 27 - Famicom Tantei Club - Kieta Koukeisha - Zen Kou Hen (Japan)","FTKJ",0,1,0,	1,	0},
			{"Famicom Mini Vol. 28 - Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo - Zen Kou Hen (Japan)","FTUJ",0,1,0,1,0},
			{"Famicom Mini Vol. 29 - Akumajou Dracula (Japan)",			"FADJ",	0,	1,	0,	1,	0},
			{"Famicom Mini Vol. 30 - SD Gundam World - Gachapon Senshi Scramble Wars (Japan)","FSDJ",0,1,	0,	1,	0},
			{"Game Boy Wars Advance 1+2 (Japan)",					"BGWJ",	131072,	0,	0,	0,	0},
			{"Golden Sun - The Lost Age (USA)",					"AGFE",	65536,	0,	0,	1,	0},
			{"Golden Sun (USA)",							"AGSE",	65536,	0,	0,	1,	0},
			{"Koro Koro Puzzle - Happy Panechu! (Japan)",				"KHPJ",	0,	4,	0,	0,	0},
			{"Mario vs. Donkey Kong (Europe)",					"BM5P",	0,	3,	0,	0,	0},
			{"Pocket Monsters - Emerald (Japan)",					"BPEJ",	131072,	0,	1,	0,	0},
			{"Pocket Monsters - Fire Red (Japan)",					"BPRJ",	131072,	0,	0,	0,	0},
			{"Pocket Monsters - Leaf Green (Japan)",				"BPGJ",	131072,	0,	0,	0,	0},
			{"Pocket Monsters - Ruby (Japan)",					"AXVJ",	131072,	0,	1,	0,	0},
			{"Pocket Monsters - Sapphire (Japan)",					"AXPJ",	131072,	0,	1,	0,	0},
			{"Pokemon Mystery Dungeon - Red Rescue Team (USA, Australia)",		"B24E",	131072,	0,	0,	0,	0},
			{"Pokemon Mystery Dungeon - Red Rescue Team (En,Fr,De,Es,It)",		"B24P",	131072,	0,	0,	0,	0},
			{"Pokemon - Blattgruene Edition (Germany)",				"BPGD",	131072,	0,	0,	0,	0},
			{"Pokemon - Edicion Rubi (Spain)",					"AXVS",	131072,	0,	1,	0,	0},
			{"Pokemon - Edicion Esmeralda (Spain)",					"BPES",	131072,	0,	1,	0,	0},
			{"Pokemon - Edicion Rojo Fuego (Spain)",				"BPRS",	131072,	1,	0,	0,	0},
			{"Pokemon - Edicion Verde Hoja (Spain)",				"BPGS",	131072,	1,	0,	0,	0},
			{"Pokemon - Eidicion Zafiro (Spain)",					"AXPS",	131072,	0,	1,	0,	0},
			{"Pokemon - Emerald Version (USA, Europe)",				"BPEE",	131072,	0,	1,	0,	0},
			{"Pokemon - Feuerrote Edition (Germany)",				"BPRD",	131072,	0,	0,	0,	0},
			{"Pokemon - Fire Red Version (USA, Europe)",				"BPRE",	131072,	0,	0,	0,	0},
			{"Pokemon - Leaf Green Version (USA, Europe)",				"BPGE",	131072,	0,	0,	0,	0},
			{"Pokemon - Rubin Edition (Germany)",					"AXVD",	131072,	0,	1,	0,	0},
			{"Pokemon - Ruby Version (USA, Europe)",				"AXVE",	131072,	0,	1,	0,	0},
			{"Pokemon - Sapphire Version (USA, Europe)",				"AXPE",	131072,	0,	1,	0,	0},
			{"Pokemon - Saphir Edition (Germany)",					"AXPD",	131072,	0,	1,	0,	0},
			{"Pokemon - Smaragd Edition (Germany)",					"BPED",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Emeraude (France)",					"BPEF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Rouge Feu (France)",				"BPRF",	131072,	0,	0,	0,	0},
			{"Pokemon - Version Rubis (France)",					"AXVF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Saphir (France)",					"AXPF",	131072,	0,	1,	0,	0},
			{"Pokemon - Version Vert Feuille (France)",				"BPGF",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Rubino (Italy)",					"AXVI",	131072,	0,	1,	0,	0},
			{"Pokemon - Versione Rosso Fuoco (Italy)",				"BPRI",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Smeraldo (Italy)",					"BPEI",	131072,	0,	1,	0,	0},
			{"Pokemon - Versione Verde Foglia (Italy)",				"BPGI",	131072,	0,	0,	0,	0},
			{"Pokemon - Versione Zaffiro (Italy)",					"AXPI",	131072,	0,	1,	0,	0},
			{"Rockman EXE 4.5 - Real Operation (Japan)",				"BR4J",	0,	0,	1,	0,	0},
			{"Rocky (Europe)(En,Fr,De,Es,It)",					"AROP",	0,	1,	0,	0,	0},
			{"Rocky (USA)(En,Fr,De,Es,It)",						"AR8e",	0,	1,	0,	0,	0},
			{"Sennen Kazoku (Japan)",						"BKAJ",	131072,	0,	1,	0,	0},
			{"Shin Bokura no Taiyou - Gyakushuu no Sabata (Japan)",			"U33J",	0,	1,	1,	0,	0},
			{"Super Mario Advance 4 (Japan)",					"AX4J",	131072,	0,	0,	0,	0},
			{"Super Mario Advance 4 - Super Mario Bros. 3 (Europe)(En,Fr,De,Es,It)","AX4P",	131072,	0,	0,	0,	0},
			{"Super Mario Advance 4 - Super Mario Bros 3 - Super Mario Advance 4 v1.1 (USA)","AX4E",131072,0,0,0,0},
			{"Top Gun - Combat Zones (USA)(En,Fr,De,Es,It)",			"A2YE",	0,	5,	0,	0,	0},
			{"Yoshi's Universal Gravitation (Europe)(En,Fr,De,Es,It)",		"KYGP",	0,	4,	0,	0,	0},
			{"Yoshi no Banyuuinryoku (Japan)",					"KYGJ",	0,	4,	0,	0,	0},
			{"Yoshi - Topsy-Turvy (USA)",						"KYGE",	0,	1,	0,	0,	0},
			{"Yu-Gi-Oh! GX - Duel Academy (USA)",					"BYGE",	0,	2,	0,	0,	1},
			{"Yu-Gi-Oh! - Ultimate Masters - 2006 (Europe)(En,Jp,Fr,De,Es,It)",	"BY6P",	0,	2,	0,	0,	0},
			{"Zoku Bokura no Taiyou - Taiyou Shounen Django (Japan)",		"U32J",	0,	0,	1,	0,	0}
};

void log(const char* format, ...) {
    // EM_ASM(debugger;);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}

static void load_image_preferences(void) {
    char buffer[5];
    buffer[0] = rom[0xac];
    buffer[1] = rom[0xad];
    buffer[2] = rom[0xae];
    buffer[3] = rom[0xaf];
    buffer[4] = 0;

    log("GameID in ROM is: %s %d\n", buffer, strlen(buffer));

    bool found = false;
    int found_no = 0;

    if (strlen(buffer) > 0) {
        for (int i = 0; i < 256; i++) {
            if (!strcmp(gbaover[i].romid, buffer)) {
                found = true;
                found_no = i;
                break;
            }
        }
    }

    if (found) {
        log("Found ROM in vba-over list: %d.\n", found_no);

        enableRtc = gbaover[found_no].rtcEnabled;

        if (gbaover[found_no].flashSize != 0)
            flashSize = gbaover[found_no].flashSize;
        else
            flashSize = 65536;

        cpuSaveType = gbaover[found_no].saveType;

        mirroringEnable = gbaover[found_no].mirroringEnabled;
    }

    log("RTC = %d.\n", enableRtc);
    log("flashSize = %d.\n", flashSize);
    log("cpuSaveType = %d.\n", cpuSaveType);
    log("mirroringEnable = %d.\n", mirroringEnable);
}

bool systemPauseOnFrame() {
    return false;
}

void systemGbPrint(u8*, int, int, int, int, int) {
    printf("System GB Print\n");
}

void systemScreenCapture(int num) {}

void systemDrawScreen() {
    // This is not the frame ready callback
}

bool systemReadJoypads() {
    // This indicates that we should prepare for systemReadJoypad, but we don't
    // care, we're always prepared for it.
    return true;
}

u32 systemReadJoypad(int joypadNum) {
    return EM_ASM_INT({return window["VBAInterface"]["getJoypad"]($0)},
                      joypadNum);
}

u32 systemGetClock() {
    return emscripten_get_now();
}

void systemMessage(int code, const char* format, ...) {
    printf("System Message (%d):\n", code);
    // EM_ASM(debugger;);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}

void systemSetTitle(const char* title) {
    // Not used
}

SoundDriver* systemSoundInit() {
    soundShutdown();
    return new EmscriptenSoundDriver();
}

void systemOnWriteDataToSoundBuffer(const u16* finalWave, int length) {
    // This is used for recording sound, not for realtime sound.
}

void systemOnSoundShutdown() {
    // Don't need this callback, the EmscriptenSoundDriver has an identical
    // callback.
}

void systemScreenMessage(const char* msg) {
    printf("System Screen Message: %s\n", msg);
}

void systemUpdateMotionSensor() {}

int systemGetSensorX() {
    return 0;
}

int systemGetSensorY() {
    return 0;
}

int systemGetSensorZ() {
    return 0;
}

u8 systemGetSensorDarkness() {
    return 0xE8;  // Not sure why, but this is what the other frontends use...
}

void systemCartridgeRumble(bool) {}

void systemPossibleCartridgeRumble(bool) {}

void updateRumbleFrame() {}

bool systemCanChangeSoundQuality() {
    return false;
}

void systemShowSpeed(int n) {
    // Don't care about VBA's calculated speed. I can calculate it better.
}

void system10Frames(int always60) {
    // I have no idea what this is for...
}

void systemFrame() {
    // This is called when the emulator wants to submit a frame.
    EM_ASM_INT({return window["VBAInterface"]["renderFrame"]($0)}, (int)pix);
}

void systemGbBorderOn() {
    // Don't care
}

void Sm60FPS_Init() {
    // Don't know what this is
}

bool Sm60FPS_CanSkipFrame() {
    // Don't know what this is
    return false;
}

void Sm60FPS_Sleep() {
    // Don't know what this is
}

void DbgMsg(const char* msg, ...) {
    EM_ASM(debugger;);
    //    va_list argptr;
    //    va_start(argptr, format);
    //    vfprintf(stderr, format, argptr);
    //    va_end(argptr);
}

void (*dbgOutput)(const char* s, u32 addr);
void _dbgOutput(const char* s, u32 addr) {
    EM_ASM_INT({return window["VBAInterface"]["dbgOutput"]($0, $1)}, (int)s,
               (int)addr);
}

u16 systemColorMap16[0x10000];  // This gets filled by utilUpdateSystemColorMaps
                                // in VBA_start
u32 systemColorMap32[0x10000];  // This gets filled by utilUpdateSystemColorMaps
                                // in VBA_start
u16 systemGbPalette[24];        // This gets filled by settings

// This is to convert to RGBA5551
int systemColorDepth = 16;
int systemRedShift = 11;
int systemGreenShift = 6;
int systemBlueShift = 1;

int systemFrameSkip = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

int systemVerbose = 1;

int emulating = 0;

struct EmulatedSystem emulator = {NULL, NULL, NULL, NULL, NULL, NULL,  NULL,
                                  NULL, NULL, NULL, NULL, NULL, false, 0};

ENTRY_FN VBA_start(int inFlashSize, int inSaveType, int inRtc, int inMirroring) {
    cpuSaveType = inSaveType >= 0 ? inSaveType : 0;
    flashSize = inFlashSize >= 0 ? inFlashSize : 0x10000;
    enableRtc = inRtc == 1 ? true : false;
    mirroringEnable = inMirroring == 1 ? true : false;    

    // Misc setup
    dbgOutput = _dbgOutput;
    utilUpdateSystemColorMaps(false);    
    agbPrintEnable(true);

    int size = CPULoadRomData(
        NULL, EM_ASM_INT({return window["VBAInterface"]["getRomSize"]()}, 0));

    if (cpuSaveType == 0)
        utilGBAFindSave(size);
    else
        saveType = cpuSaveType;

   load_image_preferences();

   if(flashSize == 0x10000 || flashSize == 0x20000) {
       printf("## setting flash size! : %d\n", flashSize);
      flashSetSize(flashSize);
   }

   if(enableRtc)
      rtcEnable(enableRtc);

    doMirroring(mirroringEnable);

    soundSetSampleRate(
        EM_ASM_INT({return window["VBAInterface"]["getAudioSampleRate"]()}, 0));    
    soundInit();

    emulator = GBASystem;
    emulator.emuReadBattery("");    

    CPUInit(0, useBios);
    CPUReset();

    emulating = 1;

    return 1;
}

ENTRY_FN VBA_get_emulating() {
    return emulating;
}

ENTRY_FN VBA_stop() {
    CPUReset();
    CPUCleanUp();
    return 1;
}

ENTRY_FN VBA_do_cycles(int cycles) {
    emulator.emuMain(cycles);
    return 1;
}

ENTRY_FN VBA_get_bios() {
    return (int)bios;
}

ENTRY_FN VBA_get_rom() {
    return (int)rom;
}

ENTRY_FN VBA_get_internalRAM() {
    return (int)internalRAM;
}

ENTRY_FN VBA_get_workRAM() {
    return (int)workRAM;
}

ENTRY_FN VBA_get_paletteRAM() {
    return (int)paletteRAM;
}

ENTRY_FN VBA_get_vram() {
    return (int)vram;
}

ENTRY_FN VBA_get_pix() {
    return (int)pix;
}

ENTRY_FN VBA_get_oam() {
    return (int)oam;
}

ENTRY_FN VBA_get_ioMem() {
    return (int)ioMem;
}

ENTRY_FN VBA_get_systemColorMap16() {
    return (int)systemColorMap16;
}

ENTRY_FN VBA_get_systemColorMap32() {
    return (int)systemColorMap32;
}

ENTRY_FN VBA_get_systemFrameSkip() {
    return (int)systemFrameSkip;
}

ENTRY_FN VBA_set_systemFrameSkip(int frameSkip) {
    systemFrameSkip = frameSkip;
    return 1;
}

ENTRY_FN VBA_get_systemSaveUpdateCounter() {
    return systemSaveUpdateCounter;
}

ENTRY_FN VBA_reset_systemSaveUpdateCounter() {
    systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
    return 1;
}

ENTRY_FN VBA_emuWriteBattery() {
    emulator.emuWriteBattery("");
    return 1;
};

ENTRY_FN VBA_agbPrintFlush() {
    agbPrintFlush();
    return 1;
};
