#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#define WIDTH 1920
#define HEIGHT 1013
#define TITLE "ConvexHull"

#define MAX_POINTS 1000

GLFWwindow* window = nullptr;

double mouseX, mouseY;

struct Point
{
	float x, y;
};

struct Shader 
{
	unsigned int m_ID;

	Shader(const std::string& vertPath, const std::string& fragPath) 
	{
		std::string  vertCode = LoadFile(vertPath);
		std::string fragCode = LoadFile(fragPath);

		unsigned int vert = CompileShader(vertCode, true);
		unsigned int frag = CompileShader(fragCode, false);

		CreateShaderProgram(vert, frag);
	}

	~Shader() { glDeleteProgram(m_ID); }

	void Use() const { glUseProgram(m_ID); }

	void SetUniformMat4(const std::string& name, const glm::mat4& mat) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void SetUniformVec3(const std::string& name, const glm::vec3& vec) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3fv(location, 1, glm::value_ptr(vec));
	}

	void SetUniformFloat(const std::string& name, float value) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
	}

	void SetUniformInt(const std::string& name, int value) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1i(location, value);
	}

	void SetUniformVec4(const std::string& name, const glm::vec4& vec) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(vec));
	}

	void SetUniformVec2(const std::string& name, const glm::vec2& vec) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2fv(location, 1, glm::value_ptr(vec));
	}

	void SetUniformBool(const std::string& name, bool value) const
	{
		int location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1i(location, static_cast<int>(value));
	}

private:

	std::string LoadFile(const std::string& path) 
	{
		std::string code;
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open(path);
			std::stringstream ss;
			ss << file.rdbuf();
			code = ss.str();
		}
		catch (const std::ifstream::failure& e) 
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
		}

		return code;
	}

	unsigned int CompileShader(const std::string& code, bool isVert) 
	{
		int success;
		char infoLog[512];
		const char* shaderCode = code.c_str();

		unsigned int shader = glCreateShader(isVert ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
		glShaderSource(shader, 1, &shaderCode, nullptr);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "ERROR::SHADER::COMPILATION_FAILED: " << infoLog << std::endl;
		}

		return shader;
	}

	void CreateShaderProgram(unsigned int& vert, unsigned int& frag) 
	{
		m_ID = glCreateProgram();

		glAttachShader(m_ID, vert);
		glAttachShader(m_ID, frag);
		glLinkProgram(m_ID);

		int success;
		char infoLog[512];

		glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
		if (!success) 
		{
			glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED: " << infoLog << std::endl;
		}

		glDeleteShader(vert);
		glDeleteShader(frag);
	}
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

bool isEqual(Point& a, Point& b) 
{
	return (a.x == b.x) && (a.y == b.y);
}

bool isPointExisting(Point& p, const std::vector<Point>& points)
{
	for (Point point : points)
	{
		if (isEqual(p, point)) return true;
	}
	return false;
}

std::vector<Point> ConvexHull(const std::vector<Point>& points) 
{
	std::vector<Point> hull;

	for (int i = 0; i < points.size(); i++) 
	{
		Point p = points[i];

		for (int j = 0; j < points.size(); j++) 
		{
			Point q = points[j];

			if (!isEqual(p, q)) 
			{
				bool allLeft = true;
				bool allRight = true;

				for (int k = 0; k < points.size(); k++) 
				{
					Point r = points[k];

					if(!isEqual(p,r) && !isEqual(q,r))
					{
						Point pq = { q.x - p.x, q.y - p.y };
						Point pr = { r.x - p.x, r.y - p.y };

						float crossProduct = pq.x * pr.y - pq.y * pr.x;

						if (crossProduct <= 0.0f) allLeft = false;
						if (crossProduct > 0.0f) allRight = false;
					}
				}

				if (allLeft || allRight) 
				{
					if (!isPointExisting(p, hull)) hull.push_back(p);
					if (!isPointExisting(q, hull)) hull.push_back(q);
				}
			}
		}
	}

	Point centroid = { 0.0f, 0.0f };

	for (const auto& point : points) 
	{
		centroid.x += point.x;
		centroid.y += point.y;
	}

	centroid.x /= points.size();
	centroid.y /= points.size();

	for (int i = 0; i < hull.size(); i++) 
	{
		for (int j = 0; j < hull.size() - 1; j++) 
		{
			float a1 = atan2(hull[j].y - centroid.y, hull[j].x - centroid.x);
			float a2 = atan2(hull[j+1].y - centroid.y, hull[j + 1].x - centroid.x);

			if (a2 < a1) 
			{
				std::swap(hull[j], hull[j+1]);
			}
		}
	}

	return hull;
}

int main(void) 
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);

	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);


	std::vector<Point> points;

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * sizeof(Point), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	Shader shader("shaders/vert.glsl", "shaders/frag.glsl");

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	float screenSpaceX = 0.0f;
	float screenSpaceY = 0.0f;

	std::vector<Point> hull;

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		glfwGetCursorPos(window, &mouseX, &mouseY);

		screenSpaceX = (mouseX / WIDTH) * 2.0f - 1.0f;
		screenSpaceY = 1.0f - (mouseY / HEIGHT) * 2.0f;

		static bool isMouseHeld = false;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isMouseHeld) 
		{
			points.push_back({ screenSpaceX, screenSpaceY });
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(Point), points.data());

			hull = ConvexHull(points);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), hull.size() * sizeof(Point), hull.data());
			
			isMouseHeld = true;
		}

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			isMouseHeld = false;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();

		glBindVertexArray(VAO);

		shader.SetUniformVec3("color", glm::vec3(0.3f, 0.3f, 1.0f));
		glPointSize(15.0f);
		glDrawArrays(GL_POINTS, 0, points.size());

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			shader.SetUniformVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
			glDrawArrays(GL_LINE_LOOP, points.size(), hull.size());
		}
		
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();
	glfwDestroyWindow(window);

	return 0;
}