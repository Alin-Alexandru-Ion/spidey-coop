#include "non_win32.h"

// #define BOOT_GAME
#define MODEL_PREVIEW

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

// #define LOCK_VALIDATION

#include "main.h"
#include "my_assert.h"
#include "ob.h"
#include "vector.h"
#include "friction.h"
#include "bit.h"
#include "front.h"
#include "pshell.h"
#include "baddy.h"
#include "mj.h"
#include "submarin.h"
#include "venom.h"
#include "ps2funcs.h"
#include "blackcat.h"
#include "torch.h"
#include "hostage.h"
#include "cop.h"
#include "carnage.h"
#include "chopper.h"
#include "docock.h"
#include "jonah.h"
#include "lizard.h"
#include "lizman.h"
#include "mysterio.h"
#include "platform.h"
#include "rhino.h"
#include "scorpion.h"
#include "simby.h"
#include "spclone.h"
#include "superock.h"
#include "thug.h"
#include "turret.h"
#include "shell.h"
#include "web.h"
#include "bit2.h"
#include "camera.h"
#include "quat.h"
#include "mem.h"
#include "exp.h"
#include "m3dcolij.h"
#include "m3dinit.h"
#include "spidey.h"
#include "message.h"
#include "bullet.h"
#include "trig.h"
#include "effects.h"
#include "FontTools.h"
#include "wire.h"
#include "powerup.h"
#include "switch.h"
#include "chain.h"
#include "Image.h"
#include "ps2pad.h"
#include "bitmap256.h"
#include "PCTex.h"
void CoOpUiRemakeScan(void);
bool CoOpUiRemakeIsHeadTexture(i32 textureId);
#include "smoke.h"
#include "panel.h"
#include "manipob.h"
#include "mess.h"
#include "ai.h"
#include <cstring>
#include "spool.h"
#include "l1a3bomb.h"
#include "chunk.h"
#include "weapons.h"
#include "backgrnd.h"
#include "dcshellutils.h"
#include "pkr.h"
#include "pcdcFile.h"
#include "ps2lowsfx.h"
#include "PCInput.h"
#include "PCShell.h"
#include "stubs.h"
#include "SpideyDX.h"
#include "DXsound.h"
#include "DXinit.h"
#include "pack.h"
#include "pal.h"
#include "db.h"
#include "ps2m3d.h"
#include "PCGfx.h"
#include "ps2gamefmv.h"
#include "init.h"
#include "utils.h"
#include "reloc.h"
#include "my_bink.h"
#include "pcdcMem.h"
#include "dcmemcard.h"
#include "ps2card.h"
#include "pcdcBkup.h"
#include "pcdcPad.h"
// Co-op integration: the original game translation unit remains the hook host.
#include "coop/core/coop_players.h"
#include "coop/camera/coop_view.h"
#include "coop/input/coop_input.h"
#include "coop/world/coop_activation.h"
#include "coop/world/coop_environment.h"
#include "vram.h"
#include "m3dzone.h"
#include "PRE.h"
#include "dcfileio.h"
#include "PCMovie.h"
#include "flash.h"


#include "my_patch.h"
#include "coop/presentation/coop_resolution_runtime.h"

extern int FAIL_VALIDATION;

const i32 POLYBUFFERSIZE = 0x17000;

EXPORT i32 gMainStuff[0x1000];

// @Ok
// @Matching
void CalcPolyBufferEnd(void)
{
	PolyBufferEnd = reinterpret_cast<u8*>(
			(reinterpret_cast<u32>(pDoubleBuffer->Polys) + POLYBUFFERSIZE - 0x100) & 0x7FFFFFFF);
}

// @MEDIUMTODO
void SpideyMain(void)
{
	DXERR_printf("xxx main\n");
	for (i32 i = 0; i < 0x1000; i++)
	{
		gMainStuff[i] = 0x4B415453;
	}

	gMainStuff[0] = 0x544C4148;

	Init_AtStart(1);
	PCTex_LoadPcIcons();
	GameFMV_PlayMovie(0, 1, 1, 2.5f);
	GameFMV_PlayMovie(1, 1, 1, 1.0f);
	GameFMV_PlayMovie(2, 1, 1, 1.0f);
	GameFMV_PlayMovie(3, 1, 1, 1.0f);

	Init_Cleanup(0);
	gRunCinemaRelated = 0;

	while (gVlanksRelated)
		;

#ifndef MODEL_PREVIEW
	if (gRenderTest & 8)
#else
	if(1)
#endif
	{
		Spool_ClearAllPSXs();
		PCGfx_DoModelPreview();
		Init_Cleanup(0);
	}
	else
	{
	}
}

// @Ok
// @Matching
// @Leak
void* CClass::operator new(size_t size)
{
	void *pnew = Mem_New(size);

	// Ensure size is a multiple of 4.
	size = ( size + 3 ) & ~0x03;

	// Zero all the newly allocated memory
	u32 *p=(u32 *)pnew;
	for (i32 i=0; i<size/4; ++i) *p++=0;

	return pnew;
}

// @Ok
void CClass::operator delete(void *ptr)
{
	Mem_Delete(ptr);
}

// @Ok
CClass::~CClass()
{
}

template<bool b>

struct StaticAssert{};



template<>

struct StaticAssert<true>

{

	static void assert() {}
};

// @Bogus
void compile_time_assertions(){
	StaticAssert<sizeof(CVector)==12>::assert();
	StaticAssert<sizeof(CFriction)==3>::assert();
	//StaticAssert<sizeof(CBit) == 0x38>::assert();

	//StaticAssert<sizeof(CMenu)==0x53C>::assert();

	//StaticAssert<sizeof(CExpandingBox)==52>::assert();

	StaticAssert<sizeof(CSVector)==6>::assert();

	StaticAssert<sizeof(SVector)==6>::assert();

	StaticAssert<sizeof(CQuadBit)==0x84>::assert();

	//StaticAssert<sizeof(CMJ)==0x324>::assert();

	StaticAssert<sizeof(MATRIX)==0x20>::assert();

	StaticAssert<sizeof(u32)==4>::assert();
	StaticAssert<sizeof(u16)==2>::assert();
	StaticAssert<sizeof(u8)==1>::assert();

	StaticAssert<sizeof(i32)==4>::assert();
	StaticAssert<sizeof(i16)==2>::assert();
	StaticAssert<sizeof(i8)==1>::assert();
}

// @Bogus
extern "C" EXPORT int run_assertions(void)
{
	puts("[*] Starting validation");



	validate_CItem();
	validate_CVector();
	validate_CSVector();
	validate_CBit();
	validate_CQuadBit();
	validate_CFT4Bit();
	validate_CFlatBit();
	validate_CBody();
	validate_SVector();
	validate_CSuper();
	validate_CBaddy();
	validate_CMJ();
	validate_CSubmariner();
	validate_CVenom();
	validate_CBlackCat();
	validate_CTorch();
	validate_CHostage();
	validate_CScriptOnlyBaddy();
	validate_CCop();
	validate_CCarnage();
	validate_CChopper();
	validate_CDocOc();
	validate_CJonah();
	validate_CLizard();
	validate_CLizMan();
	validate_CMystFoot();
	validate_CMysterio();
	validate_CSoftSpot();
	validate_CPlatform();
	validate_CRhino();
	validate_CScorpion();
	validate_CPunchOb();
	validate_CSimby();
	validate_CSimbyBase();
	validate_CSpClone();
	validate_CSuperDocOck();
	validate_CThug();
	validate_CTurret();
	validate_MATRIX();
	validate_SMatrix();
	validate_SJoint();
	validate_CRudeWordHitterSpidey();
	validate_CBulletFrag();
	validate_CImpactWeb();
	validate_CDomePiece();
	validate_CDome();
	validate_CDomeRing();
	validate_CWeb();
	validate_CSwinger();
	validate_CTurretBase();
	validate_CDummy();
	validate_CSniperSplat();
	validate_SStateFlags();
	validate_CGPolyLine();
	validate_CCamera();
	validate_CQuat();
	validate_SBlockHeader();
	validate_SHandle();
	validate_CItemFrag();
	validate_SLineInfo();
	validate_STexWibItemInfo();
	validate_CPlayer();
	validate_CSmokeTrail();
	validate_CMessage();
	validate_CTrapWebEffect();
	validate_CMenu();
	validate_SEntry();
	validate_CBullet();
	validate_SLinkInfo();
	validate_CElectrify();
	validate_CSimbySlimeBase();
	validate_CMysterioLaser();
	validate_Font();
	validate_CTurretLaser();
	validate_CLaserFence();
	validate_CGoldFish();
	validate_CPowerUp();
	validate_CSwitch();
	validate_CChain();
	validate_CGLine();
	validate_SlicedImage2();
	validate_Image();
	validate_SControl();
	validate_Bitmap256();
	validate_SPCTexture();
	validate_CPolyLine();
	validate_CSonicBubble();
	validate_CGlow();
	validate_CLinked2EndedBit();
	validate_CRibbonBit();
	validate_CSniperTarget();
	validate_CVenomWrap();
	validate_CSmokeJet();
	validate_CTexturedRibbon();
	validate_CDomeShockWave();
	validate_CMysterioHeadCircle();
	validate_SAnimFrame();
	validate_CFadePalettes();
	validate_CSimpleTexturedRibbon();
	validate_CManipOb();
	validate_SimpleMessage();
	validate_CShellMysterioHeadGlow();
	validate_CWobblyGlow();
	validate_CSimpleAnim();
	validate_CCopPing();
	validate_SHook();
	validate_Spidey_CIcon();
	validate_CEmber();
	validate_CThugPing();
	validate_CAIProc();
	validate_CAIProc_LookAt();
	validate_Texture();
	validate_CRhinoNasalSteam();
	validate_CAIProc_RotY();
	validate_CAIProc_Fall();
	validate_CAIProc_StateSwitchSendMessage();
	validate_CAIProc_MonitorAttack();
	validate_CAIProc_AccZ();
	validate_SMoveToInfo();
	validate_CAIProc_MoveTo();
	validate_CNonRenderedBit();
	validate_SPSXRegion();
	validate_CSimbyShot();
	validate_CVenomElectrified();
	validate_CCarnageElectrified();
	validate_CConstantLaser();
	validate_CShellSymBurn();
	validate_CExpandingBox();
	validate_CL1A3Bomb();
	validate_CMotionBlur();
	validate_SHitInfo();
	validate_SCommandPoint();
	validate_PendingListEntry();
	validate_CSpecialDisplay();
	validate_CSkidMark();
	validate_TextureEntry();
	validate_CShellVenomElectrified();
	validate_CSkinGoo();
	validate_SSkinGooSource();
	validate_SSkinGooSource2();
	validate_SSkinGooParams();
	validate_CShellCarnageElectrified();
	validate_CShellSuperDocOckElectrified();
	validate_CShellRhinoNasalSteam();
	validate_CShellEmber();
	validate_CShellSimbyMeltSplat();
	validate_CShellSimbyFireDeath();
	validate_CShellGoldFish();
	validate_CShellMysterioHeadCircle();
	validate_SpideyIconRelated();
	validate_CGlowFlash();
	validate_SChainData();
	validate_CSearchlight();
	validate_SFlatBitVelocity();
	validate_CMachineGunBullet();
	validate_CChopperMissile();
	validate_CChunkControl();
	validate_SChunkEntry();
	validate_CGouraudRibbon();
	validate_CCopBulletTracer();
	validate_CCombatImpactRing();
	validate_SCamera();
	validate_SRibbonPoint();
	validate_CRhinoWallImpact();
	validate_CFootprint();
	validate_CChunkSmoke();
	validate_CBouncingRock();
	validate_CFlameExplosion();
	validate_CFrag();
	validate_CPixel();
	validate_CFireySpark();
	validate_CSimbyDroplet();
	validate_CSymBurn();
	validate_CBackground();
	validate_CAngrySpark();
	validate_CBitServer();
	validate_CCarnageHitSpark();
	validate_CChunkBit();
	validate_CTextBox();
	validate_CCopLaserPing();
	validate_CDamagedSoftSpotEffect();
	validate_CElectro();
	validate_CElectroLine();
	validate_CFireyExplosion();
	validate_CFlamingImpactWeb();
	validate_CTripWire();
	validate_CSmokeRing();
	validate_CTexturedRibbon();
	validate_SLineSeg();
	validate_CWibbly();
	validate_SSmokeRingRelated();
	validate_Sprite2();
	validate_SBitServerEntry();
	validate_PKR_FILEINFO();
	validate_PKR_FOOTER();
	validate_PKR_DIRINFO();
	validate_LIBPKR_HANDLE();
	validate_NODE_DIRINFO();
	validate_PVRHeader();
	validate_ClutPC();
	validate_PKR_HEADER();
	validate_SGDOpenFile();
	validate_NODE_FILEINFO();
	validate_SSFXBank();
	validate_SMapping();
	validate_SActionMap();
	validate_SSaveGame();
	validate_MEMORY_ALLOC();
	validate_SMessageProg();
	validate_SLevel();
	validate_SMessage();
	validate_DXsound();
	validate_DXContext();
	validate_DXContextEntry();
	validate_SVideoMode();
	validate_DXVideoModeContext();
	validate_DxZBufferContext();
	validate_DXPOLY();
	validate_SFontEntry();
	validate_SDataGlyph();
	validate_POLY_FT4();
	validate_POLY_GT4();
	validate_SPack();
	validate_tag_S_Pal();
	validate_SViewport();
	validate_SDoubleBuffer();
	validate_SDXPolyField();
	validate_SPCTexPixelFormat();
	validate_SPCTexContainer();
	validate_SAccess();
	validate_AnimPacket();
	validate_SCalcBuffer();
	validate_SCheat();
	validate_SButton();
	validate_DDPIXELFORMAT();
	validate_ConvertPSXPaletteToPC();
	validate_BmpHeader();
	validate_Load8BitBMP2();
	validate_CWibbling3DExplosion();
	validate_C3DExplosion();
	validate_CGrenadeWave();
	validate_CGrenadeExplosion();
	validate_CRipple();
	validate_SSection();
	validate_SFringeQuad();
	validate_SModel();
	validate_SMessageData();
	validate_SSfxEntry();
	validate_reloc_mod();
	validate_SReloc();
	validate_SRelocEntry();
	validate_SMovieDetails();
	validate_BINKSUMMARY();
	validate_BINK();
	validate_matrix4x4();
	validate_vector3d();
	validate_vector4d();
	validate_SScore();
	validate_SRecords();
	validate_SRecordRelated();
	validate_SDCCardTime();
	validate_SCardHead();
	validate_SBackupFile();
	validate_SSaveFile();
	validate_SDCCardFullTime();
	validate_SPdPadBig();
	validate_SPdPadSmall();
	validate_tagSVRAMRect();
	validate_SZone();
	validate_DCSkaterModel();
	validate_DCMaterial();
	validate_DCObject();
	validate_DCStrip();
	validate_DCObjectList();
	validate_DCKeyFrame();
	validate_PREManager();
	validate_CSonicRipple();
	validate_Vector();
	validate_SRhinoData();
	validate_SLight();
	validate_CManipObChunk();
	validate_DB_RECT();
	validate_DR_ENV();
	validate_DRAWENV();
	validate_DISPENV();
	validate_SSfxRelated();
	validate_SSfxAsset();
	validate_CRibbon();
	validate_CGlassBit();
	validate_CSmokeGenerator();
	validate_SDXSoundHolder();
	validate_SDxSomething();
	validate_DSBUFFERDESC();
	validate_TwiddleStuff();
	validate_CSmokePuff();
	validate_SRibbonTexture();
	validate_SSimpleRibbonParams();
	validate_CSpark();
	validate_SIndicator();
	validate_POLY_F3();
	validate_CVenomHitSpark();
	validate_SPushOffset();
	validate_SLink();

	puts("[*] Validation done!");

    return FAIL_VALIDATION;
}

