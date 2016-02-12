#include "Shader.h"


//Pass the shader path to create the shader automatically
Shader::Shader(char *vertexPath, char *fragmentPath)
{
	//Initialize GLEW library and check if the initialization fails
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW \n";
		system("PAUSE");
	}
	//Create the shaders
	CreateShader(vertexPath, fragmentPath);
}

//Create a vertex and fragment shader  
void Shader::CreateShader(char *vertexPath, char *fragmentPath)
{
	//Create a handle for the program
	m_ProgramHandle = glCreateProgram();

	//Add vertex shader to the program
	AddShader(vertexPath, VERTEX_SHADER);

	//Add fragment shader to the program
	AddShader(fragmentPath, FRAGMENT_SHADER);

	//Link the shaders in the program and check for errors
	LinkShaders();
}

//Add a shader into the program  
void Shader::AddShader(char *path, ShaderType type)
{
	//Handle for the shader
	unsigned int shader;

	//Load the shader code from a file
	DataFile shaderCode = {};
	LoadFile(path, &shaderCode);
	char *code = (char*)shaderCode.Data;
	

	//Create a handle for the shader (according to the shader type
	if (type == VERTEX_SHADER)
	{
		shader = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == FRAGMENT_SHADER){
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	}

	//Put the shader code in the shader
	//NOTE(kai): glShaderSource can take an array of strings and can also specify 
	//			the length of each string	
	glShaderSource(shader			//The shader handle
				 , 1				//The number of strings in the array 
				 , &code			//The array of strings containing shader code
				 , NULL);			//The length of each string ,null means that the strings will be NULL terminated (default size)

	//Compile the shader and check for errors
	CompileShader(shader);

	//Attach the shader to the program
	glAttachShader(m_ProgramHandle, shader);

	//Delete the shader handle
	glDeleteShader(shader);

	UnloadFile(&shaderCode);
}

//Compile the shader we created
void Shader::CompileShader(unsigned int shader)
{
	//Compile the shader
	glCompileShader(shader);

	//Check for errors
	GetCompileError(shader);
}

//Link the shaders
void Shader::LinkShaders()
{
	//Link all shaders in one program
	glLinkProgram(m_ProgramHandle);

	//Check for link errors
	GetLinkError();
}

//Print out the errors in compiling the vertex and fragment shader
void Shader::GetCompileError(unsigned int shader)
{
	//The compile status
	int success;

	//The error log
	char log[512];

	//Get the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	//If success is NULL then compiling the shader failed
	if (!success)
	{
		//Store the log file
		glGetShaderInfoLog(shader, 512, NULL, log);

		//Print out the log file
		std::cout << "Failed to compile shader:\n" << log << std::endl;
		system("PAUSE");
	}
}

//Print out the errors in linking the vertex and fragment shader into a program
void Shader::GetLinkError()
{
	//The compile status
	int success;

	//The error log
	char log[512];

	//Get the link status
	glGetProgramiv(m_ProgramHandle, GL_LINK_STATUS, &success);

	//If success is NULL then linking the shader to the program failed
	if (!success)
	{
		//Store the log file
		glGetProgramInfoLog(m_ProgramHandle, 512, NULL, log);

		//Print out the log file
		std::cout << "Failed to link program:\n" << log << std::endl;
		system("PAUSE");
	}
}

//Activate the shader (We need to do this before we begin drawing)
void Shader::Activate()
{
	glUseProgram(m_ProgramHandle);
}