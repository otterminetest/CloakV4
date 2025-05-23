local heal_cooldown = 0

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