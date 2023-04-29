#include "interface.hpp"

using namespace opengl;

glm::mat4 transformForLight(void *info)
{
	auto params = (window::defaultWindowInfo*)info;
	glm::mat4 ret(1.0f);
	ret = glm::translate(ret, glm::vec3(cos((float)params->time) * 5.0, sin((float)params->time) * 5.0, 2.0f));
	return ret;
}

glm::mat4 transformForBox(void *info)
{
	auto params = (window::defaultWindowInfo*)info;
	glm::mat4 ret(1.0f);
	ret = glm::rotate(ret, glm::radians((float)params->time * (float)params->degrees), params->rotateAxis);
	return ret;
}

int main(int argc, char **argv)
{
	window *first = new window();
	auto params = (window::defaultWindowInfo*)first->params;
	params->degrees = 0.0f;
	params->jsonFileName = "model.json";
	first->init();
	auto &defaultUsage = params->renderArray->getUsage();
	defaultUsage["light"][0].callback = transformForLight;
	// for (auto &box : defaultUsage["box"])
	// {
	// 	box.callback = transformForBox;
	// }
	first->start();
	delete first;
	return 0;
}
