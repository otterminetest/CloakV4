/*
Minetest
Copyright (C) 2017 Dumbeldor, Vincent Glize <vincent.glize@live.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "l_clientobject.h"
#include "l_localplayer.h"
#include "l_internal.h"
#include "lua_api/l_item.h"
#include "script/common/c_converter.h"
#include "client/localplayer.h"
#include "hud.h"
#include "common/c_content.h"
#include "client/content_cao.h"
#include "client/client.h"
#include "client/game.h"
#include <iostream>

LuaLocalPlayer::LuaLocalPlayer(LocalPlayer *m) : m_localplayer(m)
{
}

void LuaLocalPlayer::create(lua_State *L, LocalPlayer *m)
{
	lua_getglobal(L, "core");
	luaL_checktype(L, -1, LUA_TTABLE);
	int objectstable = lua_gettop(L);
	lua_getfield(L, -1, "localplayer");

	// Duplication check
	if (lua_type(L, -1) == LUA_TUSERDATA) {
		lua_pop(L, 1);
		return;
	}

	LuaLocalPlayer *o = new LuaLocalPlayer(m);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);

	lua_pushvalue(L, lua_gettop(L));
	lua_setfield(L, objectstable, "localplayer");
}

int LuaLocalPlayer::l_get_velocity(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	push_v3f(L, player->getSendSpeed() / BS);
	return 1;
}

int LuaLocalPlayer::l_get_hp(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushinteger(L, player->hp);
	return 1;
}

int LuaLocalPlayer::l_get_yaw(lua_State *L)
{
    lua_pushnumber(L, wrapDegrees_0_360(g_game->cam_view.camera_yaw));
    return 1;
}

int LuaLocalPlayer::l_get_pitch(lua_State *L)
{
    lua_pushnumber(L, -wrapDegrees_180(g_game->cam_view.camera_pitch) );
    return 1;
}

int LuaLocalPlayer::l_get_name(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushstring(L, player->getName().c_str());
	return 1;
}

// get_wield_index(self)
int LuaLocalPlayer::l_get_wield_index(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushinteger(L, player->getWieldIndex());
	return 1;
}

// get_wielded_item(self)
int LuaLocalPlayer::l_get_wielded_item(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	ItemStack selected_item;
	player->getWieldedItem(&selected_item, nullptr);
	LuaItemStack::create(L, selected_item);
	return 1;
}

// get item range of currently wielded item
// get_wielded_item_range(self)
int LuaLocalPlayer::l_get_wielded_item_range(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	ItemStack selected_item, hand_item;
	player->getWieldedItem(&selected_item, &hand_item);
	f32 d = getToolRange(selected_item, hand_item, g_game->itemdef_manager);

	if (g_settings->getBool("reach"))
		d += 2;

	lua_pushnumber(L, d);

	return 1;
}

int LuaLocalPlayer::l_is_attached(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->getParent() != nullptr);
	return 1;
}

int LuaLocalPlayer::l_is_touching_ground(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->touching_ground);
	return 1;
}

int LuaLocalPlayer::l_is_in_liquid(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->in_liquid);
	return 1;
}

int LuaLocalPlayer::l_is_in_liquid_stable(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->in_liquid_stable);
	return 1;
}

int LuaLocalPlayer::l_get_move_resistance(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushinteger(L, player->move_resistance);
	return 1;
}

int LuaLocalPlayer::l_is_climbing(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->is_climbing);
	return 1;
}

int LuaLocalPlayer::l_swimming_vertical(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushboolean(L, player->swimming_vertical);
	return 1;
}

// get_physics_override(self)
int LuaLocalPlayer::l_get_physics_override(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	const auto &phys = player->physics_override;
	lua_newtable(L);
	lua_pushnumber(L, phys.speed);
	lua_setfield(L, -2, "speed");

	lua_pushnumber(L, phys.jump);
	lua_setfield(L, -2, "jump");

	lua_pushnumber(L, phys.gravity);
	lua_setfield(L, -2, "gravity");

	lua_pushboolean(L, phys.sneak);
	lua_setfield(L, -2, "sneak");

	lua_pushboolean(L, phys.sneak_glitch);
	lua_setfield(L, -2, "sneak_glitch");

	lua_pushboolean(L, phys.new_move);
	lua_setfield(L, -2, "new_move");

	lua_pushnumber(L, phys.speed_climb);
	lua_setfield(L, -2, "speed_climb");

	lua_pushnumber(L, phys.speed_crouch);
	lua_setfield(L, -2, "speed_crouch");

	lua_pushnumber(L, phys.liquid_fluidity);
	lua_setfield(L, -2, "liquid_fluidity");

	lua_pushnumber(L, phys.liquid_fluidity_smooth);
	lua_setfield(L, -2, "liquid_fluidity_smooth");

	lua_pushnumber(L, phys.liquid_sink);
	lua_setfield(L, -2, "liquid_sink");

	lua_pushnumber(L, phys.acceleration_default);
	lua_setfield(L, -2, "acceleration_default");

	lua_pushnumber(L, phys.acceleration_air);
	lua_setfield(L, -2, "acceleration_air");

	return 1;
}

// set_physics_override(self, override)
int LuaLocalPlayer::l_set_physics_override(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	
	auto &phys = player->physics_override;

	getfloatfield(L, 2, "speed", phys.speed);
	getfloatfield(L, 2, "jump", phys.jump);
	getfloatfield(L, 2, "gravity", phys.gravity);
	getboolfield(L, 2, "sneak", phys.sneak);
	getboolfield(L, 2, "sneak_glitch", phys.sneak_glitch);
	getboolfield(L, 2, "new_move", phys.new_move);
	getfloatfield(L, 2, "speed_climb", phys.speed_climb);
	getfloatfield(L, 2, "speed_crouch", phys.speed_crouch);
	getfloatfield(L, 2, "liquid_fluidity", phys.liquid_fluidity);
	getfloatfield(L, 2, "liquid_fluidity_smooth", phys.liquid_fluidity_smooth);
	getfloatfield(L, 2, "liquid_sink", phys.liquid_sink);
	getfloatfield(L, 2, "acceleration_default", phys.acceleration_default);
	getfloatfield(L, 2, "acceleration_air", phys.acceleration_air);

	return 0;
}

int LuaLocalPlayer::l_get_last_pos(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	push_v3f(L, player->last_position / BS);
	return 1;
}

int LuaLocalPlayer::l_get_last_velocity(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	push_v3f(L, player->last_speed);
	return 1;
}

int LuaLocalPlayer::l_get_last_look_vertical(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushnumber(L, -1.0 * player->last_pitch * core::DEGTORAD);
	return 1;
}

int LuaLocalPlayer::l_get_last_look_horizontal(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushnumber(L, (player->last_yaw + 90.) * core::DEGTORAD);
	return 1;
}

// get_control(self)
int LuaLocalPlayer::l_get_control(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	const PlayerControl &c = player->getPlayerControl();

	auto set = [L] (const char *name, bool value) {
		lua_pushboolean(L, value);
		lua_setfield(L, -2, name);
	};

	lua_createtable(L, 0, 12);
	set("jump",  c.jump);
	set("aux1",  c.aux1);
	set("sneak", c.sneak);
	set("zoom",  c.zoom);
	set("dig",   c.dig);
	set("place", c.place);
	// Player movement in polar coordinates and non-binary speed
	lua_pushnumber(L, c.movement_speed);
	lua_setfield(L, -2, "movement_speed");
	lua_pushnumber(L, c.movement_direction);
	lua_setfield(L, -2, "movement_direction");
	// Provide direction keys to ensure compatibility
	set("up",    c.direction_keys & (1 << 0));
	set("down",  c.direction_keys & (1 << 1));
	set("left",  c.direction_keys & (1 << 2));
	set("right", c.direction_keys & (1 << 3));

	return 1;
}

// get_breath(self)
int LuaLocalPlayer::l_get_breath(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_pushinteger(L, player->getBreath());
	return 1;
}

// get_pos(self)
int LuaLocalPlayer::l_get_pos(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	push_v3f(L, player->getLegitPosition() / BS);
	return 1;
}

// get_movement_acceleration(self)
int LuaLocalPlayer::l_get_movement_acceleration(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_newtable(L);
	lua_pushnumber(L, player->movement_acceleration_default);
	lua_setfield(L, -2, "default");

	lua_pushnumber(L, player->movement_acceleration_air);
	lua_setfield(L, -2, "air");

	lua_pushnumber(L, player->movement_acceleration_fast);
	lua_setfield(L, -2, "fast");

	return 1;
}

// get_movement_speed(self)
int LuaLocalPlayer::l_get_movement_speed(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_newtable(L);
	lua_pushnumber(L, player->movement_speed_walk);
	lua_setfield(L, -2, "walk");

	lua_pushnumber(L, player->movement_speed_crouch);
	lua_setfield(L, -2, "crouch");

	lua_pushnumber(L, player->movement_speed_fast);
	lua_setfield(L, -2, "fast");

	lua_pushnumber(L, player->movement_speed_climb);
	lua_setfield(L, -2, "climb");

	lua_pushnumber(L, player->movement_speed_jump);
	lua_setfield(L, -2, "jump");

	return 1;
}

// get_movement(self)
int LuaLocalPlayer::l_get_movement(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	lua_newtable(L);

	lua_pushnumber(L, player->movement_liquid_fluidity);
	lua_setfield(L, -2, "liquid_fluidity");

	lua_pushnumber(L, player->movement_liquid_fluidity_smooth);
	lua_setfield(L, -2, "liquid_fluidity_smooth");

	lua_pushnumber(L, player->movement_liquid_sink);
	lua_setfield(L, -2, "liquid_sink");

	lua_pushnumber(L, player->movement_gravity);
	lua_setfield(L, -2, "gravity");

	return 1;
}

// get_armor_groups(self)
int LuaLocalPlayer::l_get_armor_groups(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	push_groups(L, player->getCAO()->getGroups());
	return 1;
}

// hud_add(self, form)
int LuaLocalPlayer::l_hud_add(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	HudElement *elem = new HudElement;
	read_hud_element(L, elem);

	u32 id = player->addHud(elem);
	if (id == U32_MAX) {
		delete elem;
		return 0;
	}
	lua_pushnumber(L, id);
	return 1;
}

// hud_remove(self, id)
int LuaLocalPlayer::l_hud_remove(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	u32 id = luaL_checkinteger(L, 2);
	HudElement *element = player->removeHud(id);
	if (!element)
		lua_pushboolean(L, false);
	else
		lua_pushboolean(L, true);
	delete element;
	return 1;
}

// hud_change(self, id, stat, data)
int LuaLocalPlayer::l_hud_change(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	u32 id = luaL_checkinteger(L, 2);

	HudElement *element = player->getHud(id);
	if (!element)
		return 0;

	HudElementStat stat;
	void *unused;
	bool ok = read_hud_change(L, stat, element, &unused);

	lua_pushboolean(L, ok);
	return 1;
}

// hud_get(self, id)
int LuaLocalPlayer::l_hud_get(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	u32 id = luaL_checkinteger(L, -1);

	HudElement *e = player->getHud(id);
	if (!e) {
		lua_pushnil(L);
		return 1;
	}

	push_hud_element(L, e);
	return 1;
}

// get_object(self)
int LuaLocalPlayer::l_get_object(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	ClientEnvironment &env = getClient(L)->getEnv();
	ClientActiveObject *obj = env.getGenericCAO(player->getCAO()->getId());

	push_objectRef(L, obj->getId());

	return 1;
}

// get_hotbar_size(self)
int LuaLocalPlayer::l_get_hotbar_size(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	lua_pushnumber(L, player->hud_hotbar_itemcount);
	return 1;
}


// hud_get_all(self)
int LuaLocalPlayer::l_hud_get_all(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	if (player == nullptr)
		return 0;

	lua_newtable(L);
	player->hudApply([&](const std::vector<HudElement*>& hud) {
		for (std::size_t id = 0; id < hud.size(); ++id) {
			HudElement *elem = hud[id];
			if (elem != nullptr) {
				push_hud_element(L, elem);
				lua_rawseti(L, -2, id);
			}
		}
	});
	return 1;
}

LocalPlayer *LuaLocalPlayer::getobject(LuaLocalPlayer *ref)
{
	return ref->m_localplayer;
}

LocalPlayer *LuaLocalPlayer::getobject(lua_State *L, int narg)
{
	LuaLocalPlayer *ref = checkObject<LuaLocalPlayer>(L, narg);
	assert(ref);
	LocalPlayer *player = getobject(ref);
	assert(player);
	return player;
}

int LuaLocalPlayer::gc_object(lua_State *L)
{
	LuaLocalPlayer *o = *(LuaLocalPlayer **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

void LuaLocalPlayer::Register(lua_State *L)
{
	static const luaL_Reg metamethods[] = {
		{"__gc", gc_object},
		{0, 0}
	};
	registerClass(L, className, methods, metamethods);
}


/*
SETTERS
*/
// set_velocity(self, velocity)
int LuaLocalPlayer::l_set_velocity(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	v3f pos = checkFloatPos(L, 2);
	player->setSpeed(pos);

	return 0;
}

