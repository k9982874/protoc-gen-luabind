// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)

#include <google/protobuf/compiler/plugin.h>

#include <iostream>
#include <set>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#else
#include <unistd.h>
#endif

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/plugin.pb.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <google/protobuf/io/printer.h>

#include "cpp/cpp_helpers.h"
#include "cpp/cpp_generator.h"
#include "cpp/cpp_file.h"
#include "cpp_patch.h"

namespace google {
namespace protobuf {
namespace compiler {

class GeneratorResponseContext : public GeneratorContext {
public:
	GeneratorResponseContext(CodeGeneratorResponse* response,
		const vector<const FileDescriptor*>& parsed_files)
		: response_(response),
		parsed_files_(parsed_files) {}
	virtual ~GeneratorResponseContext() {}

	// implements GeneratorContext --------------------------------------

	virtual io::ZeroCopyOutputStream* Open(const string& filename) {
		CodeGeneratorResponse::File* file = response_->add_file();
		file->set_name(filename);
		return new io::StringOutputStream(file->mutable_content());
	}

	virtual io::ZeroCopyOutputStream* OpenForInsert(
		const string& filename, const string& insertion_point) {
			CodeGeneratorResponse::File* file = response_->add_file();
			file->set_name(filename);
			file->set_insertion_point(insertion_point);
			return new io::StringOutputStream(file->mutable_content());
	}

	void ListParsedFiles(vector<const FileDescriptor*>* output) {
		*output = parsed_files_;
	}