// @Bogus
void runtime_assertions()
{
	int result = run_assertions();

	while(result)
		;
}

// @Bogus
void *my_malloc(size_t s)
{
	void* res = malloc(s);

	return res;
}

// @Bogus
void my_free(void* block)
{
	free(block);
}

// @Bogus
int my_atexit(
   void (MY_CDECL *func )( void )
)
{
	return atexit(func);
}

// @Bogus
void *my_realloc(void *m, size_t s)
{
	void* res = realloc(m, s);

	return res;
}

#ifdef _WIN32
// @Bogus
_onexit_t my_onexit(
   _onexit_t function
)
{
	return _onexit(function);
}
#endif

// @Bogus
void *my_new(size_t s)
{
	void* res = ::operator new(s);
	return res;
}

// @Bogus
void *my_calloc(size_t a, size_t b)
{
	return calloc(a, b);
}

// @Bogus
void patch_alloc(void)
{
	PATCH_PUSH_RET(0x0052A227, my_malloc);
	PATCH_PUSH_RET(0x0052A3C0, my_free);
	PATCH_PUSH_RET(0x00529C39, my_atexit);

	PATCH_PUSH_RET(0x0052F250, my_realloc);
#ifdef _WIN32
	PATCH_PUSH_RET(0x00529BBB, my_onexit);
#endif
	PATCH_PUSH_RET(0x00529BA2, my_new);

	PATCH_PUSH_RET(0x0052C044, my_calloc);
}

// @Bogus
static int my_video_player(const char*, i32)
{
	return 1;
}

// @Bogus
void game_patches(void)
{
	//PATCH_CALL(0x004707BE, my_video_player);

	patch_alloc();

	patch_mem();
	patch_utils();
	patch_ps2funcs();

	patch_pkr();
	patch_pcdcMem();
	patch_pack();
	patch_vram();

	patch_CItem();
	patch_CBody();

	// Keep the original spool/resource code active while the replacement
	// functions are being validated against the retail executable.
	patch_trig();
	patch_pctex();
	patch_dcfileio();
	patch_PCMovie();

	patch_flash();
	patch_pshell();
	patch_FontTools();
	patch_mess();
	patch_m3dcolij();
	patch_CSuper();
	patch_ps2m3d();
	patch_m3dutils();
	patch_CBit();
	patch_CFT4Bit();
}

// Co-op's DirectInput wrapper is installed below without replacing Pad_Update.
#ifdef _WIN32
static void __cdecl CoOpDebug_GetMappedStates(u32*, u32*);
static void __cdecl CoOp_RenderPlayers(void*);
static void __cdecl CoOp_KillAll();
static void __cdecl CoOp_RenderSetup(void*, void*, u32*);
static void __cdecl CoOp_RenderCleanup();
static void __fastcall CoOp_PlayerAI(CPlayer*, void*);
static i32 __cdecl CoOp_ActivationDistance(CVector*, CVector*);
static void __cdecl CoOp_EnvironmentBox(const CVector*,const CVector*,bool,bool);
static bool CoOp_InstallHudHooks();
static bool CoOp_InstallDisplayMsaaHooks();
static bool CoOp_InstallConfigHooks();
static bool CoOp_InstallMainMenuHooks();
static bool CoOp_InstallCameraTuningHook();
static bool CoOp_InstallCameraPolicyHooks();
static bool CoOp_InstallProjectileCollisionHooks();
static bool CoOp_InstallLifeHooks();
static bool CoOp_InstallEffectsHooks();
static bool CoOp_InstallArmorRenderHook();
static bool CoOp_InstallRhinoShockHook();
static bool CoOp_InstallInteractionHooks();
static bool CoOp_InstallTraversalExitHook();
static bool CoOp_InstallRaceProgressHooks();
static bool CoOp_InstallCameraUpdateHook();
static bool CoOp_InstallGamepadHook();
static void __cdecl CoOp_UpdateCameras(void*,i32);
static void __cdecl CoOp_UpdatePlayers(void*,i32);
static void __cdecl CoOp_ConstructorBagHead(int,int);
static SCommandPoint* __cdecl CoOp_SectionTrigger(u32,bool);
static void __cdecl CoOp_SectionExecute(u16*,i32,i32);
static void __cdecl CoOp_SectionPending();
static void __cdecl CoOp_SectionVisibility(const char*,i32,i32,bool);
#endif

// @Bogus
void runtime_patches(void)
{
#ifdef _WIN32
    // All co-op hooks are verified against the supported retail call targets.
    // Refuse the whole set on a different executable rather than patching blindly.
    if (sizeof(CPlayer)!=0xEFC || sizeof(CCamera)!=0x2F4 ||
        sizeof(SControl)!=0x18C) return;
    if (*reinterpret_cast<u32*>(0x0053C46C) != 0x004C65C0) return;
    const u32 sites[] = {0x00505738, 0x00455750, 0x00443B05, 0x0048D931,
        0x004937AD, 0x00494202, 0x00496A6F, 0x0049705C, 0x0049B183,
        0x0049E575, 0x004A1717, 0x004E047E, 0x004E2901,
        0x00455633, 0x00455902, 0x0046102E, 0x00461187, 0x004E0E53, 0x004BA8A2,
          0x00452C19,0x004DE806,0x0045551D,0x004E10E9,0x004554A8};
    for (u32 site = 0; site < sizeof(sites) / sizeof(sites[0]); ++site)
    {
        const u8* code = reinterpret_cast<const u8*>(sites[site]);
        u32 expected = site == 0 ? 0x0050A190 : site == 1 ? 0x004739A0 :
            site == 13 ? 0x00472DC0 : site == 14 ? 0x004737F0 :
            site == 19 ? 0x004DE8D0 : site == 20 ? 0x004E0210 :
              site == 21 ? 0x004DE7F0 : site == 22 ? 0x004E6A40 :
              site == 23 ? 0x00460FC0 :
              site == 18 ? 0x004B9210 : site == 17 ? 0x004E6BC0 : site >= 15 ? 0x004E6220 : 0x004437E0;
        if (code[0] != 0xE8 || sites[site] + 5 + *reinterpret_cast<const i32*>(code + 1) != expected)
        {
            OutputDebugStringA("Co-op CPlayer build: unsupported executable; hooks skipped.\n");
            return;
        }
    }
    DWORD aiProtect;
    if (!VirtualProtect(reinterpret_cast<void*>(0x0053C46C),4,PAGE_READWRITE,&aiProtect))
        return;
    *reinterpret_cast<u32*>(0x0053C46C)=reinterpret_cast<u32>(&CoOp_PlayerAI);
    DWORD aiUnused;
    VirtualProtect(reinterpret_cast<void*>(0x0053C46C),4,aiProtect,&aiUnused);
	LPVOID text_start = (void*)0x00401000;

	SIZE_T text_size = 0x0053B000 - (int)text_start;

	DWORD text_protect;
	VirtualProtect(text_start, text_size, PAGE_EXECUTE_READWRITE, &text_protect);

	// Keep reconstructed gameplay hooks disabled while the co-op layer uses
	// retail routines. This call-site hook preserves retail Pad_Update.
	PATCH_CALL(0x00505738, CoOpDebug_GetMappedStates);
	PATCH_CALL(0x004554A8, CoOp_UpdatePlayers);
	PATCH_CALL(0x00455750, CoOp_RenderPlayers);
    PATCH_CALL(0x00455633, CoOp_RenderSetup);
    PATCH_CALL(0x00455902, CoOp_RenderCleanup);
    PATCH_CALL(0x0046102E, CoOp_ActivationDistance);
    PATCH_CALL(0x00461187, CoOp_ActivationDistance);
    PATCH_CALL(0x004E0E53, CoOp_EnvironmentBox);
    PATCH_CALL(0x004BA8A2, CoOp_ConstructorBagHead);
    PATCH_CALL(0x00452C19, CoOp_SectionTrigger);
    PATCH_CALL(0x004DE806, CoOp_SectionExecute);
    PATCH_CALL(0x0045551D, CoOp_SectionPending);
    PATCH_CALL(0x004E10E9, CoOp_SectionVisibility);
	PATCH_CALL(0x00443B05, CoOp_KillAll);
	PATCH_CALL(0x0048D931, CoOp_KillAll);
	PATCH_CALL(0x004937AD, CoOp_KillAll);
	PATCH_CALL(0x00494202, CoOp_KillAll);
	PATCH_CALL(0x00496A6F, CoOp_KillAll);
	PATCH_CALL(0x0049705C, CoOp_KillAll);
	PATCH_CALL(0x0049B183, CoOp_KillAll);
	PATCH_CALL(0x0049E575, CoOp_KillAll);
	PATCH_CALL(0x004A1717, CoOp_KillAll);
	PATCH_CALL(0x004E047E, CoOp_KillAll);
	PATCH_CALL(0x004E2901, CoOp_KillAll);
	if(!CoOp_InstallResolutionHook())
		OutputDebugStringA("Co-op display resolutions: unsupported retail callback; hook skipped.\n");

	DWORD t;
	VirtualProtect(text_start, text_size, text_protect, &t);
	FlushInstructionCache(GetCurrentProcess(), text_start, text_size);
    if(!CoOp_InstallCameraUpdateHook()) OutputDebugStringA("Co-op camera update: unsupported executable; hook skipped.\n");
    if(!CoOp_InstallGamepadHook()) OutputDebugStringA("Co-op controller: unsupported executable; P2 pad hook skipped.\n");
    if(!CoOp_InstallTraversalExitHook())OutputDebugStringA("Scripted traversal: unsupported snap call; hook skipped.\n");
    if(!CoOp_InstallInteractionHooks()) OutputDebugStringA("Co-op interactions: unsupported executable; hooks skipped.\n");
    if(!CoOp_InstallRaceProgressHooks()) OutputDebugStringA("Co-op race progress: unsupported module table; hooks skipped.\n");
    if(!CoOp_InstallEffectsHooks()) OutputDebugStringA("Co-op effects: unsupported render path; hooks skipped.\n");
    if(!CoOp_InstallArmorRenderHook()) OutputDebugStringA("Co-op armor: unsupported model-selection branch; hook skipped.\n");
    if(!CoOp_InstallRhinoShockHook()) OutputDebugStringA("Co-op Rhino: unsupported shock animation call; hook skipped.\n");
    if(!CoOp_InstallLifeHooks()) OutputDebugStringA("Co-op life: unsupported death hooks.\n");
    if(!CoOp_InstallCameraPolicyHooks()) OutputDebugStringA("Co-op camera policy: unsupported script setters.\n");
    if(!CoOp_InstallProjectileCollisionHooks()) OutputDebugStringA("Co-op projectile collision: unsupported sphere queries.\n");
    if(!CoOp_InstallCameraTuningHook()) OutputDebugStringA("Co-op camera tuning: unsupported script call.\n");
    if(!CoOp_InstallMainMenuHooks()) OutputDebugStringA("Co-op game menu: unsupported shell; hooks skipped.\n");
    if(!CoOp_InstallConfigHooks()) OutputDebugStringA("Co-op configuration: unsupported menu; hooks skipped.\n");
    if(!CoOp_InstallDisplayMsaaHooks()) OutputDebugStringA("Display MSAA: unsupported menu; hook skipped.\n");
    if(!CoOp_InstallHudHooks()) OutputDebugStringA("Co-op HUD: unsupported draw ABI; HUD hooks skipped.\n");
#endif
}

#ifdef _WIN32
#include "coop/world/coop_world_trace.h"
static CoOpPlayerRegistry gCoOpPlayers;
static CoOpWorldTrace<CItem> gCoOpWorldTrace;
// Record nearby section/object state at the selected update or render phase.
static void CoOp_TraceWorld(unsigned phase)
{
    static bool wasActive=false;
    if(!gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO)) {
        if(wasActive) gCoOpWorldTrace.Clear();
        wasActive=false;return;
    }
    wasActive=true;
    // Resource descriptors and the fixed collision grid are shared world data.
    // Hash their raw bytes without following potentially stale resource pointers.
    const u8* resources=reinterpret_cast<const u8*>(0x006B2440);
    const u8* grid=reinterpret_cast<const u8*>(0x005FC640);
    unsigned resourceHash=2166136261u,gridHash=2166136261u;
    for(unsigned i=0;i<40*0x44;++i) resourceHash=(resourceHash^resources[i])*16777619u;
    for(unsigned j=0;j<0x660;++j) gridHash=(gridHash^grid[j])*16777619u;
    gCoOpWorldTrace.Capture(*reinterpret_cast<CItem**>(0x006B2EFC),phase,resourceHash,gridHash);
}

// Preserve the retail constructor's bag-head presentation under co-op rendering.
static void __cdecl CoOp_ConstructorBagHead(int scale,int mode)
{
    // Shared mesh mutations belong to the original level/costume setup.
    if(gCoOpPlayers.InSecondaryContext()) return;
    typedef void (__cdecl *Fn)(int,int);
    reinterpret_cast<Fn>(0x004B9210)(scale,mode);
}
static CoOpEnvironment<CItem,CVector> gCoOpEnvironment;
static CItem* gCoOpEnvironmentHead;
static u8 gCoOpExitKeyWasDown;

static u8 gCoOpViewKeyWasDown;
static CoOpSplitLayout gCoOpLayout = COOP_SPLIT_LEFT_RIGHT;
static CoOpRetailViewport gCoOpView;
static void* gCoOpOriginalViewport;
static bool gCoOpViewActive;
static CoOpRetailViewport gCoOpFullView, gCoOpSecondView;
static u8 gCoOpPrimaryCamera[0x94], gCoOpSecondCamera[0x94];
// Retain each native setup's screen projection after its render state is restored.
static float gCoOpMarkerProjection[2][16];
static bool gCoOpMarkerProjectionValid[2];
static CoOpPlayerHandle gCoOpCameraOwner;






static u32* gCoOpOrderingTable;
static bool gCoOpSecondRendered;
typedef int (__cdecl *CoOpClearRayFn)(CVector*,CVector*,void*,int);

static CoOpInput gCoOpInput;
#include "coop/input/coop_gamepad_runtime.h"
#include "coop/input/coop_controller_route.h"
#include "coop/menu/coop_config_runtime.h"
#include "coop/menu/coop_menu_runtime.h"
#include "coop/camera/coop_camera_tuning.h"
#include "coop/camera/coop_camera_policy_runtime.h"
#include "coop/player/coop_life_runtime.h"
#include "coop/presentation/coop_effects_runtime.h"
#include "coop/player/coop_armor_runtime.h"
#include "coop/interaction/coop_rhino_runtime.h"