// set_yaw(self, yaw)
int LuaLocalPlayer::l_set_yaw(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	if (lua_isnumber(L, 2)) {
		double yaw = lua_tonumber(L, 2);
		player->setLegitYaw(yaw);
		if (!g_settings->getBool("freecam")) {
			g_game->cam_view.camera_yaw = yaw;
			g_game->cam_view_target.camera_yaw = yaw;
		}
	}

	return 0;
}

// set_pitch(self, pitch)
int LuaLocalPlayer::l_set_pitch(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	if (lua_isnumber(L, 2)) {
		double pitch = lua_tonumber(L, 2);
		player->setLegitPitch(pitch);
		if (!g_settings->getBool("freecam")) {
			g_game->cam_view.camera_pitch = pitch;
			g_game->cam_view_target.camera_pitch = pitch;
		}
	}

	return 0;
}

// set_wield_index(self, index)
int LuaLocalPlayer::l_set_wield_index(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	u32 index = luaL_checkinteger(L, 2) - 1;

	player->setWieldIndex(index);
	g_game->processItemSelection(&g_game->runData.new_playeritem);
	ItemStack selected_item, hand_item;
	ItemStack &tool_item = player->getWieldedItem(&selected_item, &hand_item);
	g_game->camera->wield(tool_item);
	return 0;
}

