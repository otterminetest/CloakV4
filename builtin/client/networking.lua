local get_http_api = core.get_http_api
core.get_http_api = nil

SESSION_TOKEN_SETTING_NAME = "session_token"

local announce_join_and_leave = core.settings:get_bool("announce_join")
local session_token = core.settings:get(SESSION_TOKEN_SETTING_NAME)

CloakUsers = {}

if session_token == "" or session_token == nil then return end

function is_user_on_cloakv4(joined_name_to_check)
    for _, user in ipairs(CloakUsers) do
        if user.joined_name == joined_name_to_check then
            return true
        end
    end
    return false
end

local function announce_join(username, server_address, server_port)
	core.log("action", username)
	local http = get_http_api()
	if not http then
		return false
	end

	-- Fire-and-forget join request
	local handle = http.fetch_async({
		url = "http://teamacedia.baselinux.net:22222/api/server/join/",
		timeout = 15,
		post_data = core.write_json({
			token = session_token,
			joined_username = username,
			server_address = server_address,
			server_port = server_port
		}),
		extra_headers = {
			"Content-Type: application/json"
		}
	})

	return true
end

local function announce_leave(username, server_address, server_port)
	local http = get_http_api()
	if not http then
		return
	end

	local session_token = core.settings:get(SESSION_TOKEN_SETTING_NAME)
	if not session_token or session_token == "" then
		return
	end

	-- Fire-and-forget leave request
	http.fetch_async({
		url = "http://teamacedia.baselinux.net:22222/api/server/leave/",
		timeout = 15,
		post_data = core.write_json({
			token = session_token,
			joined_username = username,
			server_address = server_address,
			server_port = server_port
		}),
		extra_headers = {
			"Content-Type: application/json"
		}
	})

	-- clear the session token
	core.settings:set(SESSION_TOKEN_SETTING_NAME, "")
end

local function fetch_cloak_users(server_address, server_port)
	local http = get_http_api()
	if not http then
		return
	end

	local handle = http.fetch_async({
		url = "http://teamacedia.baselinux.net:22222/api/server/players/",
		timeout = 15,
		post_data = core.write_json({
			token = session_token,
			server_address = server_address,
			server_port = server_port
		}),
		extra_headers = {
			"Content-Type: application/json"
		}
	})

	local function check_result()
		local result = http.fetch_async_get(handle)
		if not result.completed then
			minetest.after(0.25, check_result)
			return
		end

		if result.succeeded and result.code == 200 then
			local ok, data = pcall(core.parse_json, result.data)
			if ok and data.players then
				-- Clear previous data
				CloakUsers = {}

				-- Save each player in the table
				for _, p in ipairs(data.players) do
					table.insert(CloakUsers, {
						joined_name = p.joined_name,
						username = p.username
					})
				end
			end
		end
	end
	check_result()
end

ws.on_connect(function()
	if announce_join_and_leave then
		local server_info = core.get_server_info()
		local username = core.localplayer:get_name()
		local server_address = server_info.ip
		local server_port = tostring(server_info.port)

		announce_join(username, server_address, server_port)
		
		core.register_on_shutdown(function()
			announce_leave(username, server_address, server_port)
		end)
	end

	local update_interval = 5
	local timer = 4 -- Only wait 1 second the first time

	core.register_globalstep(function(dtime)
		timer = timer + dtime
		if timer < update_interval then return end
		timer = 0

		local server_info = core.get_server_info()
		local server_address = server_info.ip
		local server_port = tostring(server_info.port)
		fetch_cloak_users(server_address, server_port)
	end)
end)
