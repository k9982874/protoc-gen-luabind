#include "plugin.h"
#include "cpp/cpp_generator.h"

int main(int argc, char* argv[]) {
	google::protobuf::compiler::cpp::CppGenerator cpp_generator;
	return google::protobuf::compiler::PluginMain(argc, argv, &cpp_generator);
}