#include "coop/world/coop_sections.h"
#include "coop/script/coop_script_motion.h"
#include "coop/interaction/coop_interactions.h"
static CoOpDeferredOwners gCoOpDeferredOwners;
static unsigned gCoOpStaleScripts;
static CoOpPlayerHandle gCoOpCinematicOwner;
static CoOpScriptMotionState gCoOpScriptMotion;
static unsigned gCoOpSharedLevelChanges,gCoOpKillInSecondaryContext;
static CoOpSections<CItem> gCoOpSections;
static CoOpSectionOwners gCoOpNodeOwners;
static unsigned gCoOpTriggerOwner, gCoOpCommandOwners;
static unsigned gCoOpSectionCalls[4];
static unsigned gCoOpSectionTouches[2];
static unsigned gCoOpRaceEntryScripts[2];
static unsigned gCoOpCollisionScripts[2];
static void CoOp_SectionContact(const char*,int,int,bool,void*);
static void __cdecl CoOp_SectionVisibility(const char*,i32,i32,bool);
static bool gCoOpSectionsActive;
static bool gCoOpRaceSectionsActive;
static bool gCoOpSewerSectionsActive;
static CoOpSectionProfile gCoOpSectionProfile;
static unsigned __int64 gCoOpSectionGeneration;
static const void* gCoOpSectionResource;
// Read the retail list head for currently loaded level items.
static CItem* CoOp_SectionLive() {return *reinterpret_cast<CItem**>(0x006B2EFC);}
// Discover the loaded bank's section profile before mediating visibility.
static void CoOp_SyncSections()
{
    const u8* resource=0;
    for(int i=0;i<40;++i) {
        const u8* r=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(r[10] && CoOp_SectionProfileForResource((const char*)r)!=COOP_SECTION_NONE) {
            resource=r;break;
        }
    }
    const void* psx=resource ? *reinterpret_cast<const void* const*>(resource+0x18):0;
    CoOpSectionProfile previousProfile=gCoOpSectionProfile;
    gCoOpSectionProfile=CoOp_SectionProfileForResource((const char*)resource);
    gCoOpRaceSectionsActive=gCoOpSectionProfile==COOP_SECTION_RACE;
    gCoOpSewerSectionsActive=gCoOpSectionProfile==COOP_SECTION_SEWER;
    if(psx!=gCoOpSectionResource || gCoOpSectionProfile!=previousProfile) {
        gCoOpSections.Clear();gCoOpSectionsActive=false;gCoOpSectionResource=psx;
        gCoOpNodeOwners.Clear();
    }
    CPlayer* p2=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO);
    // A hidden view is still a live participant. Only actual departure may
    // discard its section requirements; cycling views must retain both masks.
    bool active=psx && p2;
    if(!active) {
        if(gCoOpSectionsActive) {
            gCoOpSections.ApplyLive(CoOp_SectionLive(),1);
            gCoOpSections.Clear();
        }
        gCoOpSectionsActive=false;
        return;
    }
    unsigned __int64 generation=gCoOpPlayers.Router().Handle(COOP_PLAYER_TWO).generation;
    if(gCoOpSectionsActive && generation!=gCoOpSectionGeneration)
        gCoOpSections.Rejoin(CoOp_SectionLive());
    gCoOpSectionGeneration=generation;gCoOpSectionsActive=true;
}
#include "coop/world/coop_fall_triggers.h"
#include "coop/script/coop_traversal_exit.h"
// Read one trigger-bank command range for fall-chain classification.
static bool CoOp_ReadFallNode(unsigned node,CoOpFallNode& out,void*) {
    int count=*reinterpret_cast<int*>(0x006B4670);
    if(count<=0 || node>=(unsigned)count)return false;
    u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
    if(!nodes || !nodes[node])return false;
    out.type=nodes[node][0];out.actorType=out.type==1?nodes[node][1]:0;out.commands=0;out.links=0;
    if(out.type!=1 && out.type!=6)return false;
    out.links=nodes[node]+(out.type==1?3:1);
    if(out.type==6) {
        SCommandPoint* p=*reinterpret_cast<SCommandPoint**>(0x006B4708);
        for(int i=0;p && i<count;++i,p=p->pNext)
            if(p->NodeIndex==node){out.commands=p->pCommands;break;}
    }
    return true;
}
static bool CoOp_VenomRaceBank();
// Correct the authored Venom traversal exit position at its native snap point.
static void __cdecl CoOp_TraversalSnap(CVector* output,int node) {
    if(node==54 && CoOp_VenomRaceBank()) {
        CoOpFallNode route;unsigned count=*reinterpret_cast<int*>(0x006B4670);
        u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
        CPlayer* actor=*reinterpret_cast<CPlayer**>(0x006A9038);
        if(actor && count>72 && CoOp_ReadFallNode(71,route,0) && route.commands &&
           nodes[72]>route.commands) {
            const u16* cursor=*reinterpret_cast<const u16* const*>(reinterpret_cast<u8*>(actor)+0x1B8);
            node=CoOp_TraversalSnapNode(node,route.commands,(unsigned)(nodes[72]-route.commands),cursor);
        }
    }
    reinterpret_cast<void (__cdecl*)(CVector*,int)>(0x004E3940)(output,node);
}
// Install install traversal exit hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallTraversalExitHook() {
    const u32 site=0x004BC6C4;
    const u8* call=reinterpret_cast<const u8*>(site);
    if(call[0]!=0xE8 || site+5+*reinterpret_cast<const i32*>(call+1)!=0x004E3940)return false;
    DWORD old,unused;if(!VirtualProtect(reinterpret_cast<void*>(site),5,PAGE_EXECUTE_READWRITE,&old))return false;
    PATCH_CALL(site,CoOp_TraversalSnap);
    VirtualProtect(reinterpret_cast<void*>(site),5,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5);return true;
}
// Convert a co-op fall trigger into per-player damage before retail game-over logic.
static bool CoOp_InterceptFallTrigger(u32 checksum,unsigned owner) {
    if(!CoOp_LifeActive() || (owner!=1 && owner!=2))return false;
    SCommandPoint* point=reinterpret_cast<SCommandPoint**>(0x006B4214)[checksum&255];
    for(;point;point=point->pNextSimilar)if(point->Checksum==checksum &&
        // Identify a linked fall/death trigger without consuming unrelated scripts.
        CoOp_FallChain(point->NodeIndex,CoOp_ReadFallNode,0)) {
        CPlayer* p=gCoOpPlayers.Get((CoOpPlayerSlot)(owner-1));
        CoOp_LifeFall(p);
        return true; // do not enqueue camera scripts or consume the shared trigger
    }
    return false;
}
// Project reachable section commands for the player who contacted the trigger.
static unsigned CoOp_ProjectSectionGraph(unsigned root,unsigned owner) {
    unsigned queue[128],count=1,touched=0;queue[0]=root;
    u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
    unsigned nodeCount=*reinterpret_cast<int*>(0x006B4670);
    for(unsigned at=0;at<count;++at) {
        unsigned node=queue[at];CoOpFallNode n;
        if(!CoOp_ReadFallNode(node,n,0) || n.type!=6 || !n.commands || node+1>=nodeCount)continue;
        touched+=CoOp_VisitSectionCommands(n.commands,nodes[node+1],CoOp_SectionContact,&owner,
            gCoOpSectionProfile);
        const u16* c=n.commands;if(c[0]==134)c+=2;
        if(c[0]!=3 || c[1]!=0xFFFF || !n.links || n.links[0]>127)continue;
        for(unsigned k=1;k<=n.links[0];++k) {
            unsigned child=n.links[k],j;for(j=0;j<count && queue[j]!=child;++j){}
            if(j==count && count<128)queue[count++]=child;
        }
    }
    return touched;
}
static unsigned gCoOpVenomRaceFailureChecks,gCoOpVenomRaceFailureDeferred;
// Accept only living player bodies as race-progress candidates.
static bool CoOp_RaceAlive(CoOpPlayerHandle,const CPlayer* p,void*) {return p && p->mHealth>0;}
// Identify the current authored Venom-race trigger bank.
static bool CoOp_VenomRaceBank() {
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[10] && !_strnicmp((const char*)resource,"L5A1_G",7))return true;
    }
    return false;
}
// Keep a trailing runner from ending the shared Venom race.
static bool CoOp_TrailingRaceOwner(CoOpPlayerHandle owner) {
    if(!gCoOpSession.enabled || !gCoOpPlayers.Get(COOP_PLAYER_TWO) ||
       !gCoOpPlayers.Router().Resolve(owner) || !CoOp_VenomRaceBank())return false;
    for(CItem* item=*reinterpret_cast<CItem**>(0x0056E990);item;item=item->mNextItem) {
        if(item->mType!=313)continue;
        CoOpPlayerRouter::Query query;query.origin=item->mPos;
        query.condition=CoOp_RaceAlive;
        CoOpPlayerHandle leader=gCoOpPlayers.Router().Select(query);
        if(!gCoOpPlayers.Router().Resolve(leader))return false;
        ++gCoOpVenomRaceFailureChecks;
        if(leader.slot!=owner.slot) {++gCoOpVenomRaceFailureDeferred;return true;}
        return false;
    }
    return false;
}
// Suppress the race-loss contact until the leading participant reaches it.
static bool CoOp_DeferRaceContact(unsigned node,unsigned owner) {
    return node==330 && (owner==1 || owner==2) &&
        CoOp_TrailingRaceOwner(gCoOpPlayers.Router().Handle((CoOpPlayerSlot)(owner-1)));
}
static unsigned CoOp_PendingCount();
// Record collision-trigger ownership and section activation before calling retail.
static SCommandPoint* __cdecl CoOp_SectionTrigger(u32 checksum,bool assertMissing)
{
    // This hook is only the collision call site, not crate destruction pulses.
    unsigned owner=gCoOpPlayers.InSecondaryContext()?2:gCoOpTriggerOwner;
    if(owner && gCoOpScriptMotion.Partner(owner-1))return 0;
    if(CoOp_InterceptFallTrigger(checksum,owner))return 0;
    if(gCoOpSession.enabled && owner) {
        SCommandPoint* point=reinterpret_cast<SCommandPoint**>(0x006B4214)[checksum&255];
        for(;point;point=point->pNextSimilar)
            if(point->Checksum==checksum) {
                // L5A1 node 330 starts the entire four-child loss sequence.
                // A trailing runner must not latch its camera or actor scripts.
                if(CoOp_DeferRaceContact(point->NodeIndex,owner))return 0;
                // Owner tracking applies to every level, including chase
                // scripts; terrain projection below is L1A3-specific.
                gCoOpNodeOwners.Record(point->NodeIndex,owner);
                if(!gCoOpSectionsActive)continue;
                // Retail's shared latch can suppress a second participant even
                // on a later frame. Terrain requirements are local; project
                // only the leading visibility commands on every contact.
                u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
                int nodeCount=*reinterpret_cast<int*>(0x006B4670);
                unsigned node=point->NodeIndex;
                if(nodes && node+1<(unsigned)nodeCount && nodes[node] &&
                    point->pCommands>=nodes[node] && nodes[node+1]>point->pCommands) {
                    unsigned touched=(point->pCommands[0]==134 || point->pCommands[0]==3)?
                        CoOp_ProjectSectionGraph(node,owner):CoOp_VisitSectionCommands(point->pCommands,
                            nodes[node+1],CoOp_SectionContact,&owner,gCoOpSectionProfile);
                    if(owner==1 || owner==2)gCoOpSectionTouches[owner-1]+=touched;
                }
            }
    }
    typedef SCommandPoint* (__cdecl *Fn)(u32,bool);
    unsigned before=CoOp_PendingCount();
    SCommandPoint* result=reinterpret_cast<Fn>(0x004DE8D0)(checksum,assertMissing);
    if(gCoOpSession.enabled && (owner==1 || owner==2)) {
        CoOpPlayerHandle participant=gCoOpPlayers.Router().Handle((CoOpPlayerSlot)(owner-1));
        if(gCoOpPlayers.Router().Resolve(participant)) {
            PendingListEntry* queue=reinterpret_cast<PendingListEntry*>(0x006B4688);
            unsigned after=CoOp_PendingCount();
            for(unsigned i=before;i<after;++i) {
                gCoOpDeferredOwners.Record(queue[i].pCommands,queue[i].NodeIndex,participant);
                gCoOpNodeOwners.Record(queue[i].NodeIndex,owner);
                ++gCoOpCollisionScripts[owner-1];
            }
        }
    }
    return result;
}
// Count native deferred script entries without reading beyond its fixed queue.
static unsigned CoOp_PendingCount()
{
    PendingListEntry* queue=reinterpret_cast<PendingListEntry*>(0x006B4688);
    unsigned n=0;while(n<16 && queue[n].pCommands) ++n;
    return n;
}
struct CoOpScriptCall {u16* commands;i32 node,flags; i32 letterbox;};
#include "coop/script/coop_cinematic_tick.h"
#include "coop/script/coop_crane_fallback.h"
static unsigned gCoOpAuthoredCameraScripts[3];
static int gCoOpLastCameraScriptNode[3]={-1,-1,-1};
static unsigned gCoOpLastCameraScriptOp[3];
static void* gCoOpP2CameraScriptBank;
// Check whether a cinematic owner still has an active, living CPlayer.
static bool CoOp_CinematicOwnerLive(CoOpPlayerHandle owner) {
    if(owner.slot>=COOP_PLAYER_COUNT)return false;
    const CoOpPlayerRouter::Participant* actor=gCoOpPlayers.Router().Resolve(owner);
    return CoOp_CinematicOwnerActive(actor && actor->player,
        actor && actor->player ? actor->player->mHealth : 0,
        CoOp_LifeWaiting(owner.slot));
}
// Clear shared letterbox ownership when its player dies or waits for respawn.
static void CoOp_ReleaseDeadCinematic() {
    if(gCoOpCinematicOwner.slot<COOP_PLAYER_COUNT &&
       !CoOp_CinematicOwnerLive(gCoOpCinematicOwner)) {
        // Release presentation before dropping the owner handle. Otherwise
        // the camera boundary cannot detect the dead owner and LifeFrame
        // remains blocked by an orphaned letterbox.
        *reinterpret_cast<i32*>(0x00660F80)=0;
        *reinterpret_cast<i32*>(0x0060F76C)=0;
        gCoOpCinematicOwner=CoOpPlayerHandle();
    }
}
// Detect an authored scene that still requires P2's independent body tick.
static bool CoOp_SecondaryInteractiveScene() {
    // L3A4 node 88 is the crane interaction's authored P2 fixed camera.
    // Restrict live controls to this scene; ordinary cinematics stay neutral.
    if(gCoOpLastCameraScriptNode[COOP_PLAYER_TWO]!=88 ||
       !CoOp_CinematicOwnerLive(gCoOpPlayers.Router().Handle(COOP_PLAYER_TWO)))return false;
    bool craneBank=false;
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[10] && !_strnicmp((const char*)resource,"L3A4_G",7)) {
            craneBank=true;break;
        }
    }
    if(!craneBank)return false;
    CCamera* first=*reinterpret_cast<CCamera**>(0x0056F3B8);
    CCamera* second=gCoOpPlayers.SecondaryCamera();
    return first && second && CoOp_InteractiveSceneOwner(
        *reinterpret_cast<i32*>(0x00660F80)!=0,
        gCoOpP2CameraScriptBank &&
            gCoOpP2CameraScriptBank==*reinterpret_cast<void**>(0x006B466C),
        first->mCameraMode,second->mCameraMode);
}
static unsigned gCoOpCraneInputTicks,gCoOpCraneInputPresses;
static const void* gCoOpCraneAutoBank;
static unsigned gCoOpCraneAutoCount;
static unsigned gCoOpCraneAutoOwner=COOP_PLAYER_COUNT;
static bool gCoOpCraneArrivalPending,gCoOpCraneArrivalRequested;
static unsigned gCoOpCraneArrivalCount;
// Finish the crane handoff after the authored exit script has placed its actor.
static void CoOp_CraneArrivalFrame(CPlayer* player) {
    if(!gCoOpCraneArrivalPending)return;
    if(!gCoOpSession.enabled) {
        gCoOpCraneArrivalPending=gCoOpCraneArrivalRequested=false;return;
    }
    if(!player || player!=gCoOpPlayers.Get(COOP_PLAYER_ONE) || player->mHealth<=0)return;
    bool arrivalBank=false;
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[10] && !_strnicmp((const char*)resource,"L3A5_G",7)) {
            arrivalBank=true;break;
        }
    }
    if(!arrivalBank)return;
    if(gCoOpCraneArrivalRequested) {
        // Retail's shared frame calls 004BC020 after script/camera updates,
        // restores the authored position/orientation, and consumes this flag.
        // Do not release the hidden partner until that handoff has finished.
        if(!player->field_1AC && !*reinterpret_cast<u8*>(0x0060CFC4)) {
            gCoOpCraneArrivalPending=gCoOpCraneArrivalRequested=false;
            ++gCoOpCraneArrivalCount;
        }
        return;
    }
    u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
    int count=*reinterpret_cast<int*>(0x006B4670);
    if(!nodes || count<=242 || !nodes[22] || !nodes[23] || !nodes[26] ||
       !nodes[242] || *nodes[22]!=6 || *nodes[26]!=3 ||
       nodes[26][1]!=1 || nodes[26][2]!=242 || *nodes[242]!=3)return;
    SCommandPoint* route=*reinterpret_cast<SCommandPoint**>(0x006B4708);
    while(route && route->NodeIndex!=22)route=route->pNext;
    if(!route || !route->pCommands || route->pCommands<nodes[22] ||
       route->pCommands>=nodes[23] ||
       !CoOp_CraneArrivalRouteMatches(route->pCommands,(unsigned)(nodes[23]-route->pCommands)))return;
    const u16* cursor=*reinterpret_cast<const u16* const*>(reinterpret_cast<const u8*>(player)+0x1B8);
    if(!player->field_1AC || player->field_1A8!=26 || !cursor ||
       cursor<nodes[22] || cursor>=nodes[23])return;
    // Let one native tick initialize CutSceneSkipSnapPos first. Request the
    // ordinary skip on the following tick, so SynthesizeAnalogueInput frees
    // its pending controls and uses marker 26. Keep the flag through the rest
    // of the shared frame: camera/NPC scripts and 004BC020 also consume it.
    *reinterpret_cast<u8*>(0x0060CFC4)=1;
    gCoOpCraneArrivalRequested=true;
}
// Resolve the supported crane command list to its completed exit route.
static u16* CoOp_CraneExitForRoute(u16* commands,i32 node) {
    if(!gCoOpSession.enabled || node!=158 || !commands)return 0;
    bool craneBank=false;
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[10] && !_strnicmp((const char*)resource,"L3A4_G",7)) {
            craneBank=true;break;
        }
    }
    u16** nodes=*reinterpret_cast<u16***>(0x006B466C);
    int count=*reinterpret_cast<int*>(0x006B4670);
    if(!craneBank || !nodes || count<=159 || !nodes[156] || !nodes[157] ||
       !nodes[158] || !nodes[159] || *nodes[156]!=6 || *nodes[158]!=6)return 0;
    SCommandPoint* route=0;SCommandPoint* exit=0;
    for(SCommandPoint* p=*reinterpret_cast<SCommandPoint**>(0x006B4708);p;p=p->pNext) {
        if(p->NodeIndex==158)route=p;
        if(p->NodeIndex==156)exit=p;
    }
    if(!route || route->pCommands!=commands || !exit || !exit->pCommands)return 0;
    if(commands<nodes[158] || commands>=nodes[159] ||
       exit->pCommands<nodes[156] || exit->pCommands>=nodes[157])return 0;
    if(!CoOp_CraneAutoReady(true,craneBank,node,commands,
        (unsigned)(nodes[159]-commands)) ||
       !CoOp_CraneExitMatches(exit->pCommands,(unsigned)(nodes[157]-exit->pCommands)))return 0;
    return exit->pCommands;
}
// Detect scripts that replace the whole bank and must run once globally.
static bool CoOp_IsSharedLevelChange(const u16* commands) {
    // Retail opcode 305 clears the world at the start of these level-change
    // command lists; later commands load the new bank. L3A1_T node 278
    // loads L3A1a_T this way. Replacing the old player bank after this call
    // would restore stale animation/model pointers into the new level.
    return commands && commands[0]==305;
}
// Ignore the shared loss message while a living runner is still ahead.
static bool CoOp_DeferTrailingRaceFailure(const u16* commands,i32 node,CoOpPlayerHandle owner) {
    return node==316 && commands && commands[0]==218 &&
        !strcmp(reinterpret_cast<const char*>(commands+3),"You Lost Venom") &&
        CoOp_TrailingRaceOwner(owner);
}
// Call the native script dispatcher and capture its shared letterbox result.
static void CoOp_RunScript(void* argument) {
    CoOpScriptCall* call=static_cast<CoOpScriptCall*>(argument);
    typedef void (__cdecl *Fn)(u16*,i32,i32);
    *reinterpret_cast<i32*>(0x00660F80)=call->letterbox;
    reinterpret_cast<Fn>(0x004E0210)(call->commands,call->node,call->flags);
    call->letterbox=*reinterpret_cast<i32*>(0x00660F80);
}
// Dispatch an authored command in its collector's context and propagate ownership.
static void __cdecl CoOp_SectionExecute(u16* commands,i32 node,i32 flags)
{
    unsigned previous=gCoOpCommandOwners;
    gCoOpCommandOwners=gCoOpNodeOwners.Get((unsigned)node);
    unsigned before=CoOp_PendingCount();
    CoOpPlayerHandle owner=gCoOpDeferredOwners.Get(commands,node);
    if(owner.slot>=COOP_PLAYER_COUNT && CoOp_CinematicOwnerLive(gCoOpCinematicOwner))
        owner=gCoOpCinematicOwner; // later NPC/trigger pulses continue the same cinematic
    u16* craneExit=CoOp_CraneExitForRoute(commands,node);
    if(craneExit) {
        // The native crane route never consumes co-op input reliably. Complete
        // it at dispatch, before it can drop the actor into the kill volume.
        // Keep the collector when alive; a waiting/dead partner cannot own it.
        if(!CoOp_CinematicOwnerLive(owner)) {
            for(unsigned slot=0;slot<COOP_PLAYER_COUNT;++slot) {
                CoOpPlayerHandle living=gCoOpPlayers.Router().Handle((CoOpPlayerSlot)slot);
                if(CoOp_CinematicOwnerLive(living)) {owner=living;break;}
            }
        }
        if(CoOp_CinematicOwnerLive(owner)) {
            const void* bank=*reinterpret_cast<void**>(0x006B466C);
            if(gCoOpCraneAutoBank==bank) {gCoOpCommandOwners=previous;return;}
            gCoOpCraneAutoBank=bank;++gCoOpCraneAutoCount;
            gCoOpCraneAutoOwner=owner.slot;
            gCoOpCraneArrivalPending=true;gCoOpCraneArrivalRequested=false;
            gCoOpCommandOwners=1u<<owner.slot;
            commands=craneExit;node=156;
            gCoOpScriptMotion.Clear();gCoOpCinematicOwner=CoOpPlayerHandle();
            *reinterpret_cast<i32*>(0x00660F80)=0;
            *reinterpret_cast<i32*>(0x0060F76C)=0;
        }
    }
    // Two contacts can queue the same shared scene before either starts it.
    // A hidden partner must not acquire an unticked native input route.
    if(gCoOpScriptMotion.active && owner.slot<COOP_PLAYER_COUNT &&
       gCoOpScriptMotion.Partner(owner.slot) && commands &&
       (commands[0]==199 || (commands[0]==195 && commands[2]==199))) {
        gCoOpCommandOwners=previous;return;
    }
    if(CoOp_DeferTrailingRaceFailure(commands,node,owner)) {
        gCoOpCommandOwners=previous;return;
    }
    if(commands && (commands[0]==185 || commands[0]==186 || commands[0]==188)) {
        unsigned slot=owner.slot<COOP_PLAYER_COUNT?owner.slot:2;
        ++gCoOpAuthoredCameraScripts[slot];
        gCoOpLastCameraScriptNode[slot]=node;
        gCoOpLastCameraScriptOp[slot]=commands[0];
        if(slot==COOP_PLAYER_TWO)
            gCoOpP2CameraScriptBank=*reinterpret_cast<void**>(0x006B466C);
    }
    const bool sharedLevelChange=CoOp_IsSharedLevelChange(commands);
    CoOpScriptCall call={commands,node,flags,*reinterpret_cast<i32*>(0x00660F80)};
    unsigned triggerPrevious=gCoOpTriggerOwner;
    if(owner.slot<COOP_PLAYER_COUNT) {
        if(gCoOpPlayers.Router().Resolve(owner)) {
            gCoOpTriggerOwner=1u<<owner.slot;
            // Pickup and later cinematic pulses carry a deferred owner, not a
            // collision-node mask. Visibility must use that same participant.
            if(!gCoOpCommandOwners)gCoOpCommandOwners=gCoOpTriggerOwner;
            if(gCoOpRaceSectionsActive && (node==71 || node==304 || node==311) &&
               commands && commands[0]==195 && commands[2]==199) {
                // L5A1's window traversal starts a synthesized player path.
                // Its Inside01 visibility was authored in an earlier scene
                // pulse, often before P2 joined. Retain that actual section
                // for the participant executing the traversal, including P2.
                CoOp_SectionVisibility("Inside",1,1,true);
                ++gCoOpRaceEntryScripts[owner.slot];
            }
            if(owner.slot==COOP_PLAYER_TWO && sharedLevelChange)
                ++gCoOpSharedLevelChanges;
            if(owner.slot==COOP_PLAYER_TWO && !sharedLevelChange)
                gCoOpPlayers.WithSecondaryContext(CoOp_RunScript,&call);
            else CoOp_RunScript(&call);
        } else ++gCoOpStaleScripts; // never redirect a departed collector's event
    } else CoOp_RunScript(&call);
    // Opcode 199 starts native synthesized input on the actual collector.
    // Retain that player beyond this command: later body ticks are part of
    // the same authored motion even if a camera/letterbox pulse changes.
    if(owner.slot<COOP_PLAYER_COUNT && !sharedLevelChange) {
        const CoOpPlayerRouter::Participant* actor=gCoOpPlayers.Router().Resolve(owner);
        if(actor && actor->player->field_1AC)
            gCoOpScriptMotion.Begin(owner,*reinterpret_cast<void**>(0x006B466C),
                actor->player->mPos,0);
    }
    if(owner.slot<COOP_PLAYER_COUNT && gCoOpPlayers.Router().Resolve(owner)) {
        // Letterbox is shared presentation. Do not lose it when a P2 bank exits.
        *reinterpret_cast<i32*>(0x00660F80)=call.letterbox;
        *reinterpret_cast<i32*>(0x0060F76C)=call.letterbox;
        if(call.letterbox && CoOp_CinematicOwnerLive(owner)) {
            if(CoOp_SecondaryInteractiveScene())
                gCoOpCinematicOwner=gCoOpPlayers.Router().Handle(COOP_PLAYER_TWO);
            else gCoOpCinematicOwner=owner;
        }
        else gCoOpCinematicOwner=CoOpPlayerHandle();
    }
    gCoOpTriggerOwner=triggerPrevious;
    // Linked command points are appended by retail; inherit the initiating
    // participant without replaying rewards, enemies or other script actions.
    PendingListEntry* queue=reinterpret_cast<PendingListEntry*>(0x006B4688);
    unsigned after=CoOp_PendingCount();
    for(unsigned i=before;i<after;++i) {
        gCoOpNodeOwners.Record(queue[i].NodeIndex,gCoOpCommandOwners);
        gCoOpDeferredOwners.Record(queue[i].pCommands,queue[i].NodeIndex,owner);
    }
    gCoOpCommandOwners=previous;
}

