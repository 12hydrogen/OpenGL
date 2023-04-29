#include "interface.hpp"
#include <iostream>

namespace opengl
{
	void window::runKeyboard(int key, int scancode, int action, int mods)
	{
		model.keyboard.callback(this, key, scancode, action, mods);
	}
	void window::runScroll(double xOffset, double yOffset)
	{
		model.scroll.callback(this, xOffset, yOffset);
	}
	void window::runMouse(double xPos, double yPos)
	{
		model.mouse.callback(this, xPos, yPos);
	}
	void window::defaultKeyboard(window* w, int key, int scancode, int action, int mods)
	{
		defaultWindowInfo *info = (defaultWindowInfo*)w->params;
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
			{
				glfwSetWindowShouldClose(w->windowPtr, true);
				break;
			}
			case GLFW_KEY_LEFT_ALT:
			{
				if (action == GLFW_PRESS)
				{
					glfwSetInputMode(w->windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					info->firstEnter = true;
				}
				if (action == GLFW_RELEASE)
					glfwSetInputMode(w->windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
	}
	void window::defaultMovement(window* w)
	{
		defaultWindowInfo *info = (defaultWindowInfo*)w->params;
		if (glfwGetKey(w->windowPtr, GLFW_KEY_W))
			info->defaultCamera->move(FRONT, info->frameDelta);
		if (glfwGetKey(w->windowPtr, GLFW_KEY_S))
			info->defaultCamera->move(BACK, info->frameDelta);
		if (glfwGetKey(w->windowPtr, GLFW_KEY_A))
			info->defaultCamera->move(LEFT, info->frameDelta);
		if (glfwGetKey(w->windowPtr, GLFW_KEY_D))
			info->defaultCamera->move(RIGHT, info->frameDelta);
	}
	void window::defaultScroll(window* w, double xOffset, double yOffset)
	{
		defaultWindowInfo *info = (defaultWindowInfo*)w->params;
		info->defaultCamera->zoomChange(yOffset);
	}
	void window::defaultMouse(window* w, double xPos, double yPos)
	{
		defaultWindowInfo *info = (defaultWindowInfo*)w->params;
		if (info->firstEnter)
		{
			info->lastX = xPos;
			info->lastY = yPos;
			info->firstEnter = false;
		}
		info->defaultCamera->view(xPos - info->lastX, info->lastY - yPos);
		info->lastX = xPos;
		info->lastY = yPos;
	}
	void window::frameBufferCallback(GLFWwindow *window, int width, int height)
	{
		try
		{
			glfwMakeContextCurrent(window);
			glViewport(0, 0, width, height);
			abstractWindowInfo *tempPtr = existingWindow.at(window)->params;
			tempPtr->width = width;
			tempPtr->height = height;
		}
		catch (std::out_of_range &e)
		{}
	}
	void window::globalKeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		try
		{
			existingWindow.at(window)->runKeyboard(key, scancode, action, mods);
		}
		catch (std::out_of_range &e)
		{}
	}
	void window::globalScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
	{
		try
		{
			existingWindow.at(window)->runScroll(xOffset, yOffset);
		}
		catch (std::out_of_range &e)
		{}
	}
	void window::globalMouseCallback(GLFWwindow *window, double xPos, double yPos)
	{
		try
		{
			existingWindow.at(window)->runMouse(xPos, yPos);
		}
		catch (std::out_of_range &e)
		{}
	}
	void window::defaultPreRenderCallback(window *currentWindow)
	{
		try
		{
			defaultWindowInfo *info = (defaultWindowInfo*)currentWindow->params;
			info->renderArray = new objectArray(info->jsonFileName);
			info->defaultCamera = new camera({0.0, 0.0, 0.0});
		}
		catch (json::parse_error &e)
		{
			throw error("Object reading failed.", e.what());
		}
		glfwSetInputMode(currentWindow->windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	void window::defaultRenderCallback(window *currentWindow)
	{
		defaultWindowInfo *info = (defaultWindowInfo*)currentWindow->params;
		currentWindow->preRenderLoop();
		for (auto &object : info->renderArray->getDefination())
		{
			shaderProgram &temp = object.second[0].getShaderProgram();
			for (auto &uniformPair : info->uniform)
			{
				temp[uniformPair.first] = uniformPair.second;
			}
		}
		defaultMovement(currentWindow);
		info->renderArray->draw(info->defaultCamera->getLookAt(),
								info->defaultCamera->getPerspective(info->width / info->height),
								info->defaultCamera->getPosition(),
								info->defaultCamera->getFacing(),
								info);
		currentWindow->postRenderLoop();
	}
	window::window(
		const char *title, int width, int height,
		interactModel model,
		const char *jsonName,
		vector<float> backgroundColor
	):
	model(model),
	preRenderCallback(defaultPreRenderCallback),
	renderCallback(defaultRenderCallback),
	counterInitialized(false),
	frameRate(0.0),
	params(new defaultWindowInfo(title, jsonName, width, height, backgroundColor))
	{
		// Init GLFW
		if (!initialized)
		{
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			initialized = true;
		}
		// Create window
		windowPtr = glfwCreateWindow(params->width, params->height, params->title, NULL, NULL);
		if (windowPtr == NULL)
		{
			const char *desp;
			glfwGetError(&desp);
			throw error("Window create failed.", desp);
		}
		glfwMakeContextCurrent(windowPtr);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			const char *desp;
			glfwGetError(&desp);
			glfwTerminate();
			throw error("GLAD loader init failed.", desp);
		}
		glViewport(0, 0, params->width, params->height);
		// Set callback
		glfwSetFramebufferSizeCallback(windowPtr, frameBufferCallback);
		glfwSetCursorPosCallback(windowPtr, globalMouseCallback);
		glfwSetScrollCallback(windowPtr, globalScrollCallback);
		glfwSetKeyCallback(windowPtr, globalKeyboardCallback);
		glEnable(GL_DEPTH_TEST);
		existingWindow.emplace(std::pair(windowPtr, this));
		glfwMakeContextCurrent(NULL);
	}
	window::~window()
	{
		glfwSetWindowShouldClose(windowPtr, true);
		existingWindow.erase(windowPtr);
		if (existingWindow.empty())
		{
			glfwTerminate();
			initialized = false;
		}
	}

	void window::frameCounter()
	{
		double curTime = glfwGetTime();
		if (counterInitialized)
		{
			params->frameDelta = curTime - lastFrame;
			frameRate = 1.0 / (curTime - lastFrame);
		}
		else
			counterInitialized = true;
		lastFrame = curTime;
	}

	void window::preRenderLoop()
	{
		glfwSwapBuffers(windowPtr);
		glfwPollEvents();
		glClearColor(params->backgroundColor[0], params->backgroundColor[1], params->backgroundColor[2], params->backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (params->enableDepth)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		if (params->enableStencil)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
		if (params->enableBlending)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		if (params->enableFaceCulling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		params->time = glfwGetTime();
	}
	void window::postRenderLoop()
	{
		glFlush();
	}

	void window::init()
	{
		glfwMakeContextCurrent(windowPtr);
		preRenderCallback(this);
		glfwMakeContextCurrent(NULL);
	}
	void window::start()
	{
		glfwMakeContextCurrent(windowPtr);
		while(!glfwWindowShouldClose(windowPtr))
		{
			renderCallback(this);
			glErrorAssert();
			frameCounter();
			const char *ptr;
			if (glfwGetError(&ptr) != GLFW_NO_ERROR)
			{
				throw error(ptr);
			}
		}
		glfwMakeContextCurrent(NULL);
	}
	void window::startDetach()
	{}

	void window::setPreRender(render preCallback)
	{
		preRenderCallback = preCallback;
	}
	void window::setRender(render callback)
	{
		renderCallback = callback;
	}

	bool window::initialized = false;
	map<GLFWwindow*, window*> window::existingWindow = map<GLFWwindow*, window*>();
}
