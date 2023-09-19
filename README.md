# SimpleIniConfig
simple ini style config loading and saving


	config::set_config_directory("configs"); 
	config::save("test");
	config::load("test");
	auto col = config::get_item(cfg::color1).get<shared::col_t>();

can save int floats bools and vectors like a color vector. 

adding values is as simple as declaring them as such:


namespace cfg
{
	ADD_CFG_ITEM(int, testInt, 0, Section1);
	ADD_CFG_ITEM(float, testFloat, 1.0f, Section1);
	ADD_CFG_ITEM(bool, testBool, true, Section1);
	ADD_CFG_ITEM_VEC(<shared::col_t>, int,4,color1,125,Section2)
	ADD_CFG_ITEM_VEC(<shared::col_t>, int, 4, color2, 255, Section2)
}
