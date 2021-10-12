#include "ShaderUtility.h"

#include <bgfx/bgfx.h>
#include <bx/file.h>
#include <filesystem>
#include <fstream>

namespace bgfx
{
	struct Options
	{
		Options();

		void dump();

		char shaderType;
		std::string platform;
		std::string profile;

		std::string	inputFilePath;
		std::string	outputFilePath;

		std::vector<std::string> includeDirs;
		std::vector<std::string> defines;
		std::vector<std::string> dependencies;

		bool disasm;
		bool raw;
		bool preprocessOnly;
		bool depends;

		bool debugInformation;

		bool avoidFlowControl;
		bool noPreshader;
		bool partialPrecision;
		bool preferFlowControl;
		bool backwardsCompatibility;
		bool warningsAreErrors;
		bool keepIntermediate;

		bool optimize;
		uint32_t optimizationLevel;
	};

    extern bool compileShader(const char* _varying, const char* _comment, char* _shader, uint32_t _shaderLen, Options& _options, bx::FileWriter* _writer);
}

using namespace Marble;
using namespace Marble::GL;
namespace fs = std::filesystem;

static std::string curPath(fs::current_path().string());

static const char* varyingdefDefault =
R"(
vec4 v_color0    : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
vec4 v_color1    : COLOR1    = vec4(0.0, 1.0, 0.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_normal    : TEXCOORD1 = vec3(0.0, 1.0, 0.0);

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL0;
vec4 a_color0    : COLOR0;
vec4 a_color1    : COLOR1;
vec2 a_texcoord0 : TEXCOORD0;
)";

std::vector<char> ShaderUtility::compileShader(std::string shaderData, const ShaderCompileOptions& options)
{
    ProfileFunction();
    
    bgfx::Options compileOptions;
    compileOptions.shaderType = (char)options.shaderType;
    switch(bgfx::getRendererType())
    {
    case bgfx::RendererType::Direct3D9:
        switch (options.shaderType)
        {
        case ShaderType::Vertex:
            compileOptions.profile = "vs_3_0";
            break;
        case ShaderType::Fragment:
        case ShaderType::Compute:
            compileOptions.profile = "ps_3_0";
            break;
        }
    case bgfx::RendererType::Direct3D11:
        switch (options.shaderType)
        {
        case ShaderType::Vertex:
            compileOptions.profile = "vs_4_0";
            break;
        case ShaderType::Fragment:
            compileOptions.profile = "ps_4_0";
            break;
        case ShaderType::Compute:
            compileOptions.profile = "cs_5_0";
            break;
        }
    case bgfx::RendererType::Direct3D12:
        switch (options.shaderType)
        {
        case ShaderType::Vertex:
            compileOptions.profile = "vs_5_0";
            break;
        case ShaderType::Fragment:
            compileOptions.profile = "ps_5_0";
            break;
        case ShaderType::Compute:
            compileOptions.profile = "cs_5_0";
            break;
        }
    case bgfx::RendererType::OpenGL:
        switch (options.shaderType)
        {
        case ShaderType::Vertex:
        case ShaderType::Fragment:
            compileOptions.profile = "120";
            break;
        case ShaderType::Compute:
            compileOptions.profile = "430";
            break;
        }
    case bgfx::RendererType::Vulkan:
        compileOptions.profile = "spirv";
        break;
    case bgfx::RendererType::Gnm:
    case bgfx::RendererType::Metal:
    case bgfx::RendererType::OpenGLES:
    case bgfx::RendererType::Noop:
    default:
        compileOptions.profile = "unknown";
    };
    compileOptions.includeDirs = options.includeDirs;
    compileOptions.defines = options.defines;
    compileOptions.backwardsCompatibility = true;
    compileOptions.optimize = true;
    compileOptions.optimizationLevel = 3;

    struct : public bx::FileWriter {
        std::vector<char> data;
        int64_t seeker = 0;
        
        virtual int64_t seek(int64_t _offset = 0, bx::Whence::Enum _whence = bx::Whence::Current) override
        {
            switch (_whence)
            {
            case bx::Whence::Begin:
                this->seeker = _offset;
                break;
            case bx::Whence::Current:
                this->seeker += _offset;
                break;
            case bx::Whence::End:
                this->seeker = this->data.size() - _offset - 1;
                break;
            }
            return this->seeker;
        }
		virtual int32_t write(const void* _data, int32_t _size, bx::Error* _err) override
        {
            this->data.resize(this->seeker + _size);
            bx::memCopy(&this->data[this->seeker], _data, _size);
            return _size;
        }
    } writer;

    if (bgfx::compileShader(varyingdefDefault, "[JIT Compiled Shader]", &shaderData[0], shaderData.size(), compileOptions, &writer))
        return std::move(writer.data);
    else return { };
}