// set_pos(self, pos)
int LuaLocalPlayer::l_set_pos(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	v3f pos = checkFloatPos(L, 2);
	player->setLegitPosition(pos);
	getClient(L)->sendPlayerPos();
	return 0;
}

// get_pointed_thing(self)
int LuaLocalPlayer::l_get_pointed_thing(lua_State *L)
{
	PointedThing pointed = g_game->runData.pointed_old;
	push_pointed_thing(L, pointed, true, true);
	return 1;
}

// get_object_or_nil(self)
int LuaLocalPlayer::l_get_object_or_nil(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);

	GenericCAO *gcao = player->getCAO();
	if (gcao) {
		//push_generic_cao
		lua_newtable(L);
		lua_pushnumber(L, gcao->getId());
		lua_setfield(L, -2, "id");
		lua_pushboolean(L, gcao->isPlayer());
		lua_setfield(L, -2, "is_player");
		lua_pushboolean(L, gcao->isLocalPlayer());
		lua_setfield(L, -2, "is_local_player");
		lua_pushboolean(L, gcao->isVisible());
		lua_setfield(L, -2, "is_visible");
		lua_pushstring(L, gcao->getName().c_str());
		lua_setfield(L, -2, "name");
		push_v3f(L, gcao->getPosition() / BS);
		lua_setfield(L, -2, "position");
		push_v3f(L, gcao->getVelocity() / BS);
		lua_setfield(L, -2, "velocity");
		push_v3f(L, gcao->getAcceleration());
		lua_setfield(L, -2, "acceleration");
		push_v3f(L, gcao->getRotation());
		lua_setfield(L, -2, "rotation");
		lua_pushnumber(L, gcao->getHp());
		lua_setfield(L, -2, "hp");
		lua_pushboolean(L, gcao->isImmortal());
		lua_setfield(L, -2, "is_immortal");
		lua_pushboolean(L, gcao->collideWithObjects());
		lua_setfield(L, -2, "collide_with_objects");
		push_object_properties(L, &gcao->getProperties());
		lua_setfield(L, -2, "props");
		return 1;
	}
	lua_pushnil(L);
	return 0;
}

