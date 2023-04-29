#pragma once

#include "loader/arrayLoader.hpp"
#include "camera.hpp"

#include <thread>
#include <atomic>
#include <functional>
#include <any>

namespace opengl
{
	using namespace std;

	class DLL_SIGN window;

	// Interaction with single input device, using callback
	template <typename T>
	class DLL_SIGN interaction
	{
	private:
		T callback;
	public:
		interaction(T callback):
		callback(callback) {}

		friend class window;
	};

	using keyboardFunction = function<void (window*, int, int, int, int)>;
	using scrollFunction = function<void (window*, double, double)>;
	using mouseFunction = function<void (window*, double, double)>;

	using keyboardInteract = interaction<keyboardFunction>;
	using scrollInteract = interaction<scrollFunction>;
	using mouseInteract = interaction<mouseFunction>;

	// Interactions with mutiply input devices
	class DLL_SIGN interactModel
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

	class DLL_SIGN window
	{
	private:
		static DLL_SIGN map<GLFWwindow*, window*> existingWindow;

		static DLL_SIGN bool initialized;

		GLFWwindow *windowPtr;
		// thread *windowThread;

		// Callback processing inputs
		interactModel model;
		void runKeyboard(int key, int scancode, int action, int mods);
		void runScroll(double xOffset, double yOffset);
		void runMouse(double xPos, double yPos);

		static DLL_SIGN void defaultKeyboard(window* w, int key, int scancode, int action, int mods);
		static DLL_SIGN void defaultMovement(window* w);
		static DLL_SIGN void defaultScroll(window* w, double xOffset, double yOffset);
		static DLL_SIGN void defaultMouse(window* w, double xPos, double yPos);
		// Render circle
		typedef void (*render)(window*);
		render preRenderCallback;
		render renderCallback;

		// Frame Counter
		bool counterInitialized;
		double lastFrame;
		double frameRate;

		// Global callback layer
		static DLL_SIGN void frameBufferCallback(GLFWwindow *window, int width, int height);

		static DLL_SIGN void globalKeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static DLL_SIGN void globalScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
		static DLL_SIGN void globalMouseCallback(GLFWwindow *window, double xPos, double yPos);
		static DLL_SIGN void defaultPreRenderCallback(window *currentWindow);
		static DLL_SIGN void defaultRenderCallback(window *currentWindow);
	public:
		// Basic window info, not including render info
		struct abstractWindowInfo
		{
		public:
			const char *title;
			double width;
			double height;

			double frameDelta;
			double lastX;
			double lastY;

			double time;
			vector<float> backgroundColor;

			bool enableDepth;
			bool enableStencil;
			bool enableBlending;
			bool enableFaceCulling;

			// This could discard the usage of pointer cast.
			vector<any> anyArgs;

			abstractWindowInfo(const char *title, int width, int height, const vector<float> &bgColor):
			title(title), width(width), height(height), frameDelta(0.0), lastX(0.0), lastY(0.0), backgroundColor(bgColor),
			enableDepth(true), enableStencil(false), enableBlending(false), enableFaceCulling(true) {}

			abstractWindowInfo(const char *title, int width, int height, vector<float> &&bgColor):
			title(title), width(width), height(height), frameDelta(0.0), lastX(0.0), lastY(0.0), backgroundColor(bgColor),
			enableDepth(true), enableStencil(false), enableBlending(false), enableFaceCulling(true) {}
		};

		// Default render info
		struct defaultWindowInfo: abstractWindowInfo
		{
		public:
			string jsonFileName;

			objectArray *renderArray;
			map<string, vector<float>> uniform;
			camera *defaultCamera;

			glm::vec3 rotateAxis;
			float degrees;

			bool firstEnter;

			defaultWindowInfo(const char *title, const char *jsonName, int width, int height, const vector<float> &bgColor):
			abstractWindowInfo(title, width, height, bgColor),
			jsonFileName(jsonName), renderArray(NULL), defaultCamera(NULL), rotateAxis(glm::vec3(0.5f, 1.0f, 0.0f)), degrees(50.0f), firstEnter(true) {}

			defaultWindowInfo(const char *title, const char *jsonName, int width, int height, vector<float> &&bgColor):
			abstractWindowInfo(title, width, height, bgColor),
			jsonFileName(jsonName), renderArray(NULL), defaultCamera(NULL), rotateAxis(glm::vec3(0.5f, 1.0f, 0.0f)), degrees(50.0f), firstEnter(true) {}
		};

		window(
			const char *title = "", int width = 800.0, int height = 600.0,
			interactModel model = interactModel(defaultKeyboard, defaultScroll, defaultMouse),
			const char *jsonName = "default.json",
			vector<float> backgroundColor = {0.1f, 0.1f, 0.1f, 1.0f}
		);
		~window();

		// Parameter of window
		abstractWindowInfo *params;

		void frameCounter();

		void preRenderLoop();
		void postRenderLoop();

		void init();
		void start();

		void startDetach();

		void setPreRender(render preCallback);
		void setRender(render callback);

	};
}
