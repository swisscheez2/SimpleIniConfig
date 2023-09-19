#include "config.h"
#include <iostream>
#include "kbinds.h"

FkeyBinds gKeyBinds;

namespace cfg
{
	ADD_CFG_ITEM(int, testInt, 0, Section1);
	ADD_CFG_ITEM(float, testFloat, 1.0f, Section1);
	ADD_CFG_ITEM(bool, testBool, true, Section1);
	ADD_CFG_ITEM(AmiKeyBind, TestKey, AmiKeyBind(VK_MBUTTON), Section1);
	ADD_CFG_ITEM(shared::col_t,color1,shared::col_t(255,0,0,255),Section2)
	ADD_CFG_ITEM(shared::col_t,  color2, shared::col_t(255,0,0,255), Section2)



}

int main()
{


	config::set_config_directory("somedir" );
	config::load_settings("test");
	auto booltest = config::get_item(cfg::c_Section1_testBool).get<bool>();
	booltest = !booltest;
	config::get_item(cfg::c_Section1_testBool).set(booltest);
	auto color1 = config::get_item(cfg::c_Section2_color1).get<shared::col_t>();
	config::get_item(cfg::c_Section2_color1).set<shared::col_t>(color1);
	config::save("test");
	auto path = std::filesystem::path(config::get_config_directory());
	auto MenuAndKeyFile = (path / "UiAndKeyData");

	gMenuKeyData = LoadMenuAndKeyNames(MenuAndKeyFile.string());
	gKeyBinds = FkeyBinds((path / "keybinds").string());

	gKeyBinds.continuousKeyCheck();
}