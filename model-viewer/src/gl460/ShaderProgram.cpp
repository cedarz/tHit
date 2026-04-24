#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram() {
    id_ = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
    if (!id_) return;
    glDeleteProgram(id_);
}

void ShaderProgram::attachShader(Shader& shader) {
    glAttachShader(id_, shader.id());
}

void ShaderProgram::attachShaders(const std::vector<Shader>& shaders) {
    for (auto shader : shaders)
    {
        attachShader(shader);
    }
}

bool ShaderProgram::link() {
    glLinkProgram(id_);
    return checkLink();
}

bool ShaderProgram::checkLink() {
    GLint success, logLength;
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);

    /* Error or warning message. The length is reported including the null
       terminator and the string implicitly has a storage for that, thus
       specify one byte less. */
    if (logLength > 1) {
        std::string infoLog;
        infoLog.reserve(std::size_t(logLength));
        glGetProgramInfoLog(id_, logLength, nullptr, infoLog.data());
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << "PROGRAM" << "\n" 
            << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
    return success;
}

void ShaderProgram::use()
{
    glUseProgram(id_);
}

void ShaderProgram::unUse()
{
    glUseProgram(0);
}

void ShaderProgram::load(const std::vector<ShaderStageDesc>& desc) {

    //std::vector<Shader> shaders;
    for (auto d : desc)
    {
        Shader shader(d.stage);
        shader.SetFile(d.code_file);
        shader.Compile();
        attachShader(shader);
    }
    //attachShader(shaders);
    link();
}