struct CoOpScriptPlacement {CPlayer* player;CPlayer* actor;CCamera* camera;CVector position;bool placed;};
// Adopt the actor's final motion and camera for the hidden partner.
static void CoOp_PlaceScriptPartner(void* argument) {
    CoOpScriptPlacement* place=static_cast<CoOpScriptPlacement*>(argument);
    CPlayer* p=place->player;
    place->placed=CoOp_AdoptEntryMotion(p,place->actor,place->position);
    CCamera* camera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    if(place->placed)CoOp_AdoptEntryCamera(camera,place->camera);
}
// Shadow the non-owner during authored motion and place both bodies at release.
static void CoOp_ScriptMotionFrame(bool release=false) {
    if(!gCoOpScriptMotion.active)return;
    if(gCoOpScriptMotion.bank!=*reinterpret_cast<void**>(0x006B466C)) {
        gCoOpScriptMotion.Clear();return;
    }
    const CoOpPlayerRouter::Participant* actor=
        gCoOpPlayers.Router().Resolve(gCoOpScriptMotion.owner);
    if(!actor) {gCoOpScriptMotion.Clear();return;}
    CPlayer* body=actor->player;
    if(body->mHealth<=0 || CoOp_LifeWaiting(gCoOpScriptMotion.owner.slot)) {
        CoOp_ReleaseDeadCinematic();
        if(gCoOpCinematicOwner.slot>=COOP_PLAYER_COUNT) {
            *reinterpret_cast<i32*>(0x00660F80)=0;
            *reinterpret_cast<i32*>(0x0060F76C)=0;
        }
        gCoOpScriptMotion.Clear();return;
    }
    const void* cursor=*reinterpret_cast<void* const*>(reinterpret_cast<const u8*>(body)+0x1B8);
    if(!release)gCoOpScriptMotion.Observe(body->field_1AC!=0,cursor,body->mPos);
    // Keep the non-acting body in the route's loaded region while it is
    // hidden. A partner left at the old segment can otherwise lose its floor
    // before the authored motion finishes. Never move a dead/waiting player.
    unsigned partnerSlot=1-gCoOpScriptMotion.owner.slot;
    CPlayer* partner=gCoOpPlayers.Get(static_cast<CoOpPlayerSlot>(partnerSlot));
    if(partner && partner->mHealth>0 && !CoOp_LifeWaiting(partnerSlot) &&
        !partner->field_1AC) {
        if(partnerSlot==COOP_PLAYER_TWO)gCoOpPlayers.ShadowSecondary(body);
        else {
            CoOp_ShadowEntry(partner,body);
        }
    }
    // Pending scripts, cameras and native skip cleanup still run after the
    // player-list boundary. Only the later render boundary releases a partner
    // from their completed transform/state, never an intermediate route end.
    if(!release || body->field_1AC || !gCoOpScriptMotion.waitingForPartner ||
       *reinterpret_cast<i32*>(0x00660F80))return;
    if(!partner || partner->mHealth<=0 || CoOp_LifeWaiting(partnerSlot)) {
        gCoOpScriptMotion.Clear();return;
    }
    if(partner->field_1AC)return; // another authored motion still owns it
    u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
    CVector side;bool safe=CoOp_FindEntrySpawn(body,side);
    memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
    if(!safe)return;
    bool placed=false;
    CCamera* actorCamera=gCoOpScriptMotion.owner.slot==COOP_PLAYER_TWO?
        gCoOpPlayers.SecondaryCamera():*reinterpret_cast<CCamera**>(0x0056F3B8);
    if(partnerSlot==COOP_PLAYER_TWO)placed=gCoOpPlayers.HandoffSecondary(body,side,actorCamera);
    else {
        CoOpScriptPlacement placement;
        placement.player=partner;placement.actor=body;placement.camera=actorCamera;
        placement.position=side;placement.placed=false;
        CoOp_PlaceScriptPartner(&placement);placed=placement.placed;
    }
    if(placed)gCoOpScriptMotion.Clear();
}
// Drain retail's deferred section command queue with recorded player ownership.
static void __cdecl CoOp_SectionPending()
{
    typedef void (__cdecl *Fn)();
    reinterpret_cast<Fn>(0x004DE7F0)();
    gCoOpNodeOwners.Clear();gCoOpDeferredOwners.Clear();
}
// Restrict visibility mediation to recognized, loaded section resources.
static bool CoOp_IsSection(const char* prefix)
{
    return CoOp_IsProtectedSection(prefix,gCoOpSectionProfile);
}
// Compute the retail checksum used to identify a section resource.
static u32 CoOp_SectionChecksum(const char* text)
{
    const u32* table=reinterpret_cast<const u32*>(0x005598BC);
    u32 crc=0xffffffff;
    for(;*text;++text) crc=(crc>>8)^table[(crc^(u8)*text)&255];
    return crc; // retail does not complement the result
}
// Apply one player's visibility contact to the section owner mask.
static void CoOp_SectionContact(const char* prefix,int first,int last,bool visible,void* context)
{
    typedef CItem* (__cdecl *FindFn)(u32);
    unsigned owner=*static_cast<unsigned*>(context);
    for(int n=first;n<=last;++n) {
        char name[24];sprintf(name,"%s%02d",prefix,n);
        CItem* item=reinterpret_cast<FindFn>(0x004C9230)(CoOp_SectionChecksum(name));
        if(item && gCoOpSections.Remember(item))gCoOpSections.Command(item,owner,visible);
    }
}
// Keep a section loaded while either player still requires it.
static void __cdecl CoOp_SectionVisibility(const char* prefix,i32 first,i32 last,bool visible)
{
    typedef void (__cdecl *Fn)(const char*,i32,i32,bool);
    typedef CItem* (__cdecl *FindFn)(u32);
    CItem* matched[100];unsigned count=0;
    bool mediated=gCoOpSectionsActive && prefix && CoOp_IsSection(prefix) &&
        first>=0 && last<100 && first<=last;
    if(mediated) {
        for(int n=first;n<=last;++n) {
            char name[24];sprintf(name,"%s%02d",prefix,n);
            CItem* item=reinterpret_cast<FindFn>(0x004C9230)(CoOp_SectionChecksum(name));
            if(item && gCoOpSections.Remember(item)) matched[count++]=item;
        }
    }
    reinterpret_cast<Fn>(0x004E6A40)(prefix,first,last,visible);
    CoOp_LifeSection(prefix,visible,gCoOpCommandOwners);
    if(mediated) {
        ++gCoOpSectionCalls[gCoOpCommandOwners&3];
        for(unsigned i=0;i<count;++i)
            gCoOpSections.Command(matched[i],gCoOpCommandOwners&3,visible);
    }
}

