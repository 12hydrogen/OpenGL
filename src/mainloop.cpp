#include <interface.hpp>
#include <arrayLoader.hpp>
#include <camera.hpp>

#include <cmath>

#include <iostream>

#include <streambuf>

void framebufferCallback(GLFWwindow *window, GLint width, GLint height);

void keyboradInput(GLFWwindow *window, GLdouble delta);

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void mouseCallback(GLFWwindow* window, double xPos, double yPos);

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

double lastX;
double lastY;

bool firstEnter = true;

GLdouble screenWidth = 800;
GLdouble screenHeight = 600;

bool polygonModeLine = false;
GLfloat mixRate = 0.2f;
const GLfloat changingRate = 0.4f;

GLfloat deltaPerFrame = 0.0f;

camera baseCamera(glm::vec3(0.0f, 0.0f, 0.0f));

GLint main(GLint argc, GLchar **argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// Create window
	GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL Test Window", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		throw "Window creation failed.";
	}
	glfwMakeContextCurrent(window);

	// Check GLAD Loader
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		throw "GLAD loader failed.";
	}

	glViewport(0, 0, 800, 600);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set callback
	glfwSetFramebufferSizeCallback(window, framebufferCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	glEnable(GL_DEPTH_TEST);

	// Fetch config and gen everything
	objectArray *oArray;
	ifstream jsonFile("./object.json");
	string b;
	b.assign(istreambuf_iterator<char>(jsonFile), istreambuf_iterator<char>());
	json jsonEntity;
	try
	{
		jsonEntity = json::parse(b);
	}
	catch (json::parse_error &t)
	{
		cerr << t.what() << "\n";
		glfwTerminate();
		return 0;
	}
	try
	{
		oArray = new objectArray(jsonEntity);
	}
	catch (const char *s)
	{
		cerr << s << "\n";
		glfwTerminate();
		return 0;
	}

	// Unbind VAO
	glBindVertexArray(0);

	// Init frame rate counter
	GLuint frameCount = 0;
	GLdouble timestamp = glfwGetTime();
	GLdouble delta = 0.0;

	// Transform matrix
	glm::mat4 model(1.0f), view(1.0f), projection(1.0f);

	// Default shader program
	auto &shaderProgram = oArray[0]["box"].getShaderProgram();

	GLdouble lastFrameTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		// Input
		deltaPerFrame = glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();
		keyboradInput(window, deltaPerFrame);

		// Matrix operation
		model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		view = baseCamera.getLookAt();
		projection = baseCamera.getPerspective((float)(screenWidth / screenHeight));

		// Render operation
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render actual object
		// simpleProgram["mixRate"] = {mixRate};
		shaderProgram.useProgram();
		shaderProgram["mixRate"] = {mixRate};
		oArray->draw(model, view, projection);

		// Buffer swap
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Timer
		GLfloat frameRate = 60.0f;
		frameCount++;
		delta = glfwGetTime() - timestamp;
		if (delta > 5)
		{
			frameRate = frameCount / delta;
			timestamp += delta;
			frameCount = 0;
			cout << frameRate << "\n";
		}

		// Error
		//cout << glGetError() << "\n";
	}

	glfwTerminate();
	return 0;
}

void framebufferCallback(GLFWwindow *window, GLint width, GLint height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void keyboradInput(GLFWwindow *window, GLdouble delta)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_UP))
	{
		if (mixRate < 1.0f)
		{
			GLfloat deltaA = changingRate * delta;
			mixRate = (1.0f - deltaA > mixRate) ? mixRate + deltaA : 1.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN))
	{
		if (mixRate > 0.0f)
		{
			GLfloat deltaA = changingRate * delta;
			mixRate = (deltaA < mixRate) ? mixRate - deltaA : 0.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		baseCamera.move(FRONT, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_S))
	{
		baseCamera.move(BACK, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		baseCamera.move(LEFT, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_D))
	{
		baseCamera.move(RIGHT, delta);
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
	{
		if (polygonModeLine)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			polygonModeLine = false;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			polygonModeLine = true;
		}
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstEnter)
	{
		lastX = xPos;
		lastY = yPos;
		firstEnter = false;
	}

	baseCamera.view(xPos - lastX, lastY - yPos);
	lastX = xPos;
	lastY = yPos;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	baseCamera.zoomChange(yoffset);
}