// get_entity_relationship(self, object_id)
int LuaLocalPlayer::l_get_entity_relationship(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	u16 object_id = lua_tointeger(L, 2);

	ClientEnvironment &env = getClient(L)->getEnv();
	GenericCAO *gcao = env.getGenericCAO(object_id);

	EntityRelationship relationship = player->getEntityRelationship(gcao);

	lua_pushinteger(L, relationship);

	return 1;
}

// punch(self, object_id)
int LuaLocalPlayer::l_punch(lua_State *L)
{
	u16 object_id = lua_tointeger(L, 2);

	g_game->runData.punching = true;
	g_game->runData.time_from_last_punch = 0;
	g_game->runData.object_hit_delay_timer = 0.2;

	PointedThing pointed(object_id, v3f(0, 0, 0), v3f(0, 0, 0), v3f(0, 0, 0), 0, PointabilityType::POINTABLE);
	getClient(L)->interact(INTERACT_START_DIGGING, pointed);

	return 0;
}

// get_time_from_last_punch(self)
int LuaLocalPlayer::l_get_time_from_last_punch(lua_State *L)
{
	lua_pushnumber(L, g_game->runData.time_from_last_punch);

	return 1;
}

// get_hotbar_length(self)
int LuaLocalPlayer::l_get_hotbar_length(lua_State *L)
{
	LocalPlayer *player = getobject(L, 1);
	lua_pushnumber(L, player->hud_hotbar_itemcount);

	return 1;
}