// Personal HUD replay projects each partner's oriented head in the matching view.
// Keep the partner marker's near-depth DX7 quads in this rebuilt translation unit.
// Rebuild the marker's readable P1/P2 glyphs and modestly larger pointers.
// Account for DX7's reverse HUD queue order and full-display label scale.
#include "coop/presentation/coop_hud_runtime.h"
#include "coop/presentation/coop_display_msaa_runtime.h"
#include "coop/interaction/coop_interaction_runtime.h"
#include "coop/interaction/coop_projectile_collision_runtime.h"

// Find the current environment item before checking each player's area.
static CItem* CoOp_CheckEnvironment()
{
    CItem* head=*reinterpret_cast<CItem**>(0x006B2EFC);
    if(head!=gCoOpEnvironmentHead)
    {
        gCoOpEnvironment.Clear(false); // old level pointers may already be freed
        gCoOpEnvironmentHead=head;
    }
    return head;
}
// Refresh environment activation for both independent player positions.
static void CoOp_UpdateEnvironment()
{
    CoOp_CheckEnvironment();
    CPlayer* body=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO);
    CPlayer* primary=gCoOpPlayers.GetRenderBody(COOP_PLAYER_ONE);
    gCoOpEnvironment.Update(body ? &body->mPos : 0,primary ? &primary->mPos : 0);
}
// Expand a native environment-box query to cover both active players.
static void __cdecl CoOp_EnvironmentBox(const CVector* min,const CVector* max,
    bool visible,bool inside)
{
    typedef void (__cdecl *BoxFn)(const CVector*,const CVector*,bool,bool);
    reinterpret_cast<BoxFn>(0x004E6BC0)(min,max,visible,inside);
    CItem* head=CoOp_CheckEnvironment();
    CPlayer* primary=*reinterpret_cast<CPlayer**>(0x006A9038);
    CVector origin;
    origin.vx=origin.vy=origin.vz=0;
    if(primary) origin=primary->mPos;
    // Fixed authored areas can be recorded during level autoexec before P1.
    if(inside || primary)
        gCoOpEnvironment.Observe(head,*min,*max,visible,inside,origin,gCoOpPlayers.InSecondaryContext()?1:0);
    CoOp_UpdateEnvironment();
}

// Only the retail suspension and wake-up distance calls are redirected.
// Keep the original metric and threshold; do not replay AI or move MechList.
static i32 __cdecl CoOp_ActivationDistance(CVector* primaryPosition, CVector* itemPosition)
{
    typedef i32 (__cdecl *DistanceFn)(CVector*,CVector*);
    CPlayer* secondary=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO);
    return CoOp_NearestActivationDistance(primaryPosition,itemPosition,
        secondary ? &secondary->mPos : 0,
        reinterpret_cast<DistanceFn>(0x004E6220));
}

// Keep co-op utility keys out of the regular player bindings.
static bool CoOp_ReservedKey(u32 key,bool p2Keyboard)
{
    return key==0xCF || key==0x33 || (p2Keyboard && gCoOpConfig.Uses(key));
}

#include "coop/script/coop_script_location_runtime.h"
#include "coop/presentation/coop_exit_capture_runtime.h"

// This replaces only Pad_Update's call to PCINPUT_GetMappedStates. The retail
// input routine still produces every normal game input value.
static void __cdecl CoOpDebug_GetMappedStates(u32* mappedButtons, u32* mappedAxes)
{
    CoOp_LoadConfig();
    CoOp_HudBeginFrame();
	typedef void (__cdecl *RetailMappedStatesFn)(u32*, u32*);
	typedef u8 (__cdecl *RetailKeyStateFn)(u8);
	RetailMappedStatesFn getMappedStates = reinterpret_cast<RetailMappedStatesFn>(0x0050A190);
	RetailKeyStateFn getKeyState = reinterpret_cast<RetailKeyStateFn>(0x00501CB0);


    gCoOpPlayers.SyncPrimaryFromRetail();
    bool joined=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO)!=0;
    unsigned padMasks[4]={0,0,0,0},connectedSlots=0;
    if(joined && !gCoOpConfigOpen) for(unsigned slot=0;slot<4;++slot) {
        bool connected=false;
        padMasks[slot]=CoOp_ReadGamepad(slot,&connected);
        if(connected) connectedSlots|=1u<<slot;
    }
    CoOpControllerRoute route=CoOp_ChooseControllerRoute(connectedSlots,
        gCoOpConfig.singleControllerOwner,joined,gCoOpConfigOpen);
    bool p2Pad=route.p2Slot>=0;
    bool p2Keyboard=route.p2Keyboard;
    unsigned padMask=p2Pad?padMasks[route.p2Slot]:0;
    gCoOpReservePrimaryController=route.reserveNativeController;
    // Hide only P2-owned keyboard bindings from the original P1 mapper.
    // The selected P2 controller is reserved by the nested poll hook below.
    // Restore keyboard bindings immediately afterwards.
    u32* mappings=reinterpret_cast<u32*>(0x00568398);
    u32 savedKeys[14];
    for(int binding=0;binding<14;++binding)
    {
        savedKeys[binding]=mappings[binding*2+1];
        if(joined && !gCoOpConfigOpen && !*reinterpret_cast<i32*>(0x00ADB4D8) && mappings[binding*2]!=0x8000 && CoOp_ReservedKey(savedKeys[binding],p2Keyboard))
            mappings[binding*2+1]=0x4000;
    }
    getMappedStates(mappedButtons,mappedAxes);
    gCoOpReservePrimaryController=false;
    for(binding=0;binding<14;++binding) mappings[binding*2+1]=savedKeys[binding];
    CoOp_CaptureScriptLocationKeys(getKeyState(0xD1)!=0,getKeyState(0xC9)!=0); // PageDown / PageUp
    unsigned int held=0;
    const u8* keys=gCoOpConfig.keys+4;
    for(int action=0;action<7;++action)
        if(p2Pad?CoOp_PadDown(padMask,gCoOpConfig.pad[action+4]):
            p2Keyboard && getKeyState(keys[action])) held|=1<<action;
    gCoOpInput.pressed=held & ~gCoOpInput.held;
    gCoOpInput.held=held;
    gCoOpInput.x=p2Pad?CoOp_PadDown(padMask,gCoOpConfig.pad[1])-CoOp_PadDown(padMask,gCoOpConfig.pad[0]):
        p2Keyboard?((getKeyState(gCoOpConfig.keys[1])!=0)-(getKeyState(gCoOpConfig.keys[0])!=0)):0;
    gCoOpInput.z=p2Pad?CoOp_PadDown(padMask,gCoOpConfig.pad[2])-CoOp_PadDown(padMask,gCoOpConfig.pad[3]):
        p2Keyboard?((getKeyState(gCoOpConfig.keys[2])!=0)-(getKeyState(gCoOpConfig.keys[3])!=0)):0;


	// Read the retail DirectInput state so this hook follows the same input path
	// as the game.
	u8 exitDown = getKeyState(0xCF) != 0; // DIK_END, dedicated End key.
	if (exitDown && !gCoOpExitKeyWasDown) {
		__try {CoOp_SaveExitCapture();}
		__except(EXCEPTION_EXECUTE_HANDLER) {OutputDebugStringA("End-key capture failed; forcing exit.\n");}
		fflush(0); // TerminateProcess does not run CRT file cleanup.
		TerminateProcess(GetCurrentProcess(), 0);
	}
	gCoOpExitKeyWasDown = exitDown;
    if(gCoOpConfigOpen) {
        gCoOpInput=CoOpInput();
        gCoOpViewKeyWasDown=getKeyState(0x33)!=0;
        return;
    }

    // Joining is owned by the CO-OP menu/session; no gameplay join/drop keys.
    u8 viewDown = getKeyState(0x33) != 0; // DIK_COMMA: layout, independent of join/leave
    if (viewDown && !gCoOpViewKeyWasDown)
        gCoOpLayout = CoOp_NextView(gCoOpLayout);
    gCoOpViewKeyWasDown = viewDown;
}

#include "coop/camera/coop_aim.h"
// Restore split presentation after a player's native aim state ends.
static void CoOp_EndAimPresentation(CPlayer* player) {
    if(CoOp_LocalAim(gCoOpPlayers.Get(COOP_PLAYER_TWO)!=0,player->field_8EA!=0,
        gCoOpPlayers.Router().Resolve(gCoOpCinematicOwner)!=0)) {
        // Native aim updates its transition from the player's CB4/CE4 timers.
        // Keep those and the camera untouched, but don't export its bars or HUD
        // displacement as a full-screen cinematic that also stops P2 simulation.
        *reinterpret_cast<i32*>(0x00660F80)=0;
        *reinterpret_cast<i32*>(0x0060F76C)=0;
    }
}
// Native P1 AI retains its own gameplay callback. P2 is scheduled at the
// shared player-list boundary below, so a waiting/dead P1 cannot stop P2.
static void __fastcall CoOp_PlayerAI(CPlayer* player, void*)
{
    typedef void (__fastcall *AIFn)(CPlayer*, void*);
    if(gCoOpPlayers.InSecondaryContext()) {
        if(CoOp_LifeBefore(player)) reinterpret_cast<AIFn>(0x004C65C0)(player,0);
    CoOp_EndAimPresentation(player);
        return;
    }
    CoOp_SyncSections();
    CoOp_LifeResumePrimary(player);
    CoOp_CraneArrivalFrame(player);
    gCoOpTriggerOwner=1;
    CoOp_TraceWorld(0);
    bool parkedPartner=gCoOpScriptMotion.Partner(COOP_PLAYER_ONE);
    if(parkedPartner) {
        player->mVel.vx=player->mVel.vy=player->mVel.vz=0;
        player->mAcc.vx=player->mAcc.vy=player->mAcc.vz=0;
    }
    if(!parkedPartner && CoOp_LifeBefore(player)) reinterpret_cast<AIFn>(0x004C65C0)(player,0);
    CoOp_EndAimPresentation(player);
    CoOp_TraceWorld(1);
    gCoOpTriggerOwner=0;
    gCoOpPlayers.SyncPrimaryFromRetail();
    if(player!=gCoOpPlayers.Get(COOP_PLAYER_ONE)) return;
    CoOp_UpdateEnvironment();
}
// Append P2's native body tick after P1 and before world scripts.
static void CoOp_TickSecondAtFrameBoundary() {
    DWORD foregroundProcess=0;
    GetWindowThreadProcessId(GetForegroundWindow(),&foregroundProcess);
    const bool letterbox=*reinterpret_cast<i32*>(0x00660F80)!=0;
    const bool interactiveP2=letterbox && CoOp_SecondaryInteractiveScene();
    CPlayer* scriptedBody=gCoOpPlayers.Get(COOP_PLAYER_TWO);
    const bool scriptedP2=(scriptedBody && scriptedBody->field_1AC) ||
        (gCoOpScriptMotion.Owns(
        gCoOpPlayers.Router().Handle(COOP_PLAYER_TWO)) ||
        (letterbox && ((gCoOpCinematicOwner.slot==COOP_PLAYER_TWO &&
          CoOp_CinematicOwnerLive(gCoOpCinematicOwner)) ||
         interactiveP2)));
    if(CoOp_ShouldTickSecondary(*reinterpret_cast<i32*>(0x00ADB4D8)!=0,
        foregroundProcess==GetCurrentProcessId(),letterbox,scriptedP2)) {
        // The native P1 body keeps updating through cinematics. P2 needs the
        // same script-driven motion, but should not read held gameplay keys.
        CoOpInput neutral;
        if(interactiveP2) {
            ++gCoOpCraneInputTicks;
            if(gCoOpInput.pressed)++gCoOpCraneInputPresses;
        }
        CPlayer* p2=gCoOpPlayers.Get(COOP_PLAYER_TWO);
        bool suppressInput=scriptedP2 && !interactiveP2 &&
                (letterbox || (p2 && p2->field_1AC));
        // The hidden partner is shadowed after the acting body's native tick;
        // a second native tick here could make it collide with unloaded floor.
        if(!gCoOpScriptMotion.Partner(COOP_PLAYER_TWO))
            gCoOpPlayers.TickSecondary(suppressInput?neutral:gCoOpInput);
    }
}

