IF (NOT PROTOBUF_SOURCE)
	MESSAGE(FATAL_ERROR ¡°No specific proto buffer source!¡±)
ENDIF(NOT PROTOBUF_SOURCE)

INCLUDE_DIRECTORIES(${PROTOBUF_SOURCE} ${PROTOBUF_SOURCE}src .)
LINK_DIRECTORIES(/usr/local/lib)
 
SET(SRC_LIST main.cc plugin.cc plugin.pb.cc cpp_patch.cc cpp/cpp_enum.cc cpp/cpp_enum_field.cc cpp/cpp_extension.cc cpp/cpp_field.cc cpp/cpp_file.cc cpp/cpp_generator.cc cpp/cpp_helpers.cc cpp/cpp_message.cc cpp/cpp_message_field.cc cpp/cpp_primitive_field.cc cpp/cpp_service.cc cpp/cpp_string_field.cc)
 
ADD_EXECUTABLE(protoc-gen-luabind ${SRC_LIST})
 
TARGET_LINK_LIBRARIES(protoc-gen-luabind protoc protobuf)
 
INSTALL(TARGETS protoc-gen-luabind RUNTIME DESTINATION bin)