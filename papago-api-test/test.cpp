#include "pch.h"
#include "parser.hpp"
#include "ishader.hpp"

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

std::string vertex_source =
"#version 450\n"
"void main(){\n"
"  gl_Position = vec4(0.0, 0.0, 0.0, 1.0);"
"}\n";

TEST(ParserTests, VertexShader) {
	Parser p = Parser("C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe");

	EXPECT_FALSE(p.compileVertexShader(vertex_source, "main") == nullptr);
	EXPECT_THROW(p.compileVertexShader("", "main"), std::runtime_error);
}

std::string fragment_source =
"#version 450\n"
"void main(){\n"
"  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);"
"}\n";

TEST(ParserTests, FragmentShader) {
	Parser p = Parser("C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe");

	EXPECT_FALSE(p.compileFragmentShader(vertex_source, "main") == nullptr);
	EXPECT_THROW(p.compileFragmentShader("", "main"), std::runtime_error);
}
