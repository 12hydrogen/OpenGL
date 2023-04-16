#pragma once

#include "arrayLoader.hpp"
#include "camera.hpp"

#include <thread>
#include <atomic>
#include <any>

namespace opengl
{
	using namespace std;

	class window;

	// Interaction with single input device, using callback
	template <typename T>
	class interaction
	{
	private:
		T callback;
	public:
		interaction(T callback):
		callback(callback) {}

		friend class window;
	};

	using keyboardFunction = void (*)(window*, int, int, int, int);
	using scrollFunction = void (*)(window*, double, double);
	using mouseFunction = void (*)(window*, double, double);

	using keyboardInteract = interaction<keyboardFunction>;
	using scrollInteract = interaction<scrollFunction>;
	using mouseInteract = interaction<mouseFunction>;

	// Interactions with mutiply input devices
	class interactModel
	{
	private:
		keyboardInteract keyboard;
		scrollInteract scroll;
		mouseInteract mouse;
	public:
		interactModel(keyboardInteract k, scrollInteract s, mouseInteract m):
		keyboard(k), scroll(s), mouse(m) {}
		interactModel(keyboardFunction k, scrollFunction s, mouseFunction m):
		keyboard(k), scroll(s), mouse(m) {}

		friend class window;
	};

	class window
	{
	private:
		static map<GLFWwindow*, window*> existingWindow;

		static bool initialized;

		GLFWwindow *windowPtr;
		thread *windowThread;

		// Callback processing inputs
		interactModel model;
		void runKeyboard(int key, int scancode, int action, int mods)
		{
			model.keyboard.callback(this, key, scancode, action, mods);
		}
		void runScroll(double xOffset, double yOffset)
		{
			model.scroll.callback(this, xOffset, yOffset);
		}
		void runMouse(double xPos, double yPos)
		{
			model.mouse.callback(this, xPos, yPos);
		}

		static void defaultKeyboard(window* w, int key, int scancode, int action, int mods)
		{
			defaultWindowInfo *info = (defaultWindowInfo*)w->params;
			switch (key)
			{
				case GLFW_KEY_ESCAPE:
				{
					glfwSetWindowShouldClose(w->windowPtr, true);
					break;
				}
				case GLFW_KEY_W:
				{
					info->defaultCamera->move(FRONT, info->frameDelta);
					break;
				}
				case GLFW_KEY_S:
				{
					info->defaultCamera->move(BACK, info->frameDelta);
					break;
				}
				case GLFW_KEY_A:
				{
					info->defaultCamera->move(LEFT, info->frameDelta);
					break;
				}
				case GLFW_KEY_D:
				{
					info->defaultCamera->move(RIGHT, info->frameDelta);
					break;
				}
			}
		}
		static void defaultScroll(window* w, double xOffset, double yOffset)
		{
			defaultWindowInfo *info = (defaultWindowInfo*)w->params;
			info->defaultCamera->zoomChange(yOffset);
		}
		static void defaultMouse(window* w, double xPos, double yPos)
		{
			defaultWindowInfo *info = (defaultWindowInfo*)w->params;
			if (info->firstEnter)
			{
				info->lastX = xPos;
				info->lastY = yPos;
				firstEnter = false;
			}

			baseCamera.view(xPos - info->lastX, info->lastY - yPos);
			info->lastX = xPos;
			info->lastY = yPos;
		}
		// Render circle
		typedef void (*render)(window*);
		render preRenderCallback;
		render renderCallback;

		// Frame Counter
		bool counterInitialized;
		double lastFrame;
		double frameRate;

		// Global callback layer
		static void frameBufferCallback(GLFWwindow *window, int width, int height)
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

		static void globalKeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			try
			{
				existingWindow.at(window)->runKeyboard(key, scancode, action, mods);
			}
			catch (std::out_of_range &e)
			{}
		}
		static void globalScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
		{
			try
			{
				existingWindow.at(window)->runScroll(xOffset, yOffset);
			}
			catch (std::out_of_range &e)
			{}
		}
		static void globalMouseCallback(GLFWwindow *window, double xPos, double yPos)
		{
			try
			{
				existingWindow.at(window)->runMouse(xPos, yPos);
			}
			catch (std::out_of_range &e)
			{}
		}
	public:
		// Basic window info, not including render info
		struct abstractWindowInfo
		{
		public:
			const char *title;
			int width;
			int height;