	bool GenerateLuaBindCode(const string& parameter, string* error) {
		string dllexport_decl;

		vector<pair<string, string> > options;
		ParseGeneratorParameter(parameter, &options);

		for (int i = 0; i < options.size(); i++) {
			if (options[i].first == "dllexport_decl") {
				dllexport_decl = options[i].second;
			} else {
				return false;
			}
		}

		scoped_ptr<io::ZeroCopyOutputStream> output(this->Open("common.pb.h"));
		io::Printer printer(output.get(), '$');

		printer.Print(
			"#ifndef GOOGLE_PROTOBUF_LUABIND_COMMON_H__"
			"\n"
			"#define GOOGLE_PROTOBUF_LUABIND_COMMON_H__"
			"\n"
			"\n");

		string basename;
		for (int i = 0; i < parsed_files_.size(); i++) {
			basename = cpp::StripProto(parsed_files_[i]->name());
			basename.append(".pb.h");
			printer.Print("#include \"$filename$\"\n", "filename", basename);
		}
		
		printer.Print(
			"\n"
			"#include <google/protobuf/io/zero_copy_stream.h>\n"
			//"\n"
			//DEF_REPEATED_FIELD(Int8, ::google::protobuf::int8)
			//DEF_REPEATED_FIELD(Int16, ::google::protobuf::int16)
			//DEF_REPEATED_FIELD(Int32, ::google::protobuf::int32)
			//DEF_REPEATED_FIELD(Int64, ::google::protobuf::int64)
			//"\n"
			//DEF_REPEATED_FIELD(UInt8, ::google::protobuf::uint8)
			//DEF_REPEATED_FIELD(UInt16, ::google::protobuf::uint16)
			//DEF_REPEATED_FIELD(UInt32, ::google::protobuf::uint32)
			//DEF_REPEATED_FIELD(UInt64, ::google::protobuf::uint64)
			"\n"
			"namespace google {\n"
			"namespace protobuf {\n"
			"\n"
			"inline void InitLuaBindEnvironment(lua_State *L) {\n"
			"	module (L) [\n"
			"		class_<MessageLite>(\"MessageLite\")\n"
			"			.def(\"InitializationErrorString\", &MessageLite::InitializationErrorString)\n"
			"			.def(\"CheckTypeAndMergeFrom\", &MessageLite::CheckTypeAndMergeFrom)\n"
			"			.def(\"ParseFromCodedStream\", &MessageLite::ParseFromCodedStream)\n"
			"			.def(\"ParsePartialFromCodedStream\", &MessageLite::ParsePartialFromCodedStream)\n"
			"			.def(\"ParseFromZeroCopyStream\", &MessageLite::ParseFromZeroCopyStream)\n"
			"			.def(\"ParsePartialFromZeroCopyStream\", &MessageLite::ParsePartialFromZeroCopyStream)\n"
			"			.def(\"ParseFromBoundedZeroCopyStream\", &MessageLite::ParseFromBoundedZeroCopyStream)\n"
			"			.def(\"ParsePartialFromBoundedZeroCopyStream\", &MessageLite::ParsePartialFromBoundedZeroCopyStream)\n"
			"			.def(\"ParseFromString\", &MessageLite::ParseFromString)\n"
			"			.def(\"ParsePartialFromString\", &MessageLite::ParsePartialFromString)\n"
			"			.def(\"ParseFromArray\", &MessageLite::ParseFromArray)\n"
			"			.def(\"ParsePartialFromArray\", &MessageLite::ParsePartialFromArray)\n"
			"			.def(\"MergeFromCodedStream\", &MessageLite::MergeFromCodedStream)\n"
			"			.def(\"SerializeToCodedStream\", &MessageLite::SerializeToCodedStream)\n"
			"			.def(\"SerializeToZeroCopyStream\", &MessageLite::SerializeToZeroCopyStream)\n"
			"			.def(\"SerializePartialToZeroCopyStream\", &MessageLite::SerializePartialToZeroCopyStream)\n"
			"			.def(\"SerializeToString\", &MessageLite::SerializeToString)\n"
			"			.def(\"SerializePartialToString\", &MessageLite::SerializePartialToString)\n"
			"			.def(\"SerializeToArray\", &MessageLite::SerializeToArray)\n"
			"			.def(\"SerializePartialToArray\", &MessageLite::SerializePartialToArray)\n"
			"			.def(\"SerializeAsString\", &MessageLite::SerializeAsString)\n"
			"			.def(\"SerializePartialAsString\", &MessageLite::SerializePartialAsString)\n"
			"			.def(\"AppendToString\", &MessageLite::AppendToString)\n"
			"			.def(\"AppendPartialToString\", &MessageLite::AppendPartialToString)\n"
			"			.def(\"SerializeWithCachedSizesToArray\", &MessageLite::SerializeWithCachedSizesToArray),\n"
			"\n"
			"		class_<Message, MessageLite>(\"Message\")\n"
			"			.def(\"CopyFrom\", &Message::CopyFrom)\n"
			"			.def(\"MergeFrom\", &Message::MergeFrom)\n"
			"			.def(\"CheckInitialized\", &Message::CheckInitialized)\n"
			"			.def(\"FindInitializationErrors\", &Message::FindInitializationErrors)\n"
			"			.def(\"InitializationErrorString\", &Message::InitializationErrorString)\n"
			"			.def(\"DiscardUnknownFields\", &Message::DiscardUnknownFields)\n"
			"			.def(\"SpaceUsed\", &Message::SpaceUsed)\n"
			"			.def(\"DebugString\", &Message::DebugString)\n"
			"			.def(\"ShortDebugString\", &Message::ShortDebugString)\n"
			"			.def(\"Utf8DebugString\", &Message::Utf8DebugString)\n"
			"			.def(\"PrintDebugString\", &Message::PrintDebugString)\n"
			"			.def(\"SpaceUsed\", &Message::SpaceUsed)\n"
			"			.def(\"ParseFromFileDescriptor\", &Message::ParseFromFileDescriptor)\n"
			"			.def(\"ParsePartialFromFileDescriptor\", &Message::ParsePartialFromFileDescriptor)\n"
			"			.def(\"ParseFromIstream\", &Message::ParseFromIstream)\n"
			"			.def(\"ParsePartialFromIstream\", &Message::ParsePartialFromIstream)\n"
			"			.def(\"SerializeToFileDescriptor\", &Message::SerializeToFileDescriptor)\n"
			"			.def(\"SerializePartialToFileDescriptor\", &Message::SerializePartialToFileDescriptor)\n"
			"			.def(\"SerializeToOstream\", &Message::SerializeToOstream)\n"
			"			.def(\"SerializePartialToOstream\", &Message::SerializePartialToOstream)\n"
			"			.def(\"GetTypeName\", &Message::GetTypeName)\n"
			"			.def(\"Clear\", &Message::Clear)\n"
			"			.def(\"IsInitialized\", &Message::IsInitialized)\n"
			"			.def(\"CheckTypeAndMergeFrom\", &Message::CheckTypeAndMergeFrom)\n"
			"			.def(\"MergePartialFromCodedStream\", &Message::MergePartialFromCodedStream)\n"
			"			.def(\"ByteSize\", &Message::ByteSize)\n"
			"			.def(\"SerializeWithCachedSizes\", &Message::SerializeWithCachedSizes)\n"
			//",\n"
			//REG_REPEATED_FIELD(Int8, ::google::protobuf::int8)
			//",\n"
			//REG_REPEATED_FIELD(Int16, ::google::protobuf::int16)
			//",\n"
			//REG_REPEATED_FIELD(Int32, ::google::protobuf::int32)
			//",\n"
			//REG_REPEATED_FIELD(Int64, ::google::protobuf::int64)
			//",\n"
			//REG_REPEATED_FIELD(UInt8, ::google::protobuf::uint8)
			//",\n"
			//REG_REPEATED_FIELD(UInt16, ::google::protobuf::uint16)
			//",\n"
			//REG_REPEATED_FIELD(UInt32, ::google::protobuf::uint32)
			//",\n"
			//REG_REPEATED_FIELD(UInt64, ::google::protobuf::uint64)
			"	];\n"
			"\n");

		for (int i = 0; i < parsed_files_.size(); i++) {
			cpp::FileGenerator file_generator(parsed_files_[i], dllexport_decl);
			file_generator.GenerateLuaBindRegisterCode(&printer);
		}

		printer.Print(
			"}\n"
			"\n"
			"}  // namespace protobuf\n"
			"}  // namespace google\n"
			"\n"
			"#endif /* GOOGLE_PROTOBUF_LUABIND_COMMON_H__ */");

		return true;
	}

private:
	CodeGeneratorResponse* response_;
	const vector<const FileDescriptor*>& parsed_files_;
};

int PluginMain(int argc, char* argv[], const CodeGenerator* generator) {

	if (argc > 1) {
		cerr << argv[0] << ": Unknown option: " << argv[1] << endl;
		return 1;
	}

#ifdef _WIN32
	_setmode(STDIN_FILENO, _O_BINARY);
	_setmode(STDOUT_FILENO, _O_BINARY);
#endif

	CodeGeneratorRequest request;
	if (!request.ParseFromFileDescriptor(STDIN_FILENO)) {
		cerr << argv[0] << ": protoc sent unparseable request to plugin." << endl;
		return 1;
	}

	DescriptorPool pool;
	for (int i = 0; i < request.proto_file_size(); i++) {
		const FileDescriptor* file = pool.BuildFile(request.proto_file(i));
		if (file == NULL) {
			// BuildFile() already wrote an error message.
			return 1;
		}
	}

	vector<const FileDescriptor*> parsed_files;
	for (int i = 0; i < request.file_to_generate_size(); i++) {
		parsed_files.push_back(pool.FindFileByName(request.file_to_generate(i)));
		if (parsed_files.back() == NULL) {
			cerr << argv[0] << ": protoc asked plugin to generate a file but "
				"did not provide a descriptor for the file: "
				<< request.file_to_generate(i) << endl;
			return 1;
		}
	}

	CodeGeneratorResponse response;
	GeneratorResponseContext context(&response, parsed_files);

	for (int i = 0; i < parsed_files.size(); i++) {
		const FileDescriptor* file = parsed_files[i];

		string error;
		bool succeeded = generator->Generate(
			file, request.parameter(), &context, &error);

		if (!succeeded && error.empty()) {
			error = "Code generator returned false but provided no error "
				"description.";
		}
		if (!error.empty()) {
			response.set_error(file->name() + ": " + error);
			break;
		}
	}

	string error;
	context.GenerateLuaBindCode(request.parameter(), &error);

	if (!response.SerializeToFileDescriptor(STDOUT_FILENO)) {
		cerr << argv[0] << ": Error writing to stdout." << endl;
		return 1;
	}

	return 0;
}

}  // namespace compiler
}  // namespace protobuf
}  // namespace google