// Draw through retail's existing UI renderer. Preserve all message/font state
// so our overlay cannot change the game's own HUD or subsequent menu text.
static void CoOp_DrawStatus()
{
    if (!gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO)) return;
    Font* font = reinterpret_cast<Font*>(0x0060D238);
    Font** fonts = reinterpret_cast<Font**>(0x005FAD5C);
    Font* statusFont = 0;
    for (int i = 0; i < 6; ++i)
        if (fonts[i] && !strcmp(fonts[i]->field_38, "sp_fnt02.fnt"))
            statusFont = fonts[i];
    if (!statusFont || !statusFont->pCharTab) return;

    u8 savedFont[sizeof(Font)];
    memcpy(savedFont, font, sizeof(Font));
    u16* scale = reinterpret_cast<u16*>(0x0060D5A4);
    u16* sort = reinterpret_cast<u16*>(0x0060D5A6);
    u8* justify = reinterpret_cast<u8*>(0x0060D5A8);
    u16 savedScale = *scale, savedSort = *sort;
    u8 savedJustify = *justify;
    memcpy(font, statusFont, sizeof(Font));
    *scale = 192;
    *sort = 4095;
    *justify = 1; // left aligned, in the game's 512 by 240 UI space
    typedef i32 (__cdecl *DrawTextFn)(i32, i32, const char*, i32, u32);
    DrawTextFn draw = reinterpret_cast<DrawTextFn>(0x00458700);
    const char* lines[7] = {
        "P2 - ORIGINAL PLAYER CONTROLLER",
        "INDEPENDENT PLAYER / CAMERA",
        "CO-OP SESSION",
        gCoOpLayout == COOP_SPLIT_LEFT_RIGHT ? ",: VIEW LEFT/RIGHT" :
            gCoOpLayout == COOP_SPLIT_TOP_BOTTOM ? ",: VIEW TOP/BOTTOM" : gCoOpLayout == COOP_VIEW_PLAYER_TWO ? ",: VIEW P2 FULL SCREEN" : ",: VIEW P1 FULL SCREEN",
        gCoOpViewActive ? (gCoOpSecondRendered ? "P2 CAMERA ACTIVE" : "P2 CAMERA UNAVAILABLE") : "",
        "ARROWS: MOVE  NUM1: JUMP",
        "NUM2/3: ATTACK  NUM0/4/5/6: WEBS"

    };
    int statusX = gCoOpViewActive && gCoOpLayout == COOP_SPLIT_TOP_BOTTOM ? 16 : 300;
    int statusY = gCoOpViewActive && gCoOpLayout == COOP_SPLIT_TOP_BOTTOM ? 142 : 24;
    for (int line = 0; line < 7; ++line)
    {
        font->mRed = font->mGreen = font->mBlue = 0;
        draw(statusX + 1, statusY + 1 + line * 12, lines[line], 0, 0x1000);
        font->mRed = line == 0 ? 100 : 220;
        font->mGreen = 240;
        font->mBlue = line == 0 ? 130 : 220;
        draw(statusX, statusY + line * 12, lines[line], 0, 0x1000);
    }
    memcpy(font, savedFont, sizeof(Font));
    *scale = savedScale;
    *sort = savedSort;
    *justify = savedJustify;
}

static CCamera* gCoOpBossCamera;
static ECameraMode gCoOpBossPreviousMode;
static unsigned long gCoOpCameraTicks[2],gCoOpCameraModeChanges[2];
static int gCoOpCameraLastMode[2]={-1,-1};
// Save one camera's native state for per-player diagnostic comparison.
static void CoOp_RecordCameraTick(unsigned slot,CCamera* camera) {
    if(!camera || slot>1)return;
    ++gCoOpCameraTicks[slot];
    int mode=(int)camera->mCameraMode;
    if(gCoOpCameraLastMode[slot]!=mode) {
        ++gCoOpCameraModeChanges[slot];gCoOpCameraLastMode[slot]=mode;
    }
}
// Mirror the authored boss-camera mode into P2's camera bank.
static void CoOp_SyncBossCamera() {
    CCamera* second=gCoOpPlayers.SecondaryCamera();
    CCamera* first=*reinterpret_cast<CCamera**>(0x0056F3B8);
    bool bossActive=false;
    if(gCoOpSession.enabled && first && second && first->mCameraMode==CAMERAMODE_USER) {
        for(CItem* item=*reinterpret_cast<CItem**>(0x0056E990);item;item=item->mNextItem)
            if((item->mType==310 || item->mType==307) &&
                reinterpret_cast<CBaddy*>(item)->mHealth>0) {
                bossActive=true;break;
            }
    }
    if(bossActive) {
        // USER dispatches retail CM_Boss3, which uses the shared boss target
        // and this camera's own player tripod. P1 already enters it in combat.
        if(gCoOpBossCamera!=second) {
            gCoOpBossCamera=second;
            gCoOpBossPreviousMode=second->mCameraMode;
        }
        second->mCameraMode=CAMERAMODE_USER;
    } else if(gCoOpBossCamera==second && second) {
        if(second->mCameraMode==CAMERAMODE_USER)
            second->mCameraMode=gCoOpBossPreviousMode;
        gCoOpBossCamera=0;
    } else gCoOpBossCamera=0;
    // Retail boss code enables the camera's eight-item obstruction ray on P1.
    // The second camera has its own ray/list, but boss scripts only toggle
    // P1's flag. Mirror that camera feature wherever both use USER mode;
    // the native P2 camera tick then chooses props near its own tripod.
    if(gCoOpSession.enabled && first && second &&
        first->mCameraMode==CAMERAMODE_USER &&
        second->mCameraMode==CAMERAMODE_USER)
        second->field_F9=first->field_F9;
}
// Run retail's player list, then P2 and shared-world maintenance once per frame.
static void __cdecl CoOp_UpdatePlayers(void* list,i32 flags) {
    gCoOpPlayers.SyncPrimaryFromRetail();
    // Recover interrupted holds before a native callback can pin a victim
    // again. This runs independently of which enemies will tick this frame.
    if(!*reinterpret_cast<i32*>(0x00ADB4D8))CoOp_GrabFrame();
    // Retail updates P1's body list here, before pending scripts and shared
    // enemy/world lists. Append one P2 body tick at the same frame boundary.
    reinterpret_cast<void (__cdecl*)(void*,i32)>(0x00460FC0)(list,flags);
    gCoOpPlayers.SyncPrimaryFromRetail();
    if(gCoOpPlayers.Get(COOP_PLAYER_ONE)) {
        CoOp_SyncSections();
        CoOp_TickSecondAtFrameBoundary();
        CoOp_ScriptMotionFrame();
        CoOp_UpdateEnvironment();
        CoOp_TraceWorld(2);
        CoOp_LifeFrame();
    }
    gCoOpInput.pressed=0;
}
// Run retail camera updates, then P2's native camera in its own context.
static void __cdecl CoOp_UpdateCameras(void* list,i32 flags) {
    CoOp_FlushCameraPolicy();
    CoOp_FlushCameraSettings();
    reinterpret_cast<void (__cdecl*)(void*,i32)>(0x00460FC0)(list,flags);
    CoOp_RecordCameraTick(0,*reinterpret_cast<CCamera**>(0x0056F3B8));
    CoOp_ReleaseDeadCinematic();
    // Camera updates follow both players' single body ticks, including when
    // one player is waiting at a checkpoint or owns a scripted route.
    if(!*reinterpret_cast<i32*>(0x00ADB4D8)) {
        CoOp_SyncBossCamera();
        gCoOpPlayers.UpdateSecondaryCamera();
        CoOp_RecordCameraTick(1,gCoOpPlayers.SecondaryCamera());
    }
}
// Draw P1 and P2 bodies while maintaining scripted shadows and join state.
static void __cdecl CoOp_RenderPlayers(void* list)
{
    typedef void (__cdecl *RenderFn)(void*);
    gCoOpPlayers.SyncPrimaryFromRetail();
    CoOp_ScriptMotionFrame(true);
    CPlayer* primary=gCoOpPlayers.Get(COOP_PLAYER_ONE);
    u16 savedFlags=primary?primary->mFlags:0;
    if(primary && gCoOpScriptMotion.Partner(COOP_PLAYER_ONE))primary->mFlags|=1;
    reinterpret_cast<RenderFn>(0x004739A0)(list);
    if(primary)primary->mFlags=savedFlags;
    gCoOpPlayers.SyncPrimaryFromRetail();
    CoOp_LifeResumePrimary(gCoOpPlayers.Get(COOP_PLAYER_ONE));
    if(gCoOpPlayers.ConsumePrimaryReplacement()) gCoOpSession.LevelEnded();
    if(gCoOpSession.pending && gCoOpPlayers.SyncPrimaryFromRetail()) {
        gCoOpLayout=(CoOpSplitLayout)gCoOpConfig.layout;
        gCoOpPlayers.SetCarry(&gCoOpSession.carry);
        gCoOpPlayers.RequestSecondarySpawn();
    }
    gCoOpPlayers.RenderSecondary(gCoOpScriptMotion.Partner(COOP_PLAYER_TWO));
    if(gCoOpSession.pending && gCoOpPlayers.Get(COOP_PLAYER_TWO)) gCoOpSession.Joined();
    CoOp_SyncSections();
}

// Correct a chase camera's target position for a narrow split viewport.
static void CoOp_CenterNarrowCamera(SCamera* frame,CPlayer* primary) {
    if(!frame || !primary || primary->field_8EA || gCoOpLayout!=COOP_SPLIT_LEFT_RIGHT) return;
                double rx=frame->Transform.m[0][0]/4096.0;
                double ry=frame->Transform.m[1][0]/4096.0;
                double rz=frame->Transform.m[2][0]/4096.0;
                double shift=(primary->mPos.vx/4096.0-frame->Position.vx)*rx+
                    (primary->mPos.vy/4096.0-frame->Position.vy)*ry+
                    (primary->mPos.vz/4096.0-frame->Position.vz)*rz;
                if(shift>96) shift=96; if(shift< -96) shift=-96;
                CVector from,to;
                from.vx=frame->Position.vx*4096; from.vy=frame->Position.vy*4096;
                from.vz=frame->Position.vz*4096;
                to=from; to.vx+=(i32)(shift*rx*4096); to.vy+=(i32)(shift*ry*4096);
                to.vz+=(i32)(shift*rz*4096);
                if(reinterpret_cast<CoOpClearRayFn>(0x004E67A0)(&from,&to,0,0))
                {
                    frame->Position.vx=to.vx/4096; frame->Position.vy=to.vy/4096;
                    frame->Position.vz=to.vz/4096;
                    frame->Transform.t[0]=frame->Position.vx;
                    frame->Transform.t[1]=frame->Position.vy;
                    frame->Transform.t[2]=frame->Position.vz;
                }
}

// Detect the chase segment whose camera has an authored building constraint.
static bool CoOp_InChaseBuilding() {
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[10] && !_strnicmp((const char*)resource,"L3A2_G",7))return true;
    }
    return false;
}
// Choose the viewport/camera pair for each render pass without changing simulation.
static void __cdecl CoOp_RenderSetup(void* camera, void* viewport, u32* orderingTable)
{
    CoOpUiRemakeScan();
    CoOp_TraceWorld(3);
    typedef void (__cdecl *SetupFn)(void*, void*, u32*);
    gCoOpPlayers.SyncPrimaryFromRetail();
    if((CoOp_LifeWaiting(0) || CoOp_LifeWaiting(1)) &&
        (gCoOpLayout==COOP_SPLIT_OFF || gCoOpLayout==COOP_VIEW_PLAYER_TWO))
        gCoOpLayout=(CoOpSplitLayout)gCoOpConfig.layout;
    gCoOpViewActive=false; gCoOpSecondRendered=false;
    gCoOpMarkerProjectionValid[0]=gCoOpMarkerProjectionValid[1]=false;
    gCoOpEffectsCamera=0;gCoOpEffectsHandled=false;
    gCoOpEffectsCalls[0]=gCoOpEffectsCalls[1]=gCoOpEffectsUnknown=0;
    gCoOpOriginalViewport=viewport; gCoOpOrderingTable=orderingTable;
    CoOp_UpdateEnvironment();
    CPlayer* primary=gCoOpPlayers.Get(COOP_PLAYER_ONE);
    bool gameplay=camera==reinterpret_cast<void*>(0x0056F1B0) && viewport &&
        primary && (!*reinterpret_cast<i32*>(0x00660F80) || CoOp_LifeWaiting(0) || CoOp_LifeWaiting(1));
    void* selectedCamera=camera;
    void* selectedViewport=viewport;
    if(*reinterpret_cast<i32*>(0x00660F80) &&
       ((gCoOpCinematicOwner.slot==COOP_PLAYER_TWO &&
         CoOp_CinematicOwnerLive(gCoOpCinematicOwner)) ||
        CoOp_SecondaryInteractiveScene()) &&
       gCoOpPlayers.CopySecondaryCamera(gCoOpSecondCamera))
        selectedCamera=gCoOpSecondCamera; // one full-screen authored cinematic
    if(gameplay)
    {
        gCoOpFullView=*reinterpret_cast<CoOpRetailViewport*>(viewport);
        CoOp_ApplyDisplayAspect(gCoOpFullView,
            *reinterpret_cast<i32*>(0x00568154),*reinterpret_cast<i32*>(0x00568158));
        selectedViewport=&gCoOpFullView;
        CPlayer* body=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO);
        if(body && CoOp_BuildPrimaryView(gCoOpFullView,gCoOpLayout,gCoOpView))
        {
            gCoOpViewActive=CoOp_BuildSecondaryView(gCoOpFullView,gCoOpLayout,gCoOpSecondView);
            memcpy(gCoOpPrimaryCamera,camera,sizeof(gCoOpPrimaryCamera));
            selectedCamera=gCoOpPrimaryCamera; selectedViewport=&gCoOpView;
            CoOp_CenterNarrowCamera(reinterpret_cast<SCamera*>(gCoOpPrimaryCamera),primary);
            gCoOpCameraOwner=gCoOpPlayers.Router().Handle(COOP_PLAYER_TWO);
            if(!gCoOpPlayers.CopySecondaryCamera(gCoOpSecondCamera))
                gCoOpViewActive=false;
            else CoOp_CenterNarrowCamera(reinterpret_cast<SCamera*>(gCoOpSecondCamera),body);
        }
    }
    reinterpret_cast<SetupFn>(0x00472DC0)(selectedCamera,selectedViewport,orderingTable);
    if(gameplay && gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO) &&
        (selectedCamera==gCoOpPrimaryCamera ||
         (gCoOpLayout==COOP_SPLIT_OFF && selectedCamera==camera))) {
        if(selectedCamera==camera)
            memcpy(gCoOpPrimaryCamera,camera,sizeof(gCoOpPrimaryCamera));
        memcpy(gCoOpMarkerProjection[0],reinterpret_cast<void*>(0x0056E570),
            sizeof(gCoOpMarkerProjection[0]));
        gCoOpMarkerProjectionValid[0]=true;
    }
    if(gameplay && gCoOpPlayers.Get(COOP_PLAYER_TWO) &&
        *reinterpret_cast<void**>(0x006B791C) && !*reinterpret_cast<i32*>(0x006B78F8))
        gCoOpEffectsCamera=selectedCamera;
}