			double frameDelta;
			double lastX;
			double lastY;

			abstractWindowInfo(const char *title, int width, int height):
			title(title), width(width), height(height), frameDelta(0.0), lastX(0.0), lastY(0.0) {}
		};

		// Default render info
		struct defaultWindowInfo: abstractWindowInfo
		{
		public:
			string jsonFileName;

			objectArray *renderArray;
			map<string, initializer_list<any>> uniform;
			camera *defaultCamera;

			vector<float> backgroundColor;
			bool firstEnter;

			defaultWindowInfo(const char *title, const char *jsonName, int width, int height, vector<float> bgColor):
			abstractWindowInfo(title, width, height),
			jsonFileName(jsonName), renderArray(NULL), defaultCamera(NULL), backgroundColor(bgColor), firstEnter(true) {}
		};

		window(
			const char *title = "", int width = 800.0, int height = 600.0,
			interactModel model = interactModel(defaultKeyboard, defaultScroll, defaultMouse),
			const char *jsonName = "default.json",
			vector<float> backgroundColor = {0.2f, 0.3f, 0.3f, 1.0f}
		):
		model(model),
		params(new defaultWindowInfo(title, jsonName, width, height, backgroundColor)),
		preRenderCallback(defaultPreRenderCallback),
		renderCallback(defaultRenderCallback),
		counterInitialized(false),
		frameRate(0.0)
		{
			// Init GLFW
			if (!initialized)
			{
				glfwInit();
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
				{
					const char *desp;
					int code = glfwGetError(&desp);
					glfwTerminate();
					throw error("GLAD loader init failed.", desp);
				}
				initialized = true;
			}
			// Create window
			windowPtr = glfwCreateWindow(params->width, params->height, params->title, NULL, NULL);
			if (windowPtr == NULL)
			{
				const char *desp;
				int code = glfwGetError(&desp);
				throw error("Window create failed.", desp);
			}

			glfwMakeContextCurrent(windowPtr);

			glViewport(0, 0, params->width, params->height);

			// Set callback
			glfwSetFramebufferSizeCallback(windowPtr, frameBufferCallback);
			glfwSetCursorPosCallback(windowPtr, globalMouseCallback);
			glfwSetScrollCallback(windowPtr, globalScrollCallback);
			glfwSetKeyCallback(windowPtr, globalKeyboardCallback);

			glEnable(GL_DEPTH_TEST);

			existingWindow.emplace(std::pair(windowPtr, this));
		}
		~window()
		{
			glfwSetWindowShouldClose(windowPtr, true);
			existingWindow.erase(windowPtr);
			if (existingWindow.empty())
			{
				glfwTerminate();
				initialized = false;
			}
		}

		// Parameter of window
		abstractWindowInfo *params;

		void frameCounter()
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

		void start()
		{
			preRenderCallback(this);
			while(!glfwWindowShouldClose(windowPtr))
			{
				renderCallback(this);
				frameCounter();
			}
		}

		void startDetach()
		{}

		void setPreRender(render preCallback)
		{
			preRenderCallback = preCallback;
		}
		void setRender(render callback)
		{
			renderCallback = callback;
		}

		static void defaultPreRenderCallback(window *currentWindow)
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
			catch (error &e)
			{
				throw error("Camera init failed.", e.what());
			}
		}
		static void defaultRenderCallback(window *currentWindow)
		{
			defaultWindowInfo *info = (defaultWindowInfo*)currentWindow->params;
			glClearColor(info->backgroundColor[0], info->backgroundColor[1], info->backgroundColor[2], info->backgroundColor[3]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (auto &object : info->renderArray->getDefination())
			{
				const shaderProgram &temp = object.second.getShaderProgram();
				for (auto uniformPair : info->uniform)
				{
					temp[uniformPair.first] = uniformPair.second;
				}
				object.second.draw();
			}
		}
	};

	bool window::initialized = false;
}
