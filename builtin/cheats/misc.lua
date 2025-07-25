local heal_cooldown = 0
-- AutoHeal
core.register_globalstep(function(dtime)
	if core.localplayer then
		if heal_cooldown > 0 then
			heal_cooldown = heal_cooldown - dtime
		end

		if heal_cooldown <= 0 and core.localplayer:get_hp() < tonumber(core.settings:get("auto_heal.hp")) and core.settings:get_bool("auto_heal") then
			local current_wield_index = core.localplayer:get_wield_index() + 1
			local food_index = nil

			for index, stack in ipairs(core.get_inventory("current_player").main) do
				if stack and stack:get_name() ~= "" then
					for group_name, _ in pairs(core.get_item_def(stack:get_name()).groups) do
						if group_name:sub(1, 5) == "food_" then
							food_index = index
							break
						end
					end
				end
				if food_index then break end
			end

			if food_index then
				core.localplayer:set_wield_index(food_index)
				heal_cooldown = tonumber(core.settings:get("auto_heal.cooldown"))

				core.after(tonumber(core.settings:get("auto_heal.delay")), function()
					core.interact("use", {type="nothing"})
					core.localplayer:set_wield_index(current_wield_index)
				end)
			end
		end
	end
end)

core.register_cheat("Auto Heal", "Misc", "auto_heal")
core.register_cheat_description("Auto Heal", "Misc", "auto_heal", "Automatically eat food if health is below a set value.")

core.register_cheat_setting("Delay", "Misc", "auto_heal", "auto_heal.delay", {type="slider_float", min=0.0, max=1.5, steps=16})
core.register_cheat_setting("Cooldown", "Misc", "auto_heal", "auto_heal.cooldown", {type="slider_float", min=0.0, max=1.5, steps=16})
core.register_cheat_setting("HP", "Misc", "auto_heal", "auto_heal.hp", {type="slider_int", min=1, max=30, steps=30})

-- AppleAura
local function destroy_apple(pos)
    minetest.dig_node(pos)
end

minetest.register_globalstep(function()
    local player = minetest.localplayer
    if not player then
        return
    end

    if minetest.settings:get_bool("appleaura") then
        local player_pos = player:get_pos()
        local apple_nodes = minetest.find_nodes_near(player_pos, tonumber(minetest.settings:get("appleaura.range")), {"default:apple"}) -- Find apples within specified range

        if apple_nodes then
            for _, apple_pos in ipairs(apple_nodes) do
                destroy_apple(apple_pos)
            end
        end
    end
end)
  

minetest.register_cheat_with_infotext("AppleAura", "Misc", "appleaura", "")
minetest.register_cheat_setting("Radius", "Misc", "appleaura", "appleaura.range", {type="slider_float", min=1, max=6, steps=6})
core.register_cheat_description("AppleAura", "Misc", "appleaura", "Automatically digs all apples within a specific radius.")

local spam_active = false

local function spam()
    if minetest.settings:get_bool("spammer") then
        minetest.send_chat_message(minetest.settings:get("spammer.message"))
        minetest.after(tonumber(minetest.settings:get("spammer.cooldown") or 5), spam)
    else
        spam_active = false -- stop spam loop when disabled
    end
end

minetest.register_globalstep(function()
    if minetest.settings:get_bool("spammer") and not spam_active then
        spam_active = true
        spam()
    end
end)




minetest.register_cheat("Spammer", "Misc", "spammer")
core.register_cheat_setting("Cooldown", "Misc", "spammer", "spammer.cooldown", {type="slider_int", min=1, max=50, steps=50})
core.register_cheat_setting("Text", "Misc", "spammer", "spammer.message", {type="text", size=10})


--could look better but it works
local message_sent_combat_target_hud = false
local message_sent_coords

minetest.register_globalstep(function()
    if minetest.settings:get_bool("enable_combat_target_hud") and minetest.settings:get_bool("hud_elements_advice") then
        if not message_sent_combat_target_hud then
            local message = minetest.colorize("#3250af", "[Advice]: To modify this HUD element's (and some others) position and size, open the Click GUI (F8 by default), press 'Edit HUD' button and then you can modify them. You can hide this message with the command .hide_hud_elements_advice")
            ws.dcm(message)
            message_sent_combat_target_hud = true
        end
    else
		message_sent_combat_target_hud = false
	end
end)

minetest.register_globalstep(function()
    if minetest.settings:get_bool("coords") and minetest.settings:get_bool("hud_elements_advice") then
        if not message_sent_coords then
            local message = minetest.colorize("#3250af", "[Advice]: To modify this HUD element's (and some others) position and size, open the Click GUI (F8 by default), press 'Edit HUD' button and then you can modify them. You can hide this message with the command .hide_hud_elements_advice")
            ws.dcm(message)
            message_sent_coords = true
        end
    else
		message_sent_coords = false
	end
end)

minetest.register_chatcommand("hide_hud_elements_advice", {
    func = function()
        core.settings:set_bool("hud_elements_advice", false)
    end,
})

-- I don't know why you'd wanna use this
minetest.register_chatcommand("show_hud_elements_advice", {
    func = function()
        core.settings:set_bool("hud_elements_advice", true)
    end,
})
