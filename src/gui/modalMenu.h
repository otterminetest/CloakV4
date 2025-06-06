/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#pragma once

#include "irrlichttypes_extrabloated.h"
#include "irr_ptr.h"
#include "util/string.h"
#ifdef __ANDROID__
	#include <porting_android.h>
#endif

enum class PointerType {
	Mouse,
	Touch,
};

struct PointerAction {
	v2s32 pos;
	u64 time; // ms

	static PointerAction fromEvent(const SEvent &event);
	bool isRelated(PointerAction other);
};

class GUIModalMenu;

class IMenuManager
{
public:
	// A GUIModalMenu calls these when this class is passed as a parameter
	virtual void createdMenu(gui::IGUIElement *menu) = 0;
	virtual void deletingMenu(gui::IGUIElement *menu) = 0;
};

// Remember to drop() the menu after creating, so that it can
// remove itself when it wants to.

class GUIModalMenu : public gui::IGUIElement
{
public:
	GUIModalMenu(gui::IGUIEnvironment* env, gui::IGUIElement* parent, s32 id,
		IMenuManager *menumgr, bool remap_dbl_click = true);
	virtual ~GUIModalMenu();

	void allowFocusRemoval(bool allow);
	bool canTakeFocus(gui::IGUIElement *e);
	void draw();
	void quitMenu();

	virtual void regenerateGui(v2u32 screensize) = 0;
	virtual void drawMenu() = 0;
	virtual bool preprocessEvent(const SEvent &event);
	virtual bool OnEvent(const SEvent &event) { return false; };
	virtual bool pausesGame() { return false; } // Used for pause menu
#ifdef __ANDROID__
	virtual void getAndroidUIInput() {};
	porting::AndroidDialogState getAndroidUIInputState();
#endif

	PointerType getPointerType() { return m_pointer_type; };

protected:
	virtual std::wstring getLabelByID(s32 id) = 0;
	virtual std::string getNameByID(s32 id) = 0;

	// Stores the last known pointer type.
	PointerType m_pointer_type = PointerType::Mouse;
	// Stores the last known pointer position.
	// If the last input event was a mouse event, it's the cursor position.
	// If the last input event was a touch event, it's the finger position.
	v2s32 m_pointer;
	v2s32 m_old_pointer;  // Mouse position after previous mouse event

	v2u32 m_screensize_old;
	float m_gui_scale;
#ifdef __ANDROID__
	std::string m_jni_field_name;
#endif

	struct ScalingInfo {
		f32 scale;
		core::rect<s32> rect;
	};
	ScalingInfo getScalingInfo(v2u32 screensize, v2u32 base_size);


	// This is set to true if the menu is currently processing a second-touch event.
	bool m_second_touch = false;
	// This is set to true if the menu is currently processing a mouse event
	// that was synthesized by the menu itself from a touch event.
	bool m_simulated_mouse = false;

private:
	IMenuManager *m_menumgr;
	/* If true, remap a click outside the formspec to ESC. This is so that, for
	 * example, touchscreen users can close formspecs.
	 * The default for this setting is true. Currently, it's set to false for
	 * the mainmenu to prevent Minetest from closing unexpectedly.
	 */
	bool m_remap_click_outside;
	bool remapClickOutside(const SEvent &event);
	PointerAction m_last_click_outside{};

	// This might be necessary to expose to the implementation if it
	// wants to launch other menus
	bool m_allow_focus_removal = false;

	// Stuff related to touchscreen input

	irr_ptr<gui::IGUIElement> m_touch_hovered;

	// Converts touches into clicks.
	bool simulateMouseEvent(ETOUCH_INPUT_EVENT touch_event, bool second_try=false);
	void enter(gui::IGUIElement *element);
	void leave();

	// Used to detect double-taps and convert them into double-click events.
	PointerAction m_last_touch{};
};
