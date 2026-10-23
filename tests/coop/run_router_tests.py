"""Run behavioral tests against production headers, with fake retail objects/calls.
No game process is started. Generated binaries and headers live in a temp directory.
"""
from pathlib import Path
from source_paths import header_path
import os
import re
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[2]
tests = Path(__file__).resolve().parent
toolchain = Path(os.environ.get("MSVCDir", r"C:\vs"))
env = os.environ.copy()
env["INCLUDE"] = str(toolchain / "INCLUDE")
env["LIB"] = str(toolchain / "LIB")
with tempfile.TemporaryDirectory(prefix="spidey-router-tests-") as work:
    work = Path(work)
    names = ("router_test", "registry_test", "script_motion_test", "context_test", "input_test", "gamepad_test", "controller_route_test",
             "activation_test", "environment_test", "world_trace_test", "sections_test", "hud_test", "view_activation_test", "interaction_test", "camera_layout_test", "boss_camera_test", "config_test", "effects_test", "menu_test", "menu_preview_test", "camera_tuning_test", "spawn_test", "life_test", "waiting_hud_test")
    names += ("compass_test", "race_hud_test", "partner_marker_test", "capture_pixels_test", "crane_fallback_test", "entry_state_test", "script_handoff_test")
    names += ("carry_test", "grab_test", "section_graph_test", "traversal_exit_test")
    names += ("armor_test", "rhino_shock_test", "camera_policy_test", "projectile_collision_test")
    for name in ("router_test_types.h",) + tuple(n + ".cpp" for n in names):
        (work / name).write_bytes((tests / name).read_bytes())
    (work / "ps2pad.h").write_text((repo / "ps2pad.h").read_text().replace(
        '#include "export.h"', '#include "router_test_types.h"\n#define EXPORT\n#define PADDING(n) unsigned char pad[n]'))
    for name in ("coop_router.h", "coop_players.h", "coop_retail_player.h", "coop_entry_state.h", "coop_costume.h",
                 "coop_context.h", "coop_script_motion.h", "coop_input.h", "coop_gamepad_runtime.h", "coop_controller_route.h", "coop_activation.h",
                 "coop_environment.h", "coop_world_trace.h", "coop_sections.h", "coop_hud.h", "coop_partner_marker.h", "coop_view.h", "coop_interactions.h", "coop_gamepad.h", "coop_config.h", "coop_effects.h", "coop_session.h", "coop_spawn.h", "coop_aim.h", "coop_cinematic_tick.h", "coop_life.h", "coop_fall_triggers.h", "coop_traversal_exit.h", "coop_capture_pixels.h", "coop_crane_fallback.h"):
        source = header_path(repo, name).read_text().replace(
            '#include "spidey.h"', '#include "router_test_types.h"').replace(
            '#include "camera.h"', '#include "router_test_types.h"')
        if name == "coop_spawn.h":
            for address, fn in {"0x004E6840":"SpawnFloor","0x004E67A0":"SpawnClear","0x004E60C0":"SpawnDirection"}.items():
                source=source.replace(address,"reinterpret_cast<u32>(&"+fn+")")
            source=source[:source.index("static bool CoOp_SpawnBlocked(")]+"static bool CoOp_SpawnBlocked(bool=false){return SpawnBlocked();}\n"
        if name == "coop_costume.h":
            source=source.replace("0x006828D8", "Address(0x006828D8)")
        if name == "coop_players.h":
            source=source.replace("GetTickCount()","SpawnNow()")
            source = source.replace("0x006A9038", "Address(0x006A9038)").replace(
                "0x004739A0", "reinterpret_cast<u32>(&Render)")
        if name == "coop_entry_state.h":
            for address,target in {"0x00455390":"Allocate", "0x004C9130":"EntryAnim",
                "0x004F6F00":"EntrySwing", "0x00458210":"EntryFree", "0x00416AA0":"Start"}.items():
                source=source.replace(address,"reinterpret_cast<u32>(&::"+target+")")
            source=source.replace("0x006B4CA8","Address(0x006B4CA8)")
        if name == "coop_retail_player.h":
            source = source.replace("reinterpret_cast<short*>(presetAddresses[n])", "reinterpret_cast<short*>(Address(presetAddresses[n]))")
            for address, target in {
                "0x00455390": "Allocate", "0x004B9EB0": "Construct", "0x004CAC20":"LoadSuit",
                "0x004BAEC0": "SetArmor", "0x00415EE0": "CameraConstruct", "0x00416AA0": "Start",
                "0x004B9E50": "Orient", "0x004BE4B0": "Stand",
                "0x00416A20": "Load", "0x00479540": "Button",
                "0x00460F90": "Update", "0x004C9210": "Destroy",
            }.items():
                source = source.replace(address, "reinterpret_cast<u32>(&::" + target + ")")
            # Keep numeric descriptor/range addresses unchanged; translate only
            # dereferences at the boundary so production bank loops still run.
            source = re.sub(r"(reinterpret_cast<[^;\n]+?>\()(0x00[0-9A-F]{6})(\))",
                            r"\1Address(\2)\3", source)
            source = source.replace("reinterpret_cast<u32**>(a)", "reinterpret_cast<u32**>(Address(a))")
            source = source.replace("reinterpret_cast<void*>(spans[i][0])",
                                    "reinterpret_cast<void*>(Address(spans[i][0]))")
        target = work / header_path(repo, name).relative_to(repo)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(source)
    from bank_fixture import generate as generate_bank
    generate_bank(repo,work)
    from fall_fixture import generate_one
    main = (repo / "main.cpp").read_text()
    (work / "section_graph_fixture.h").write_text(generate_one(repo,work,b"l5a6_t.trg","Plant")+"\n"+generate_one(repo,work,b"l3a3_t.trg","Crane"))
    graph=main[main.index("static bool CoOp_ReadFallNode("):main.index("static bool CoOp_VenomRaceBank();")]+main[main.index("static unsigned CoOp_ProjectSectionGraph("):main.index("static unsigned gCoOpVenomRaceFailureChecks")]
    for a,b in {"*reinterpret_cast<int*>(0x006B4670)":"::nodeCount","*reinterpret_cast<u16***>(0x006B466C)":"::nodes","*reinterpret_cast<SCommandPoint**>(0x006B4708)":"::pointList"}.items():graph=graph.replace(a,b)
    (work / "section_graph_runtime_test.h").write_text(graph)
    from crane_fixture import generate as generate_crane
    generate_crane(repo,work)
    spawn_source=(header_path(repo, "coop_spawn.h")).read_text()
    spawn_gate=spawn_source[spawn_source.index("static bool CoOp_SpawnBlocked("):]
    spawn_gate=spawn_gate.replace("CoOp_SpawnBlocked(","CoOp_ActualSpawnBlocked(")
    for expression,replacement in {
        "*reinterpret_cast<CPlayer**>(0x006A9038)":"spawnPrimary",
        "*reinterpret_cast<void**>(0x0056F3B8)":"spawnCamera",
        "*reinterpret_cast<i32*>(0x00ADB4D8)":"paused",
        "*reinterpret_cast<i32*>(0x00660F80)":"letterbox"}.items():
        spawn_gate=spawn_gate.replace(expression,replacement)
    (work / "spawn_gate_runtime_test.h").write_text(spawn_gate)
    # Compile the actual runtime section synchronization, only replacing its
    # external resource-table address. This catches presentation/lifetime coupling.
    main = (repo / "main.cpp").read_text()
    armor=(header_path(repo, "coop_armor_runtime.h")).read_text()
    armor=armor[:armor.index("static void __declspec(naked)")]+"\n#endif\n"
    for address,variable in {"0x006A9038":"&mech","0x006B78F8":"&lowGraphics","0x0054AE18":"&armorRegion"}.items():
        armor=armor.replace(address,variable)
    (work / "armor_runtime_test.h").write_text(armor)
    shock=(header_path(repo, "coop_rhino_runtime.h")).read_text()
    shock=shock[:shock.index("static bool CoOp_InstallRhinoShockHook()")]+"\n#endif\n"
    shock=shock.replace("0x00460C10","reinterpret_cast<u32>(&NativeRun)")
    (work / "rhino_shock_runtime_test.h").write_text(shock)
    mapper_start=main.index("static void __cdecl CoOpDebug_GetMappedStates(u32* mappedButtons")
    mapper=main[mapper_start:main.index("// Native P1 AI retains its own gameplay callback.",mapper_start)]
    assert "RequestSecondarySpawn" not in mapper and "RequestSecondaryRemoval" not in mapper
    assert "getKeyState(0x09)" not in mapper and "getKeyState(0x0A)" not in mapper
    assert "getKeyState(0xCF)" in mapper and "getKeyState(0x33)" in mapper
    assert "getKeyState(0x0B)" not in mapper and "getKeyState(0x07)" not in mapper

    start=main.index("static SCommandPoint* __cdecl CoOp_SectionTrigger(u32 checksum,bool assertMissing)\n{")
    end=main.index("static unsigned CoOp_PendingCount()",start)
    trigger=main[start:end].replace("reinterpret_cast<SCommandPoint**>(0x006B4214)","hashTable")
    trigger=trigger.replace("*reinterpret_cast<u16***>(0x006B466C)","nodeTable")
    trigger=trigger.replace("*reinterpret_cast<int*>(0x006B4670)","::nodeCount")
    trigger=trigger.replace("0x004DE8D0","reinterpret_cast<unsigned>(&NativeTrigger)")
    trigger=trigger.replace("reinterpret_cast<PendingListEntry*>(0x006B4688)","::queue")
    (work / "section_trigger_test.h").write_text(trigger)
    start = main.index("static void CoOp_SyncSections()\n{")
    end = main.index('#include "coop/world/coop_fall_triggers.h"', start)
    sync = main[start:end].replace("reinterpret_cast<const u8*>(0x006B2440)", "resources")
    (work / "runtime_section_sync.h").write_text(sync)
    policy=(header_path(repo, "coop_camera_policy_runtime.h")).read_text()
    policy=policy[:policy.index("static bool CoOp_InstallCameraPolicyHooks()")]+"\n#endif\n"
    policy=policy.replace("0x004167F0","reinterpret_cast<u32>(&NativeMode)")
    policy=policy.replace("0x004B9E10","reinterpret_cast<u32>(&NativeLock)")
    policy=policy.replace("*reinterpret_cast<CCamera**>(0x0056F3B8)","currentCamera")
    policy=policy.replace("*reinterpret_cast<CPlayer**>(0x006A9038)","currentPlayer")
    (work / "camera_policy_runtime_test.h").write_text(policy)
    projectile=(header_path(repo, "coop_projectile_collision_runtime.h")).read_text()
    projectile=projectile[:projectile.index("static bool CoOp_InstallProjectileCollisionHooks()")]+"\n#endif\n"
    projectile=projectile.replace("0x00452C30","reinterpret_cast<u32>(&Sphere)")
    projectile=projectile.replace("0x005FBD38","&projectionScratch").replace("0x005FBDE0","&bodyScratch").replace("0x00610B20","mathScratch")
    (work / "projectile_collision_runtime_test.h").write_text(projectile)
    grab = (header_path(repo, "coop_interaction_runtime.h")).read_text()
    grab = grab[grab.index("struct CoOpGrabLease"):grab.index("static bool CoOp_EnemyAlive(")]+grab[grab.index("struct CoOpEnemyCall"):grab.index("static void CoOp_EnemyAI(")]
    for address,fn in {"0x004583F0":"Resolve","0x00458360":"Handle","0x004C9130":"Release","0x00408930":"EndWrap"}.items():
        grab=grab.replace(address,"reinterpret_cast<u32>(&"+fn+")")
    grab=grab.replace("*reinterpret_cast<CPlayer**>(0x006A9038)","mech")
    (work / "grab_runtime_test.h").write_text(grab)
    interaction = (header_path(repo, "coop_interaction_runtime.h")).read_text()
    interaction = interaction[:interaction.index("static bool CoOp_InstallCameraUpdateHook()")]+"\n#endif\n"
    for address, function in {"0x004BAEC0":"SetArmor", "0x00460700":"IsDead", "0x004E67A0":"LineOfSight",
        "0x004583F0":"ResolveEnemy", "0x004C9130":"ReleasePlayer", "0x00488590":"NativeScorpion", "0x00458360":"MakeHandle",
        "0x00451350":"NativeLizMan", "0x004DB280":"NativeEnemy", "0x0042FA10":"NativeEnemy", "0x004819C0":"NativeEnemy", "0x004EC040":"NativeEnemy", "0x004262A0":"NativeChopper",
        "0x00421900":"NativeChaseBody", "0x00422EF0":"NativeChaseBody", "0x00425400":"NativeChaseBody",
        "0x00421100":"NativeBullet",
        "0x004E7E10":"NativeProgress", "0x004477F0":"NativeProgress",
        "0x004489B0":"NativeProgress", "0x004D0E70":"NativeProgress",
        "0x004E7D30":"NativeRelocInit", "0x00447640":"NativeRelocInit",
        "0x00448730":"NativeRelocInit", "0x004CBD70":"NativeRelocInit",
        "0x0046B860":"NativePickup"}.items():
        interaction = interaction.replace(address, "reinterpret_cast<u32>(&"+function+")")
    interaction = interaction.replace("*reinterpret_cast<CPlayer**>(0x006A9038)","mech")
    interaction = interaction.replace("reinterpret_cast<PendingListEntry*>(0x006B4688)","::queue")
    (work / "interaction_runtime_test.h").write_text(interaction)
    start = main.index("struct CoOpScriptCall {")
    end = main.index("struct CoOpScriptPlacement",start)
    dispatch = main[start:end].replace("0x004E0210", "reinterpret_cast<u32>(&NativeScript)")
    dispatch = re.sub(r"static bool CoOp_SecondaryInteractiveScene\(\) \{.*?\n\}",
                      "static bool CoOp_SecondaryInteractiveScene(){return interactiveScene;}",
                      dispatch, flags=re.S)
    dispatch = re.sub(r"static bool CoOp_DeferTrailingRaceFailure\(.*?\n\}",
                      "static bool CoOp_DeferTrailingRaceFailure(const u16*,i32,CoOpPlayerHandle){return deferRaceFail;}",
                      dispatch, flags=re.S)
    dispatch = dispatch.replace("*reinterpret_cast<void**>(0x006B466C)", "scriptBank")
    dispatch = dispatch.replace("reinterpret_cast<const u8*>(0x006B2440)", "craneResources")
    dispatch = dispatch.replace("*reinterpret_cast<u16***>(0x006B466C)", "craneNodes")
    dispatch = dispatch.replace("*reinterpret_cast<int*>(0x006B4670)", "craneNodeCount")
    dispatch = dispatch.replace("*reinterpret_cast<SCommandPoint**>(0x006B4708)", "cranePoints")
    dispatch = dispatch.replace("reinterpret_cast<PendingListEntry*>(0x006B4688)","::queue")
    dispatch = dispatch.replace("*reinterpret_cast<i32*>(0x00660F80)", "letterbox")
    dispatch = dispatch.replace("*reinterpret_cast<i32*>(0x0060F76C)", "hudOffset")
    dispatch = dispatch.replace("*reinterpret_cast<u8*>(0x0060CFC4)", "nativeSkipFlag")
    (work / "script_dispatch_test.h").write_text(dispatch)
    handoff_start=main.index("struct CoOpScriptPlacement")
    handoff=main[handoff_start:main.index("static void __cdecl CoOp_SectionPending()",handoff_start)]
    handoff=handoff.replace("*reinterpret_cast<void**>(0x006B466C)","scriptBank")
    handoff=handoff.replace("*reinterpret_cast<i32*>(0x00660F80)","letterbox")
    handoff=handoff.replace("*reinterpret_cast<i32*>(0x0060F76C)","hudOffset")
    handoff=handoff.replace("*reinterpret_cast<CCamera**>(0x0056F3B8)","primaryCamera")
    handoff=handoff.replace("reinterpret_cast<void*>(0x00610B20)","mathScratch")
    (work / "script_handoff_runtime_test.h").write_text(handoff)
    aim_start=main.index("static void CoOp_EndAimPresentation(")
    aim=main[aim_start:main.index("// Native P1 AI retains its own gameplay callback.",aim_start)]
    aim=aim.replace("*reinterpret_cast<i32*>(0x00660F80)","::letterbox").replace("*reinterpret_cast<i32*>(0x0060F76C)","::hudOffset")
    (work / "aim_runtime_test.h").write_text(aim)

    start = main.index("static void CoOp_CenterNarrowCamera(")
    end = main.index("static void __cdecl CoOp_RenderSetup(void* camera",start)
    center = main[start:end].replace("0x004E67A0", "reinterpret_cast<u32>(&ClearRay)")
    (work / "camera_layout_runtime.h").write_text(center)
    start = main.index("static CCamera* gCoOpBossCamera;")
    end = main.index("static void __cdecl CoOp_UpdatePlayers(void* list,i32 flags) {",start)
    boss_camera = main[start:end].replace("*reinterpret_cast<CCamera**>(0x0056F3B8)","retailCamera")
    boss_camera = boss_camera.replace("*reinterpret_cast<CItem**>(0x0056E990)","baddies")
    (work / "boss_camera_runtime_test.h").write_text(boss_camera)
    config = (header_path(repo, "coop_config_runtime.h")).read_text()
    config = config[:config.index("static bool CoOp_InstallConfigHooks()")]
    for address,function in {"0x0043FFF0":"Add","0x00440600":"Process","0x00501CB0":"Key",
        "0x0050C180":"Trigger","0x0050C6C0":"Mouse","0x0043FE60":"Zoomed","0x0048D9C0":"Title"}.items():
        config=config.replace(address,"reinterpret_cast<u32>(&"+function+")")
    (work / "config_runtime_test.h").write_text(config)
    effects=(header_path(repo, "coop_effects_runtime.h")).read_text()
    effects=effects[:effects.index("static bool CoOp_InstallEffectsHooks()")]
    for address,function in {"0x00411CF0":"Original","0x0046D7B0":"SetRot","0x0046E460":"Zero"}.items():
        effects=effects.replace(address,"reinterpret_cast<u32>(&"+function+")")
    effects=re.sub(r"reinterpret_cast<([^>]+)>\((0x[0-9A-Fa-f]+)\)",r"reinterpret_cast<\1>(Address(\2))",effects)
    effects=effects.replace("reinterpret_cast<u32>(slots[i].list)","Retail(reinterpret_cast<u32>(slots[i].list))")
    effects=effects.replace("slots[i].draw(slots[i].list)","DrawSlot(slots[i].draw,slots[i].list)")
    (work / "effects_runtime_test.h").write_text(effects)
    menu=(header_path(repo, "coop_menu_runtime.h")).read_text()
    menu=menu[:menu.index('#include "coop_menu_preview.h"')]
    start=menu.index("static void __declspec(naked) CoOp_MainNavigationBridge()")
    end=menu.index("static u8 __cdecl CoOp_MainTrigger",start)
    menu=menu[:start]+menu[end:]
    for address,function in {"0x0050C180":"Trigger","0x00471C50":"Sound","0x00458700":"Text",
        "0x0050C470":"Cursor","0x00493990":"NativeMain"}.items():
        menu=menu.replace(address,"reinterpret_cast<u32>(&"+function+")")
    menu=re.sub(r"reinterpret_cast<([^>]+)>\((0x[0-9A-Fa-f]+)\)",r"reinterpret_cast<\1>(Address(\2))",menu)
    (work / "menu_runtime_test.h").write_text(menu)
    start=main.index("static void __cdecl CoOp_RenderPlayers(void* list)")
    end=main.index("static void CoOp_CenterNarrowCamera",start)
    render=main[start:end].replace("0x004739A0","reinterpret_cast<u32>(&Render)")
    render=render.replace("CoOp_ScriptMotionFrame(true);", "")
    render=render.replace("CPlayer* primary=gCoOpPlayers.Get(COOP_PLAYER_ONE);", "bool primary=gCoOpPlayers.Get(COOP_PLAYER_ONE);")
    render=render.replace("u16 savedFlags=primary?primary->mFlags:0;", "u16 savedFlags=0;")
    render=render.replace("if(primary && gCoOpScriptMotion.Partner(COOP_PLAYER_ONE))primary->mFlags|=1;", "")
    render=render.replace("if(primary)primary->mFlags=savedFlags;", "")
    render=render.replace("gCoOpPlayers.RenderSecondary(gCoOpScriptMotion.Partner(COOP_PLAYER_TWO));", "gCoOpPlayers.RenderSecondary();")
    (work / "session_render_test.h").write_text(render)
    preview=(header_path(repo, "coop_menu_preview.h")).read_text()
    preview=preview.replace("0x005559DC","reinterpret_cast<u32>(&currentSuit)")
    for address,function in {"0x00490DF0":"Construct","0x00455390":"Allocate","0x00460C10":"RunAnim",
        "0x004739A0":"Draw","0x00411B30":"Effects"}.items():
        preview=preview.replace(address,"reinterpret_cast<u32>(&"+function+")")
    (work / "menu_preview_runtime_test.h").write_text(preview)
    tuning=(header_path(repo, "coop_camera_tuning.h")).read_text()
    tuning=tuning[:tuning.index("static bool CoOp_InstallCameraTuningHook()")]
    tuning=tuning.replace("0x004B97D0","reinterpret_cast<u32>(&SetCamera)")
    tuning=tuning.replace("*reinterpret_cast<CPlayer**>(0x006A9038)","current")
    (work / "camera_tuning_runtime_test.h").write_text(tuning)
    from fall_fixture import generate
    generate(repo,work)
    fall=main[main.index("static bool CoOp_ReadFallNode("):main.index("static bool CoOp_VenomRaceBank();")]+main[main.index("static bool CoOp_InterceptFallTrigger("):main.index("static unsigned CoOp_ProjectSectionGraph(")]
    for expression,replacement in {"*reinterpret_cast<int*>(0x006B4670)":"::nodeCount", "*reinterpret_cast<u16***>(0x006B466C)":"::nodes", "*reinterpret_cast<SCommandPoint**>(0x006B4708)":"::pointList", "reinterpret_cast<SCommandPoint**>(0x006B4214)":"::hashTable"}.items():
        fall=fall.replace(expression,replacement)
    (work / "fall_trigger_runtime_test.h").write_text(fall)
    life=(header_path(repo, "coop_life_runtime.h")).read_text()
    life=life[:life.index("static void __declspec(naked) CoOp_DeathEndBridge()")]
    for address,function in {"0x004BAEC0":"SetArmor", "0x00460700":"IsDead","0x004BDFF0":"DeathMode","0x004E6840":"Floor",
        "0x004B9E50":"Orient","0x004BE4B0":"Stand","0x004BF720":"FloorCamera","0x00416AA0":"CameraStart"}.items():
        life=life.replace(address,"reinterpret_cast<u32>(&"+function+")")
    life=life.replace("reinterpret_cast<void*>(0x00610B20)","::mathScratch")
    life=life.replace("*reinterpret_cast<int*>(0x0055804C)","::checkpoint")
    life=life.replace("*reinterpret_cast<i32*>(0x00ADB4D8)","::paused")
    life=life.replace("*reinterpret_cast<i32*>(0x00660F80)","letterbox")
    life=life.replace("*reinterpret_cast<i32*>(0x0060F76C)","::hudOffset")
    life=life.replace("*reinterpret_cast<CCamera**>(0x0056F3B8)","currentCamera")
    (work / "life_runtime_test.h").write_text(life)
    hud=(header_path(repo, "coop_hud_runtime.h")).read_text()
    hud=hud[hud.index("static void CoOp_HudWaitingPane("):hud.index("static D3DVIEWPORT7 CoOp_HudViewport(")]
    for address,variable in {"0x0060D238":"&::font", "0x005FAD5C":"::fonts", "0x0060D5A4":"textState", "0x0060D5A6":"textState+2", "0x0060D5A8":"textState+4", "0x0061B5FC":"&logicalWidth", "0x00628614":"&logicalHeight", "0x00458700":"&Draw"}.items():
        hud=hud.replace(address,variable)
    (work / "waiting_hud_runtime_test.h").write_text(hud)
    compass=(header_path(repo, "coop_hud_runtime.h")).read_text()
    compass=compass[compass.index("static void __cdecl CoOp_HudCompass()"):compass.index("static void __cdecl CoOp_HudPanel()")]
    compass=compass.replace("0x00463860","reinterpret_cast<u32>(&NativeCompass)")
    compass=compass.replace("reinterpret_cast<u8*>(0x0060F77C)","&compassStatus")
    (work / "compass_runtime_test.h").write_text(compass)
    race=(header_path(repo, "coop_hud_runtime.h")).read_text()
    race=race[race.index("static void CoOp_HudRaceProgress("):race.index("static void __cdecl CoOp_HudBoss()")]
    (work / "race_hud_runtime_test.h").write_text(race)
    # Extracted runtime fragments live at the temp root, unlike production headers.
    # Convert their relative co-op includes to temp-root paths before compiling.
    for fragment in work.glob("*.h"):
        data = fragment.read_text()
        data = re.sub(r'#include "(?:\.\./[a-z]+/)?(coop_[\w]+\.h)"',
            lambda match: '#include "' + header_path(repo, match.group(1)).relative_to(repo).as_posix() + '"', data)
        fragment.write_text(data)
    for name in names:
        subprocess.run([str(toolchain / "BIN" / "cl.exe"), "/nologo",
                        "/I", ".", name + ".cpp", "/link", "kernel32.lib", "user32.lib"],
                       cwd=work, env=env, check=True)
        subprocess.run([str(work / (name + ".exe"))], cwd=work, check=True)
