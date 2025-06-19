core.cheats = {
	["Render"] = {
		["CheatHUD"] = "cheat_hud",
		["FullBright"] = "fullbright",
		["BrightNight"] = "no_night",
		["Xray"] = "xray",
		["EntityESP"] = "enable_entity_esp",
		["EntityTracers"] = "enable_entity_tracers",
		["PlayerESP"] = "enable_player_esp",
		["PlayerTracers"] = "enable_player_tracers",
		["NodeESP"] = "enable_node_esp",
		["NodeTracers"] = "enable_node_tracers",
	--	["HUDBypass"] = "hud_flags_bypass", dont wanna work, will fix later
		["NoHurtCam"] = "no_hurt_cam",
		["NoParticles"] = "norender.particles",
		["TaskTracers"] = "enable_task_tracers",
		["TaskNodes"] = "enable_task_nodes",
		["DetachedCamera"] = "detached_camera",
    },
	["Player"] = {
		["PrivBypass"] = "priv_bypass",
		["NoFallDamage"] = "prevent_natural_damage",
		["Reach"] = "reach",
		["AutoRespawn"] = "autorespawn",
		["LuaControl"] = "lua_control",
		["NoForceRotate"] = "no_force_rotate",
		["QuickMenu"] = "use_old_menu",
    },
	["Movement"] = {
		["Freecam"] = "freecam",
		["AutoForward"] = "continuous_forward",
		["PitchMove"] = "pitch_move",
		["AutoJump"] = "autojump",
		["Flight"] = "free_move",
		["Noclip"] = "noclip",
		["FastMove"] = "fast_move",
		["Jesus"] = "jesus",
		["NoSlow"] = "no_slow",
		["JetPack"] = "jetpack",
		["AntiSlip"] = "antislip",
		["AirJump"] = "airjump",
		["Spider"] = "spider",
		["AutoSneak"] = "autosneak",
    },
	["Combat"] = {
		["AntiKnockback"] = "antiknockback",
		["AttachmentFloat"] = "float_above_parent",
    },
	["Interact"] = {
		["FastHit"] = "spamclick",
		["AutoHit"] = "autohit",
		["FastPlace"] = "fastplace",
		["AutoPlace"] = "autoplace",
		["AutoDig"] = "autodig",
		["FastDig"] = "fastdig",
		["InstantBreak"] = "instant_break",
		["AutoTool"] = "autotool",
    }
}
-----------------------------------------------------------REGISTER CHEATS-----------------------------------------------------------
function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end
-----------------------------------------------------------CHEAT SETTINGS-----------------------------------------------------------
core.cheat_settings = {}

function core.register_cheat_setting(setting_name, parent_category, parent_setting, setting_id, setting_data)
	 --settingname is the formatted setting name, e.g "Assist Mode"
	 --parent_category is the category of the parent setting, e.g "Combat", 
	 --parent_setting is the cheat this setting is for, e.g "autoaim", 
	 --setting_id is the setting string, e.g "autoaim.mode", 
	 --setting_data is the setting table, e.g 
	 --if its a bool,         {type="bool"}
	 --if its an int slider,  {type="slider_int", min=0, max=10, steps=10}
	 --if its a float slider, {type="slider_float", min=0.0, max=10.0, steps=100}
     --if its a text field,   {type="text", size=10}
	 --if its a selectionbox, {type="selectionbox", options={"lock", "assist"}}
	core.cheat_settings[parent_category] = core.cheat_settings[parent_category] or {}
	core.cheat_settings[parent_category][parent_setting] = core.cheat_settings[parent_category][parent_setting] or {}

	core.cheat_settings[parent_category][parent_setting][setting_id] = {
        name = setting_name,
        type = setting_data.type,
        min = setting_data.min,
        max = setting_data.max,
        steps = setting_data.steps,
        size = setting_data.size,
		options = setting_data.options
    }
end
-----------------------------------------------------------CHEAT INFOTEXTS-----------------------------------------------------------
core.infotexts = {}


function core.register_cheat_with_infotext(cheatname, category, func, infotext)
	core.infotexts[category] = core.infotexts[category] or {}	
	core.infotexts[category][cheatname] = infotext	
	core.register_cheat(cheatname, category, func)	
end
	
function core.update_infotext(cheatname, category, func, infotext)
	core.infotexts[category] = core.infotexts[category] or {}	
	core.infotexts[category][cheatname] = infotext	
	core.update_infotexts()
end
-----------------------------------------------------------CHEAT DESCRIPTIONS-----------------------------------------------------------
core.descriptions = {}

function core.register_cheat_with_description(cheatname, category, func, description)
	core.descriptions[category] = core.descriptions[category] or {}
	core.descriptions[category][cheatname] = description
	core.get_description()
end

function core.register_cheat_description(cheatname, category, func, description)
	core.descriptions[category] = core.descriptions[category] or {}
	core.descriptions[category][cheatname] = description
	core.get_description()
end
-----------------------------------------------------------TESTS, PRESET VALUES, ETC-----------------------------------------------------------
local start_pos = {x=0, y=0, z=0}
local end_pos = {x=5, y=0, z=5}

local last_path = {}
local last_tracers = {}

-- Clear nodes and tracers from previous path
local function clear_old_path()
	for _, pos in ipairs(last_path) do
		core.clear_task_node(pos)
	end
	last_path = {}

	for _, tracer in ipairs(last_tracers) do
		core.clear_task_tracer(tracer[1], tracer[2])
	end
	last_tracers = {}
end

-- Draws path and records it
local function draw_path(path, reached_goal, target_pos)
	clear_old_path()
	last_path = path

	-- Always show start node (blue)
	core.add_task_node(start_pos, {r=0, g=0, b=255})

	if reached_goal then
		core.add_task_node(end_pos, {r=0, g=255, b=0}) -- End node green
	else
		local final = path[#path]
		core.add_task_node(final, {r=255, g=0, b=255}) -- Last reachable node purple
		core.add_task_node(target_pos, {r=0, g=255, b=0}) 
	end

	-- Tracers: green if complete path, purple if partial
	local tracer_color = reached_goal and {r=0, g=255, b=0} or {r=255, g=0, b=255}

	for i = 1, #path - 1 do
		local a = path[i]
		local b = path[i+1]
		core.add_task_tracer(a, b, tracer_color)
		table.insert(last_tracers, {a, b})
	end
end

-- Continuously updates the path
local function refresh_path_loop()
	local path = core.find_path(start_pos, end_pos)

	-- Determine if goal was reached (based on last node)
	local reached_goal = false
	if path and #path > 0 then
		local last = path[#path]
		reached_goal = vector.equals(last, end_pos)
	end

	if path and #path > 0 then
		draw_path(path, reached_goal, end_pos)
	else
		-- Path is completely empty
		clear_old_path()
		core.add_task_node(start_pos, {r=0, g=0, b=255})
		core.add_task_node(end_pos, {r=255, g=0, b=255}) -- Purple end
	end

	core.after(0.5, refresh_path_loop)
end

-- Start the refresh loop
core.after(0.5, refresh_path_loop)