static unsigned gCoOpChaseSpecialDraws[3];
static void __fastcall CoOp_DrawChaseSpecial(void* item,void*) {
    void** methods=*reinterpret_cast<void***>(item);
    reinterpret_cast<void (__fastcall*)(void*,void*)>(methods[5])(item,0);
}
// Submit authored chase effects visible from P2's camera.
static void CoOp_DrawChaseSpecialList(u32 address,u16 firstType,u16 secondType) {
    for(CItem* item=*reinterpret_cast<CItem**>(address);item;item=item->mNextItem) {
        if(item->mType!=firstType && item->mType!=secondType)continue;
        // Retail's render loop invokes virtual slot 5 after RenderSuperItem.
        // These searchlight/reticle callbacks are absent from RenderList.
        CoOp_DrawProjectedPlayer(item,gCoOpSecondCamera,CoOp_DrawChaseSpecial);
        ++gCoOpChaseSpecialDraws[item->mType-321];
    }
}
// Render the shared world a second time using P2's camera and clip rectangle.
static bool CoOp_SubmitSecondView()
{
    // Software rasterizer has different submission semantics; use the existing
    // reserved pane there. DX7 submits pretransformed vertices, with flags=0.
    LPDIRECT3DDEVICE7 device=*reinterpret_cast<LPDIRECT3DDEVICE7*>(0x006B791C);
    if(!device || *reinterpret_cast<i32*>(0x006B78F8) ||
        !gCoOpPlayers.Router().Resolve(gCoOpCameraOwner)) return false;
    D3DVIEWPORT7 original, first, second;
    if(FAILED(device->GetViewport(&original))) return false;
    first=second=original;
    float sx=(float)*reinterpret_cast<i32*>(0x00568154)/
        *reinterpret_cast<u32*>(0x0061B5FC);
    float sy=(float)*reinterpret_cast<i32*>(0x00568158)/
        *reinterpret_cast<u32*>(0x00628614);
    first.dwX=(DWORD)(gCoOpView.left*sx); first.dwY=(DWORD)(gCoOpView.top*sy);
    first.dwWidth=(DWORD)(gCoOpView.right*sx)-first.dwX;
    first.dwHeight=(DWORD)(gCoOpView.bottom*sy)-first.dwY;
    second.dwX=(DWORD)(gCoOpSecondView.left*sx); second.dwY=(DWORD)(gCoOpSecondView.top*sy);
    second.dwWidth=(DWORD)(gCoOpSecondView.right*sx)-second.dwX;
    second.dwHeight=(DWORD)(gCoOpSecondView.bottom*sy)-second.dwY;
    if(!first.dwWidth || !first.dwHeight || !second.dwWidth || !second.dwHeight) return false;
    if(FAILED(device->SetViewport(&first))) return false;
    typedef void (__cdecl *EndFn)(i32);
    typedef void (__cdecl *BeginFn)(u32,i32);
    typedef void (__cdecl *RenderFn)(void*);
    typedef void (__cdecl *SetupFn)(void*,void*,u32*);
    EndFn end=reinterpret_cast<EndFn>(0x00506160);
    BeginFn begin=reinterpret_cast<BeginFn>(0x00505E00);
    end(0); // submit P1, do not present
    if(FAILED(device->SetViewport(&second)))
    {
        device->SetViewport(&original);
        // No replay/clear on recovery: leave a valid continuation below.
    }
    else
    {
        begin(3,-1); // retail clears only the selected P2 viewport
        u8 matrixState[0x248], planeState[0x50], cameraPlanes[0x40];
        u8 gteState[0x100], fogProjection[0x1C];
        memcpy(gteState,reinterpret_cast<void*>(0x00610B20),sizeof(gteState));
        memcpy(fogProjection,reinterpret_cast<void*>(0x0056817C),sizeof(fogProjection));
        memcpy(matrixState,reinterpret_cast<void*>(0x0056E570),sizeof(matrixState));
        memcpy(planeState,reinterpret_cast<void*>(0x0065CEB8),sizeof(planeState));
        memcpy(cameraPlanes,reinterpret_cast<void*>(0x00628620),sizeof(cameraPlanes));
        // Setup's fog interpolation is a simulation side effect: suppress it
        // on the extra view, rather than advancing it twice per game frame.
        i32 fogTicks=*reinterpret_cast<i32*>(0x0064E558);
        *reinterpret_cast<i32*>(0x0064E558)=0;
        reinterpret_cast<SetupFn>(0x00472DC0)(gCoOpSecondCamera,&gCoOpSecondView,gCoOpOrderingTable);
        memcpy(gCoOpMarkerProjection[1],reinterpret_cast<void*>(0x0056E570),
            sizeof(gCoOpMarkerProjection[1]));
        gCoOpMarkerProjectionValid[1]=true;
        *reinterpret_cast<i32*>(0x0064E558)=fogTicks;
        // The render owner is distinct from the lists being drawn. Retail
        // RenderSuperItem consults MechList for owner-specific ceiling fading.
        CPlayer* savedRenderOwner=*reinterpret_cast<CPlayer**>(0x006A9038);
        CCamera* savedRenderCamera=*reinterpret_cast<CCamera**>(0x0056F3B8);
        *reinterpret_cast<CPlayer**>(0x006A9038)=gCoOpPlayers.Get(COOP_PLAYER_TWO);
        *reinterpret_cast<CCamera**>(0x0056F3B8)=gCoOpPlayers.SecondaryCamera();
        void* background=*reinterpret_cast<void**>(0x0056E89C);
        if(background) reinterpret_cast<RenderFn>(0x004747C0)(background);
        // Same live actor lists, rendered only. No AI, input, triggers or Bit_Display.
        // ControlBaddyList contains the chase searchlight and sniper target.
        // Their SpecialRenderer callbacks must run against each view's camera.
        const u32 lists[]={0x006B2EFC,0x0060DAAC,0x006A9038,0x006A903C,
            0x0060DAB0,0x0056E990,0x0056E994,0x0060FB94,0x0060DAB8,0x0056EFE4};
        for(u32 i=0;i<sizeof(lists)/sizeof(lists[0]);++i)
        {
            void* list=lists[i]==0x006A9038 ? savedRenderOwner : *reinterpret_cast<void**>(lists[i]);
            if(list)
            {
                CoOpVisibilityUnion<CItem> visibility(reinterpret_cast<CItem*>(list));
                reinterpret_cast<RenderFn>(0x004739A0)(list);
            }
        }
        CoOp_DrawChaseSpecialList(0x0056E994,322,323);
        CoOp_DrawChaseSpecialList(0x0056E990,321,321);
        gCoOpPlayers.DrawSecondaryOnly();
        CoOp_DrawWorldEffects(gCoOpSecondCamera,2);
        *reinterpret_cast<CPlayer**>(0x006A9038)=savedRenderOwner;
        *reinterpret_cast<CCamera**>(0x0056F3B8)=savedRenderCamera;
        CoOp_HudCaptureSecondView();
        end(0); // submit P2, still no present
        memcpy(reinterpret_cast<void*>(0x0056E570),matrixState,sizeof(matrixState));
        memcpy(reinterpret_cast<void*>(0x0065CEB8),planeState,sizeof(planeState));
        memcpy(reinterpret_cast<void*>(0x00628620),cameraPlanes,sizeof(cameraPlanes));
        memcpy(reinterpret_cast<void*>(0x00610B20),gteState,sizeof(gteState));
        memcpy(reinterpret_cast<void*>(0x0056817C),fogProjection,sizeof(fogProjection));
        gCoOpSecondRendered=true;
    }
    device->SetViewport(&original);
    // Resume queued HUD/effects without retail BeginScene's framebuffer clear.
    // These are the DX7 branch's queue initialization steps from DXPOLY_BeginScene.
    if(SUCCEEDED(device->BeginScene()))
    {
        memset(reinterpret_cast<void*>(0x006B7AA0),0,0x1001*sizeof(u32));
        *reinterpret_cast<u8*>(0x006BBAA4)=1;
        *reinterpret_cast<u8*>(0x00AC08C4)=1;
        *reinterpret_cast<float*>(0x0073C77C)=0.01f;
        *reinterpret_cast<float*>(0x00AC07B8)=-0.2f;
    }
    return gCoOpSecondRendered;
}
// Snapshot native ceiling-camera preset values for diagnostics.
static void CoOp_ReadCeilingPreset(void* output) {
    short* values=static_cast<short*>(output);
    values[0]=*reinterpret_cast<short*>(0x006A825C);
    values[1]=*reinterpret_cast<short*>(0x006A8CAC);
}
// Write live camera, section and player observations for End-key capture.
static void CoOp_RecordViewDiagnostics()
{
    static DWORD last=0;
    static FILE* log=0;
    DWORD now=GetTickCount();
    CPlayer* diagnosticP2=gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO);
    if(now-last<1000 || (!gCoOpSession.enabled && !diagnosticP2)) return;
    last=now;
    if(!log)
    {
        char path[MAX_PATH];
        if(!GetModuleFileNameA(0,path,sizeof(path))) return;
        char* slash=strrchr(path,'\\');
        if(!slash || slash-path+23>=MAX_PATH) return;
        strcpy(slash+1,"coop-diagnostics.log");
        log=fopen(path,"a");
        if(!log) return;
        fprintf(log,"\nCOOP CPLAYER-31-CHASE-BOSSES-RACE: t layout active submitted P1xyz P2xyz camera2xyz inputxz dt ground\n");
    }
    CPlayer* p1=gCoOpPlayers.Get(COOP_PLAYER_ONE);
    CPlayer* p2=diagnosticP2;
    void* nativeCamera=*reinterpret_cast<void**>(0x0056F3B8);
    fprintf(log,"session: enabled=%d pending=%d P1=%p P2=%p camera=%p mode=%d pause=%d letterbox=%d\n",
        (int)gCoOpSession.enabled,(int)gCoOpSession.pending,p1,p2,nativeCamera,
        nativeCamera?*reinterpret_cast<i32*>(static_cast<u8*>(nativeCamera)+0x2A0):-1,
        *reinterpret_cast<i32*>(0x00ADB4D8),*reinterpret_cast<i32*>(0x00660F80));
    unsigned controlBodies=0;
    for(CItem* control=*reinterpret_cast<CItem**>(0x0056E994);control && controlBodies<1024;
        control=control->mNextItem)++controlBodies;
    fprintf(log,"chase: control-bodies=%u collision-scripts=%u,%u second-view=%d effects=%u,%u\n",
        controlBodies,gCoOpCollisionScripts[0],gCoOpCollisionScripts[1],
        (int)gCoOpSecondRendered,gCoOpEffectsCalls[0],gCoOpEffectsCalls[1]);
    fprintf(log,"chase-routing: building=%d chopper-targets=%u,%u sniper-light-missile=%u,%u\n",
        (int)CoOp_InChaseBuilding(),gCoOpChopperTargets[0],gCoOpChopperTargets[1],
        gCoOpChaseTargets[0],gCoOpChaseTargets[1]);
    fprintf(log,"chase-actor-pulses: P1=%u P2=%u\n",gCoOpChaseActorPulses[0],gCoOpChaseActorPulses[1]);
    fprintf(log,"transitions: shared-level-changes=%u kill-in-P2-context=%u\n",
        gCoOpSharedLevelChanges,gCoOpKillInSecondaryContext);
    fprintf(log,"venom-race-failure: checked=%u trailing-player-deferred=%u\n",
        gCoOpVenomRaceFailureChecks,gCoOpVenomRaceFailureDeferred);
    fprintf(log,"venom-race-entry: P1=%u P2=%u retained-sections=%u\n",
        gCoOpRaceEntryScripts[0],gCoOpRaceEntryScripts[1],gCoOpSections.Count());
    fprintf(log,"boss-targets: rhino=%u,%u venom=%u,%u\n",
        gCoOpRhinoTargets[0],gCoOpRhinoTargets[1],
        gCoOpVenomTargets[0],gCoOpVenomTargets[1]);
    CoOp_RhinoStatus(log);
    CoOp_ArmorStatus(log);
    CoOp_CameraPolicyStatus(log);
    fprintf(log,"lizman-targets: P1=%u P2=%u\n",gCoOpLizManTargets[0],gCoOpLizManTargets[1]);
    fprintf(log,"projectile-collision: installed=%u hits=%u,%u\n",gCoOpProjectileCollisionInstalled,gCoOpProjectileHits[0],gCoOpProjectileHits[1]);
    fprintf(log,"join: P1health=%d P1script=%u pending=%d requested=%d attempts=%u failures=%u reason=%u blocked=%d strategy=%u revive-on-level=%d P1xyz=%ld,%ld,%ld\n",
        p1?p1->mHealth:-1,p1?(unsigned)(u8)p1->field_1AC:0,(int)gCoOpSession.pending,(int)gCoOpPlayers.SpawnRequested(),
        gCoOpPlayers.SpawnAttempts(),gCoOpPlayers.SpawnFailures(),gCoOpPlayers.SpawnFailureReason(),
        (int)CoOp_SpawnBlocked(),gCoOpPlayers.SpawnStrategy(),(int)gCoOpRevivePrimaryNextLevel,
        p1?p1->mPos.vx/4096:0,p1?p1->mPos.vy/4096:0,p1?p1->mPos.vz/4096:0);
    fprintf(log,"boss-hud: commands=%u split=%d offsets=%.1f,%.1f %.1f,%.1f\n",
        gHudBossLast.sourceCount,(int)gHudBossLast.split,
        gHudBossLast.dx[0],gHudBossLast.dy[0],gHudBossLast.dx[1],gHudBossLast.dy[1]);
    fprintf(log,"race-hud: commands=%u active=%d offset=%.1f,%.1f\n",
        gHudRaceLast.sourceCount,(int)gHudRaceLast.active,gHudRaceLast.dx,gHudRaceLast.dy);
    fprintf(log,"compass: active=%u draws=%u,%u target=%ld,%ld,%ld\n",
        (unsigned)*reinterpret_cast<u8*>(0x0060F77C),gHudCompassCount[0],gHudCompassCount[1],
        *reinterpret_cast<i32*>(0x0060F708),*reinterpret_cast<i32*>(0x0060F70C),
        *reinterpret_cast<i32*>(0x0060F710));
    for(CItem* bossItem=*reinterpret_cast<CItem**>(0x0056E990);bossItem;
        bossItem=bossItem->mNextItem) if(bossItem->mType==310) {
        CScorpion* boss=reinterpret_cast<CScorpion*>(bossItem);
        fprintf(log,"scorpion: target-phase=%d handle=%p health=%d routed-ticks=%u,%u\n",
            boss->field_BF8,boss->hCurrentTarget.pWhatever,boss->mHealth,
            gCoOpEnemyCount[0],gCoOpEnemyCount[1]);
        break;
    }
    SCamera* cam=reinterpret_cast<SCamera*>(gCoOpSecondCamera);
    if(!p1 || !p2) return;
    SCamera* firstFrame=reinterpret_cast<SCamera*>(gCoOpPrimaryCamera);
    CCamera* firstCamera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    CCamera* secondCamera=gCoOpPlayers.SecondaryCamera();
    if(firstCamera && secondCamera) fprintf(log,
        "camera-detail: view-angles=%d,%d,%d / %d,%d,%d body-yaw=%d,%d cam-yaw=%d,%d surface=%ld,%ld flags=%02x,%02x\n",
        firstFrame->Angles.vx,firstFrame->Angles.vy,firstFrame->Angles.vz,
        cam->Angles.vx,cam->Angles.vy,cam->Angles.vz,
        p1->mAngles.vy,p2->mAngles.vy,
        firstCamera->field_23A,secondCamera->field_23A,
        p1->field_E1C,p2->field_E1C,firstCamera->field_F8,secondCamera->field_F8);
    fprintf(log,"%lu %d %d %d %ld,%ld,%ld %ld,%ld,%ld %ld,%ld,%ld %d,%d %ld %d\n",
        now,(int)gCoOpLayout,(int)gCoOpViewActive,(int)gCoOpSecondRendered,
        p1->mPos.vx/4096,p1->mPos.vy/4096,p1->mPos.vz/4096,
        p2->mPos.vx/4096,p2->mPos.vy/4096,p2->mPos.vz/4096,
        cam->Position.vx,cam->Position.vy,cam->Position.vz,
        gCoOpInput.x,gCoOpInput.z,p1->field_80,-1);
    unsigned int suspended=0;
    for(CItem* item=*reinterpret_cast<CItem**>(0x0060DAB4);item;item=item->mNextItem)
        ++suspended;
    fprintf(log,"activation: radius=%lu suspended=%u p1flags=%04x p2flags=%04x\n",
        *reinterpret_cast<u32*>(0x0054E33C),suspended,p1->mFlags,p2->mFlags);
    fprintf(log,"life: installed=%d active=%d HP=%d,%d waiting=%d,%d recover=%d,%d revive=%d,%d checkpoint=%d section=%d both-dead=%d\n",
        (int)gCoOpLifeInstalled,(int)gCoOpLife.active,p1->mHealth,p2->mHealth,
        (int)gCoOpLife.slot[0].waiting,(int)gCoOpLife.slot[1].waiting,
        (int)gCoOpLife.slot[0].recover,(int)gCoOpLife.slot[1].recover,
        (int)gCoOpLife.slot[0].revive,(int)gCoOpLife.slot[1].revive,
        gCoOpLifeCheckpoint,gCoOpLifeSection,(int)gCoOpLife.BothDead());
    fprintf(log,"fall-triggers: P1=%u P2=%u\n",gCoOpFallEvents[0],gCoOpFallEvents[1]);
    fprintf(log,"environment: tracked=%u held=%u\n",gCoOpEnvironment.Tracked(),gCoOpEnvironment.Held());
    fprintf(log,"controller: retail CPlayer ticks=%lu health=%d\n",
        gCoOpPlayers.SecondaryTicks(),gCoOpPlayers.Get(COOP_PLAYER_TWO)->mHealth);
    fprintf(log,"sections: active=%d profile=%u race=%d sewer=%d tracked=%u commands-global=%u P1=%u P2=%u both=%u\n",
        (int)gCoOpSectionsActive,(unsigned)gCoOpSectionProfile,(int)gCoOpRaceSectionsActive,(int)gCoOpSewerSectionsActive,
        gCoOpSections.Count(),gCoOpSectionCalls[0],
        gCoOpSectionCalls[1],gCoOpSectionCalls[2],gCoOpSectionCalls[3]);
    fprintf(log,"section-contacts: P1=%u P2=%u (terrain-only, independent of shared trigger latch)\n",
        gCoOpSectionTouches[0],gCoOpSectionTouches[1]);
    fprintf(log,"ceiling: P1attached=%u preset=%ld normalY=%d P2attached=%u preset=%ld normalY=%d\n",
        p1->field_8E9,p1->field_540,p1->field_A8.vy,p2->field_8E9,p2->field_540,p2->field_A8.vy);
    short ceilingP1[2],ceilingP2[2]={0,0};
    CoOp_ReadCeilingPreset(ceilingP1);
    gCoOpPlayers.WithSecondaryContext(CoOp_ReadCeilingPreset,ceilingP2);
    fprintf(log,"ceiling-presets: P1xz=%d y=%d P2xz=%d y=%d\n",
        ceilingP1[0],ceilingP1[1],ceilingP2[0],ceilingP2[1]);
    CCamera* primaryCamera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    fprintf(log,"aim: P1active=%u reticle=%u P2active=%u reticle=%u\n",
        (unsigned)p1->field_8EA,(unsigned)(u8)p1->field_DE4,
        (unsigned)p2->field_8EA,(unsigned)(u8)p2->field_DE4);
    fprintf(log,"camera-lifecycle: letterbox=%ld cinematic-owner=%u P1mode=%d P1tripod=%p\n",
        *reinterpret_cast<i32*>(0x00660F80),gCoOpCinematicOwner.slot,
        primaryCamera?(int)primaryCamera->mCameraMode:-1,primaryCamera?primaryCamera->mTripod:0);
    const CoOpPlayerRouter::Participant* motionActor=
        gCoOpPlayers.Router().Resolve(gCoOpScriptMotion.owner);
    const u8* motionBytes=motionActor?reinterpret_cast<const u8*>(motionActor->player):0;
    fprintf(log,"script-motion: active=%d owner=%u waiting-partner=%d P1native=%u P2native=%u ticks=%lu motion=%lu stalled=%lu native-active=%u native-time=%ld cursor=%p entry=%d,%d,%d actor=%d,%d,%d P2shadow=%d\n",
        (int)gCoOpScriptMotion.active,gCoOpScriptMotion.owner.slot,
        (int)gCoOpScriptMotion.waitingForPartner,(unsigned)(u8)p1->field_1AC,
        (unsigned)(u8)p2->field_1AC,gCoOpScriptMotion.ticks,
        gCoOpScriptMotion.motionTicks,gCoOpScriptMotion.stalledTicks,
        motionBytes?(unsigned)motionBytes[0x1B4]:0,
        motionBytes?*reinterpret_cast<const i32*>(motionBytes+0x1B0):0,
        gCoOpScriptMotion.cursor,gCoOpScriptMotion.entry.vx/4096,
        gCoOpScriptMotion.entry.vy/4096,gCoOpScriptMotion.entry.vz/4096,
        gCoOpScriptMotion.last.vx/4096,gCoOpScriptMotion.last.vy/4096,
        gCoOpScriptMotion.last.vz/4096,(int)gCoOpPlayers.SecondaryShadowed());
    fprintf(log,"camera-ticks: P1=%lu P2=%lu mode-changes=%lu,%lu last-modes=%d,%d\n",
        gCoOpCameraTicks[0],gCoOpCameraTicks[1],gCoOpCameraModeChanges[0],
        gCoOpCameraModeChanges[1],gCoOpCameraLastMode[0],gCoOpCameraLastMode[1]);
    fprintf(log,"camera-scripts: owned-P1=%u owned-P2=%u unowned=%u last-node=%d,%d,%d last-op=%u,%u,%u\n",
        gCoOpAuthoredCameraScripts[0],gCoOpAuthoredCameraScripts[1],gCoOpAuthoredCameraScripts[2],
        gCoOpLastCameraScriptNode[0],gCoOpLastCameraScriptNode[1],gCoOpLastCameraScriptNode[2],
        gCoOpLastCameraScriptOp[0],gCoOpLastCameraScriptOp[1],gCoOpLastCameraScriptOp[2]);
    CCamera* secondaryCamera=gCoOpPlayers.SecondaryCamera();
    if(secondaryCamera) fprintf(log,"P2camera: mode=%d tripod=%p expected=%p enabled=%u focus=%p occlusion=%u P1occlusion=%u\n",
        (int)secondaryCamera->mCameraMode,secondaryCamera->mTripod,p2,
        secondaryCamera->field_F8,secondaryCamera->field_13C,
        secondaryCamera->field_F9,primaryCamera?primaryCamera->field_F9:0);
    fprintf(log,"interactions: installed=%d camera-update=%d pickup-scripts=%u,%u enemy-ticks=%u,%u stale-scripts=%u\n",
        (int)gCoOpInteractionsInstalled,(int)gCoOpCameraUpdateInstalled,gCoOpPickupCount[0],gCoOpPickupCount[1],
        gCoOpEnemyCount[0],gCoOpEnemyCount[1],gCoOpStaleScripts);
    fprintf(log,"chase-hit-routing: bullet-move-targets=%u,%u P2-input-held=%02x pressed=%02x crane-scene=%d ticks=%u presses=%u auto-exits=%u auto-owner=%u arrival=%u pending=%u requested=%u skip-node=%ld skip-flag=%u\n",
        gCoOpBulletTargets[0],gCoOpBulletTargets[1],gCoOpInput.held,gCoOpInput.pressed,
        (int)CoOp_SecondaryInteractiveScene(),gCoOpCraneInputTicks,gCoOpCraneInputPresses,
        gCoOpCraneAutoCount,gCoOpCraneAutoOwner,gCoOpCraneArrivalCount,
        gCoOpCraneArrivalPending,gCoOpCraneArrivalRequested,p1->field_1A8,
        *reinterpret_cast<u8*>(0x0060CFC4));
    fprintf(log,"effects: installed=%u P1lists=%u P2lists=%u unknown-slots=%u\n",
        gCoOpEffectsInstalled,gCoOpEffectsCalls[0],gCoOpEffectsCalls[1],gCoOpEffectsUnknown);
    fprintf(log,"chase-special-draws: missile=%u searchlight=%u sniper=%u\n",
        gCoOpChaseSpecialDraws[0],gCoOpChaseSpecialDraws[1],gCoOpChaseSpecialDraws[2]);
    fprintf(log,"hud: installed=%d enabled=%d quads=%u dropped=%u panels=%u,%u timers=%u\n",
        (int)gHudInstalled,(int)CoOp_HudEnabled(),gHudLastCount,gHudLastDropped,
        gHudPanelCount[0],gHudPanelCount[1],gHudTimerCount);
    fprintf(log,"partner-markers: P1-view=%u P2-view=%u (0 hidden, 1 label, 2 edge arrow)\n",
        gHudPartnerState[0],gHudPartnerState[1]);
    gCoOpWorldTrace.Write(log);
    const i32* zone=reinterpret_cast<const i32*>(0x005FC640);
    fprintf(log,"zone: flags=%08lx xzBounds=%ld,%ld,%ld,%ld cell=%ld,%ld dims=%d,%d\n",
        zone[0],zone[1],zone[2],zone[3],zone[4],zone[5],zone[6],
        reinterpret_cast<const i16*>(zone)[14],reinterpret_cast<const i16*>(zone)[15]);
    for(int r=0;r<40;++r) {
        const u8* region=reinterpret_cast<const u8*>(0x006B2440)+r*0x44;
        if(region[0]) fprintf(log,"resource %d %.9s super=%u usable=%u items=%08lx models=%08lx psx=%08lx\n",
            r,(const char*)region,region[9],region[10],
            *reinterpret_cast<const u32*>(region+0x10),
            *reinterpret_cast<const u32*>(region+0x14),
            *reinterpret_cast<const u32*>(region+0x18));
    }
    fprintf(log,"contacts: p1flagsE0=%08lx p2flagsE0=%08lx p1state=%ld p2state=%ld\n",
        *reinterpret_cast<const u32*>((const u8*)p1+0xE0),
        *reinterpret_cast<const u32*>((const u8*)p2+0xE0),
        *reinterpret_cast<const i32*>((const u8*)p1+0xE1C),
        *reinterpret_cast<const i32*>((const u8*)p2+0xE1C));
    fflush(log);
}

