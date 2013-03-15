#ifndef GOOGLE_PROTOBUF_PATCH_CPP_H__
#define GOOGLE_PROTOBUF_PATCH_CPP_H__

#define INC_LUA_HEADERS \
	printer->Print("extern \"C\" {\n" \
				   "#include <lua.h>\n" \
				   "#include <lauxlib.h>\n" \
				   "#include <lualib.h>\n" \
				   "}\n" \
				   "\n" \
				   "#include <luabind/luabind.hpp>\n" \
				   "\n");

#define LUA_CONST_DEFINITION \
	printer->Print( \
		"#ifdef LUABIND_API\n" \
		"#define LUA_CONST_START( class, state ) { luabind::object g = luabind::globals(state); luabind::object table = g[#class];\n" \
		"#define LUA_CONST( class, name ) table[#name] = class::name;\n" \
		"#define LUA_CONST_END }\n" \
		"using namespace luabind;\n" \
		"#endif\n" \
		"\n");

#define CPP_PATCH_GENERATOR_DEFINITION \
	bool GenerateLuaBindCode(const vector<const FileDescriptor*>& parsed_files, const string& parameter, string* error);

#define CPP_PATCH_FILE_GENERATOR_DEFINITION \
	void GenerateLuaBindRegisterCode(io::Printer* printer); \
	void GenerateLuaBindCode(io::Printer* printer);

#define CPP_PATCH_ENUM_DEFINITION \
	void GenerateLuaBindCode(io::Printer* printer); \
	void GenerateLuaBindDefinition(io::Printer* printer); \
	void GenerateLuaBindMethods(io::Printer* printer);

#define CPP_PATCH_MESSAGE_DEFINITION \
	void GenerateLuaBindCode(io::Printer* printer); \
	void GenerateLuaBindDefinition(io::Printer* printer); \
	void GenerateLuaBindMethods(io::Printer* printer);

#define CPP_PATCH_FIELD_VOID_DEFINITION \
	virtual void GenerateLuaBindCode(io::Printer* printer) const = 0;

#define CPP_PATCH_FIELD_DEFINITION \
	virtual void GenerateLuaBindCode(io::Printer* printer) const;

#define DEF_REPEATED_FIELD(name, type) \
	"typedef ::google::protobuf::RepeatedField< "#type" > Repeated"#name";\n"

#define REG_REPEATED_FIELD(name, type) \
	"		class_< Repeated"#name" >(\"Repeated"#name"\")\n" \
	"			.def(constructor<>())\n" \
	"			.def(\"size\", &Repeated"#name"::size)\n" \
	"			.def(\"Get\", &Repeated"#name"::Get)\n" \
	"			.def(\"Mutable\", &Repeated"#name"::Mutable)\n" \
	"			.def(\"Set\", &Repeated"#name"::Set)\n" \
	"			.def(\"Add\", (void(Repeated"#name"::*)(const "#type" &))&Repeated"#name"::Add)\n" \
	"			.def(\"New\", ("#type" *(Repeated"#name"::*)())&Repeated"#name"::Add)\n" \
	"			.def(\"RemoveLast\", &Repeated"#name"::RemoveLast)\n" \
	"			.def(\"Clear\", &Repeated"#name"::Clear)\n" \
	"			.def(\"MergeFrom\", &Repeated"#name"::MergeFrom)\n" \
	"			.def(\"CopyFrom\", &Repeated"#name"::CopyFrom)\n" \
	"			.def(\"Reserve\", &Repeated"#name"::Reserve)\n" \
	"			.def(\"Truncate\", &Repeated"#name"::Truncate)\n" \
	"			.def(\"AddAlreadyReserved1\", (void(Repeated"#name"::*)(const "#type"&))&Repeated"#name"::AddAlreadyReserved)\n" \
	"			.def(\"AddAlreadyReserved2\", ("#type" *(Repeated"#name"::*)())&Repeated"#name"::AddAlreadyReserved)\n" \
	"			.def(\"Capacity\", &Repeated"#name"::Capacity)\n" \
	"			.def(\"mutable_data\", &Repeated"#name"::mutable_data)\n" \
	"			.def(\"data\", &Repeated"#name"::data)\n" \
	"			.def(\"Swap\", &Repeated"#name"::Swap)\n" \
	"			.def(\"SwapElements\", &Repeated"#name"::SwapElements)\n" \
	"			.def(\"begin\", ("#type" *(Repeated"#name"::*)())&Repeated"#name"::begin)\n" \
	"			.def(\"end\", ("#type" *(Repeated"#name"::*)())&Repeated"#name"::end)\n" \
	"			.def(\"const_begin\", (const "#type" *(Repeated"#name"::*)() const)&Repeated"#name"::begin)\n" \
	"			.def(\"const_end\", (const "#type" *(Repeated"#name"::*)() const)&Repeated"#name"::end)\n" \
	"			.def(\"SpaceUsedExcludingSelf\", &Repeated"#name"::SpaceUsedExcludingSelf)\n"

#endif // #define GOOGLE_PROTOBUF_PATCH_CPP_H__