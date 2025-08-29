core.load_media("custom_character.b3d")
core.load_media("empty.png")
core.load_media("cape.png")
core.load_media("crown_cape.png")

local update_interval = 0.1
local timer = 0
local frame = 0
local frame_count = 10
local original_textures = {}
local original_meshes = {}

core.register_globalstep(function(dtime)
    timer = timer + dtime
    if timer < update_interval then return end
    timer = 0

	local objects = core.get_nearby_objects(100)
	for _, obj in ipairs(objects) do
		if obj:is_player() and is_user_on_cloakv4(obj:get_name()) then

			local cape_id = "crown" -- allow custom capes soon

			local frame_tex = cape_id .. "_cape.png\\^[verticalframe\\:"..frame_count.."\\:"..frame

			local placed_frame = "[combine:160x80:140,48="..frame_tex

			local overlay_texture = "cape.png^"..placed_frame

			local props = obj:get_properties()
			local current_textures = props.textures or {}
			if original_textures[obj:get_name()] == nil then
				original_textures[obj:get_name()] = current_textures[1] or "character.png"
				original_meshes[obj:get_name()] = props.mesh
			end
			obj:set_properties({
				textures = {
					original_textures[obj:get_name()] .. "^" .. overlay_texture,
					current_textures[2] or "empty.png",
					current_textures[3] or "empty.png",
					current_textures[4] or "empty.png",
				},
				mesh = "custom_character.b3d",
			})
		end
	end
    -- Loop through frames
    frame = (frame + 1) % frame_count
end)
