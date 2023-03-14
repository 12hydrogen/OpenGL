#pragma once

#include "arrayLoader.hpp"
#include "camera.hpp"

namespace opengl
{
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

	using keyboardInteract = interaction<void (*)(window*, int, int, int, int)>;
	using scrollInteract = interaction<void (*)(window*, double, double)>;
	using mouseInteract = interaction<void (*)(window*, double, double)>;

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

		friend class window;
	};

	class window
	{
	private:
		static map<GLFWwindow*, window*> callbackMap;

		static bool initialized;

		GLFWwindow *windowPtr;

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
				abstractWindowInfo *tempPtr = callbackMap.at(window)->params;
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
				callbackMap.at(window)->runKeyboard(key, scancode, action, mods);
			}
			catch (std::out_of_range &e)
			{}
		}
		static void globalScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
		{
			try
			{
				callbackMap.at(window)->runScroll(xOffset, yOffset);
			}
			catch (std::out_of_range &e)
			{}
		}
		static void globalMouseCallback(GLFWwindow *window, double xPos, double yPos)
		{
			try
			{
				callbackMap.at(window)->runMouse(xPos, yPos);
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

			abstractWindowInfo(const char *title, int width, int height):
			title(title), width(width), height(height) {}
		};

		// Default render info
		struct defaultWindowInfo: abstractWindowInfo
		{
		public:
			string jsonFileName;
			objectArray *renderArray;
			camera *defaultCamera;
			vector<float> backgroundColor;

			defaultWindowInfo(const char *title, const char *jsonName, int width, int height, vector<float> bgColor):
			abstractWindowInfo(title, width, height),
			jsonFileName(jsonName), renderArray(NULL), defaultCamera(NULL), backgroundColor(bgColor) {}
		};

		window(
			interactModel model, int width = 800.0, int height = 600.0,
			const char *title = "", const char *jsonName = "default.json",
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
					glfwTerminate();
					throw error("GLAD loader init failed.");
				}
				initialized = true;
			}
			// Create window
			windowPtr = glfwCreateWindow(params->width, params->height, params->title, NULL, NULL);
			if (windowPtr == NULL)
				throw error("Window create failed.");

			glfwMakeContextCurrent(windowPtr);

			glViewport(0, 0, params->width, params->height);

			// Set callback
			glfwSetFramebufferSizeCallback(windowPtr, framebufferCallback);
			glfwSetCursorPosCallback(windowPtr, globalMouseCallback);
			glfwSetScrollCallback(windowPtr, globalScrollCallback);
			glfwSetKeyCallback(windowPtr, globalKeyboardCallback);

			glEnable(GL_DEPTH_TEST);

			callbackMap.emplace(std::pair(windowPtr, this));
		}
		~window()
		{
			callbackMap.erase(windowPtr);
			if (callbackMap.empty())
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
				frameRate = 1.0 / (curTime - lastFrame);
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
			defaultWindowInfo *temp = (defaultWindowInfo*)currentWindow->params;
			temp->renderArray = new objectArray(temp->jsonFileName);
			temp->defaultCamera = new camera({0.0, 0.0, 0.0});
		}
		static void defaultRenderCallback(window *currentWindow)
		{}
	};

	bool window::initialized = false;
}