// Restore retail render state after the second view and HUD are submitted.
static void __cdecl CoOp_RenderCleanup()
{
    CoOp_TraceWorld(4);
    CoOp_HudViewMarkers(1);
    typedef void (__cdecl *CleanupFn)();
    reinterpret_cast<CleanupFn>(0x004737F0)();
    if(gCoOpEffectsCamera) CoOp_DrawWorldEffects(gCoOpEffectsCamera,1);
    if (gCoOpViewActive && !CoOp_SubmitSecondView())
    {
        // Reserve the unused pane with an opaque UI quad. World projection and
        // CPU frustum already use the smaller viewport; no framebuffer is scaled.
        // This mask also contains legacy effects that draw in full-screen space.
        typedef void (__cdecl *TextureFn)(i32, i32);
        typedef void (__cdecl *QuadFn)(float,float,float,float,float,float,float,float,u32,float,bool);
        TextureFn texture = reinterpret_cast<TextureFn>(0x00506440);
        i32 savedTexture = *reinterpret_cast<i32*>(0x00568170);
        i32 savedBlend = *reinterpret_cast<i32*>(0x00AC08DC);
        u8 savedUntextured = *reinterpret_cast<u8*>(0x00AC08D0);
        float width = (float)*reinterpret_cast<i32*>(0x00568154);
        float height = (float)*reinterpret_cast<i32*>(0x00568158);
        float logicalWidth = (float)*reinterpret_cast<u32*>(0x0061B5FC);
        float logicalHeight = (float)*reinterpret_cast<u32*>(0x00628614);
        if (width > 0 && height > 0 && logicalWidth > 0 && logicalHeight > 0)
        {
            bool vertical = gCoOpLayout == COOP_SPLIT_LEFT_RIGHT;
            float x = vertical ? width * gCoOpView.right / logicalWidth : 0;
            float y = vertical ? 0 : height * gCoOpView.bottom / logicalHeight;
            texture(1, 0);
            reinterpret_cast<QuadFn>(0x00507470)(
                x, y, width-x, height-y, 0,0,1,1,0xFF101820,-4.0f,false);
            texture(savedTexture, savedBlend);
            *reinterpret_cast<u8*>(0x00AC08D0) = savedUntextured;
        }
    }
    CoOp_TraceWorld(5);
    // Undo the retained scratch pointer before the following HUD/menu work.
    *reinterpret_cast<void**>(0x0064E514) = gCoOpOriginalViewport;
    CoOp_HudFinish();
    if(!CoOp_HudEnabled()) CoOp_DrawStatus();
    CoOp_RecordViewDiagnostics();
}

// Retire co-op ownership before retail destroys the current level bank.
static void __cdecl CoOp_KillAll()
{
    if(gCoOpPlayers.InSecondaryContext())++gCoOpKillInSecondaryContext;
    gCoOpBossCamera=0;
    gCoOpVenomGrabBoss=0;gCoOpVenomGrabOwner=CoOpPlayerHandle();
    gCoOpCameraTicks[0]=gCoOpCameraTicks[1]=0;
    gCoOpCameraModeChanges[0]=gCoOpCameraModeChanges[1]=0;
    gCoOpCameraLastMode[0]=gCoOpCameraLastMode[1]=-1;
    memset(gCoOpAuthoredCameraScripts,0,sizeof(gCoOpAuthoredCameraScripts));
    gCoOpLastCameraScriptNode[0]=gCoOpLastCameraScriptNode[1]=gCoOpLastCameraScriptNode[2]=-1;
    memset(gCoOpLastCameraScriptOp,0,sizeof(gCoOpLastCameraScriptOp));
    gCoOpP2CameraScriptBank=0;
    gCoOpCraneAutoBank=0;
    if(!gCoOpSession.enabled)gCoOpCraneArrivalPending=gCoOpCraneArrivalRequested=false;
    gCoOpCollisionScripts[0]=gCoOpCollisionScripts[1]=0;
    CoOp_LifeRememberLevelChange();
    gCoOpLife.Clear();memset(gCoOpHasSafeGround,0,sizeof(gCoOpHasSafeGround));
    gCoOpLifeHandles[0]=gCoOpLifeHandles[1]=CoOpPlayerHandle();
    CoOp_MenuDestroyPreview();
    memset(gCoOpCameraSettings,0,sizeof(gCoOpCameraSettings));
    gCoOpCameraPolicy=CoOpCameraPolicy();
    gCoOpSession.LevelEnded();
    if(gCoOpSession.enabled)
        {gCoOpPlayers.CaptureCarry(gCoOpSession.carry,*reinterpret_cast<i32*>(0x0060CFA4));
        gCoOpSession.primaryCarry.Capture(gCoOpPlayers.Get(COOP_PLAYER_ONE));}
    // Release our independent body before retail frees level model resources.
    gCoOpWorldTrace.Clear();
    gCoOpSections.Clear();gCoOpSectionsActive=false;gCoOpSectionResource=0;
    gCoOpRaceSectionsActive=gCoOpSewerSectionsActive=false;
    gCoOpSectionProfile=COOP_SECTION_NONE;
    gCoOpNodeOwners.Clear();gCoOpDeferredOwners.Clear();gCoOpCinematicOwner=CoOpPlayerHandle();
    gCoOpScriptMotion.Clear();
    gCoOpP2CameraScriptBank=0;
    gCoOpTriggerOwner=gCoOpCommandOwners=0;
    gCoOpGrab[0]=gCoOpGrab[1]=CoOpGrabLease();
    gCoOpEnvironment.Clear(true); gCoOpEnvironmentHead=0;
    gCoOpPlayers.Clear();
    gCoOpCameraOwner=CoOpPlayerHandle();
    gCoOpViewActive = false;
    typedef void (__cdecl *KillFn)();
    reinterpret_cast<KillFn>(0x004437E0)();
}
#endif
#include "runtime_version.h"

#ifndef RUNTIME_VERSION
#define RUNTIME_VERSION "LOCAL"

#endif

#ifndef _WIN32

int main()
{
	compile_time_assertions();
	return run_assertions();
}


#else

HMODULE bink_dll;

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved ) 
{
	compile_time_assertions();
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
        {
			if(GetModuleHandle("tobey_validator.exe") != NULL)
			{
				puts("In validator");
				break;
			}

			AllocConsole();
			SetConsoleTitle("spidey-decomp - " RUNTIME_VERSION);
			freopen("CONOUT$", "w", stdout);

			bink_dll = GetModuleHandleA("binkw32.dll");


			puts("spidey-decomp starting " RUNTIME_VERSION);

			runtime_assertions();
			runtime_patches();


        }

            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
#endif

// @Bogus
void DoAssert(u8 cond, const char* str, ...)
{
	if (!cond)
		puts(str);
}