const char LuaLocalPlayer::className[] = "LocalPlayer";
const luaL_Reg LuaLocalPlayer::methods[] = {
		luamethod(LuaLocalPlayer, get_velocity),
		luamethod(LuaLocalPlayer, get_hp),
		luamethod(LuaLocalPlayer, get_name),
		luamethod(LuaLocalPlayer, get_yaw),
		luamethod(LuaLocalPlayer, get_pitch),
		luamethod(LuaLocalPlayer, get_wield_index),
		luamethod(LuaLocalPlayer, get_wielded_item),
		luamethod(LuaLocalPlayer, is_attached),
		luamethod(LuaLocalPlayer, is_touching_ground),
		luamethod(LuaLocalPlayer, is_in_liquid),
		luamethod(LuaLocalPlayer, is_in_liquid_stable),
		luamethod(LuaLocalPlayer, is_climbing),
		luamethod(LuaLocalPlayer, swimming_vertical),
		luamethod(LuaLocalPlayer, get_physics_override),
        luamethod(LuaLocalPlayer, set_physics_override),
		// TODO: figure our if these are useful in any way
		luamethod(LuaLocalPlayer, get_last_pos),
		luamethod(LuaLocalPlayer, get_last_velocity),
		luamethod(LuaLocalPlayer, get_last_look_horizontal),
		luamethod(LuaLocalPlayer, get_last_look_vertical),
		//
		luamethod(LuaLocalPlayer, get_control),
		luamethod(LuaLocalPlayer, get_breath),
		luamethod(LuaLocalPlayer, get_pos),
		luamethod(LuaLocalPlayer, get_movement_acceleration),
		luamethod(LuaLocalPlayer, get_movement_speed),
		luamethod(LuaLocalPlayer, get_movement),
		luamethod(LuaLocalPlayer, get_armor_groups),
		luamethod(LuaLocalPlayer, hud_add),
		luamethod(LuaLocalPlayer, hud_remove),
		luamethod(LuaLocalPlayer, hud_change),
		luamethod(LuaLocalPlayer, hud_get),
        luamethod(LuaLocalPlayer, get_object),
        luamethod(LuaLocalPlayer, get_hotbar_size),
		luamethod(LuaLocalPlayer, hud_get_all),

		luamethod(LuaLocalPlayer, get_move_resistance),

		luamethod(LuaLocalPlayer, set_velocity),
		luamethod(LuaLocalPlayer, set_yaw),
		luamethod(LuaLocalPlayer, set_pitch),
		luamethod(LuaLocalPlayer, set_wield_index),
		luamethod(LuaLocalPlayer, set_pos),
		luamethod(LuaLocalPlayer, get_pointed_thing),
		luamethod(LuaLocalPlayer, get_object_or_nil),
		luamethod(LuaLocalPlayer, get_entity_relationship),
		luamethod(LuaLocalPlayer, punch),
		luamethod(LuaLocalPlayer, get_time_from_last_punch),
		luamethod(LuaLocalPlayer, get_wielded_item_range),
		luamethod(LuaLocalPlayer, get_hotbar_length),

		{0, 0}
};
