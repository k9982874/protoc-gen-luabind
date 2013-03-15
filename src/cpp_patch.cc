#include <algorithm>
#include <google/protobuf/stubs/hash.h>
#include <map>
#include <utility>
#include <vector>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/descriptor.pb.h>

#include <google/protobuf/io/zero_copy_stream.h>

#include "cpp/cpp_generator.h"
#include "cpp/cpp_file.h"
#include "cpp/cpp_message.h"
#include "cpp/cpp_field.h"
#include "cpp/cpp_enum.h"
#include "cpp/cpp_extension.h"
#include "cpp/cpp_helpers.h"

#include "cpp/cpp_string_field.h"
#include "cpp/cpp_enum_field.h"
#include "cpp/cpp_message_field.h"
#include "cpp/cpp_primitive_field.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

// ----------------------------------------------------
// FileGenerator
// begin
void FileGenerator::GenerateLuaBindRegisterCode(io::Printer* printer) {
	if ((file_->message_type_count() > 0) || (file_->enum_type_count() > 0)) {
		printer->Print("	");
		for (int i = 0; i < package_parts_.size(); i++) {
			printer->Print("$part$::", "part", package_parts_[i]);
		}
		printer->Print("$filename$_RegisterToLua(L);\n", "filename", cpp::StripProto(file_->name()));
	}
}

void FileGenerator::GenerateLuaBindCode(io::Printer* printer) {
	printer->Print(
		"#ifdef LUABIND_API\n"
		"inline void $filename$_RegisterToLua(lua_State *L) {\n", "filename",
		cpp::StripProto(file_->name()));

	for (int i = 0; i < file_->enum_type_count(); i++) {
		enum_generators_[i]->GenerateLuaBindCode(printer);
	}
	for (int i = 0; i < file_->message_type_count(); i++) {
		message_generators_[i]->GenerateLuaBindCode(printer);
	}

	printer->Print(
		"}\n"
		"#endif"
		"\n");
}
// end

// ----------------------------------------------------
// MessageGenerator
// begin
void MessageGenerator::GenerateLuaBindCode(io::Printer* printer) {
	printer->Print("	$classname$::RegisterToLua(L);\n", "classname", classname_);

	for (int i = 0; i < descriptor_->enum_type_count(); i++) {
		enum_generators_[i]->GenerateLuaBindCode(printer);
	}

	for (int i = 0; i < descriptor_->nested_type_count(); i++) {
		nested_generators_[i]->GenerateLuaBindCode(printer);
	}
}

void MessageGenerator::GenerateLuaBindDefinition(io::Printer* printer) {
	printer->Outdent();
	printer->Print("\n"
				   "public:\n");
	printer->Indent();
	printer->Print("#ifdef LUABIND_API\n"
				   "void import(luabind::object table);\n"
				   "static void RegisterToLua(lua_State* L);\n"
				   "#endif\n"
				   "\n");
}

void MessageGenerator::GenerateLuaBindMethods(io::Printer* printer) {
	printer->Print(
		"#ifdef LUABIND_API\n"
		"void $classname$::import(luabind::object table) {\n"
		"	luabind::object v;\n", "classname", classname_);

	for (int i = 0; i < descriptor_->field_count(); i++) {
		const FieldDescriptor* field = descriptor_->field(i);

		if (field->type() != FieldDescriptor::TYPE_MESSAGE) {
			map<string, string> vars;
			SetCommonFieldVariables(field, &vars);

			if (field->type() == FieldDescriptor::TYPE_ENUM) {
				vars["type"] = ClassName(field->enum_type(), true);
			} else {
				vars["type"] = PrimitiveTypeName(field->cpp_type());
			}

			if (field->is_repeated()) {
				printer->Print(vars,
					"	v = table[\"$name$\"];\n"
					"	if (v) {\n"
					"		for (luabind::iterator iter(v), end; iter != end; ++iter) { \n"
					"			this->add_$name$(luabind::object_cast< $type$ >(v));\n"
					"		}\n"
					"	}\n");
			} else {
				printer->Print(vars,
					"	v = table[\"$name$\"];\n"
					"	if (v) { this->set_$name$(luabind::object_cast< $type$ >(v)); }\n");
			}
		}
	}

	printer->Print(
		"}\n"
		"\n");

	printer->Print(
		"void $classname$::RegisterToLua(lua_State* L) {\n"
		"	module(L) [\n"
		"		class_<$classname$, ::google::protobuf::Message>(\"$classname$\")\n"
		"			.scope [\n"
		"				def(\"default_instance\", &$classname$::default_instance)\n"
		"			]\n"
		"\n"
		"			.def(constructor<>())\n"
		"			.def(constructor<const $classname$ &>())\n"
		"\n"
		"			.def(\"Swap\", &$classname$::Swap)\n"
		"\n"
		"			.def(\"New\", &$classname$::New)\n",
		"classname", classname_);

	if (HasDescriptorMethods(descriptor_->file())) {
		printer->Print(
			"\n"
			"			.def(\"CopyFromMessage\", (void($classname$::*)(const ::google::protobuf::Message&))&$classname$::CopyFrom)\n"
			"			.def(\"MergeFromMessage\", (void($classname$::*)(const ::google::protobuf::Message&))&$classname$::MergeFrom)\n",
			"classname", classname_);
	} else {
		printer->Print(
			"\n"
			"			.def(\"CheckTypeAndMergeFrom\", &$classname$::CheckTypeAndMergeFrom)\n",
			"classname", classname_);
	}

	printer->Print(
		"\n"
		"			.def(\"CopyFrom\", (void($classname$::*)(const $classname$&))&$classname$::CopyFrom)\n"
		"			.def(\"MergeFrom\", (void($classname$::*)(const $classname$&))&$classname$::MergeFrom)\n"
		"			.def(\"Clear\", &$classname$::Clear)\n"
		"			.def(\"IsInitialized\", &$classname$::IsInitialized)\n"
		"\n"
		"			.def(\"ByteSize\", &$classname$::ByteSize)\n"
		"			.def(\"MergePartialFromCodedStream\", &$classname$::MergePartialFromCodedStream)\n"
		"			.def(\"SerializeWithCachedSizes\", &$classname$::SerializeWithCachedSizes)\n"
		"\n"
		"			.def(\"import\", &$classname$::import)\n",
		"classname", classname_);

	if (HasFastArraySerialization(descriptor_->file())) {
		printer->Print(
			"			.def(\"SerializeWithCachedSizesToArray\", &$classname$::SerializeWithCachedSizesToArray)\n",
			"classname", classname_);
	}

	printer->Print(
		"\n"
		"			.def(\"GetCachedSize\", &$classname$::GetCachedSize)\n",
		"classname", classname_);

	if (HasDescriptorMethods(descriptor_->file())) {
		printer->Print(
			"\n"
			"			.def(\"Metadata\", &$classname$::GetMetadata)\n"
			"\n",
			"classname", classname_);
	} else {
		printer->Print(
			"\n"
			"			.def(\"GetTypeName\", &$classname$::GetTypeName)\n"
			"\n",
			"classname", classname_);
	}

	for (int i = 0; i < descriptor_->field_count(); i++) {
		const FieldDescriptor* field = descriptor_->field(i);

		map<string, string> vars;
		SetCommonFieldVariables(field, &vars);
		vars["constant_name"] = FieldConstantName(field);

		if (field->is_repeated()) {
			printer->Print(vars,
				"			.def(\"$name$_size\", &$classname$::$name$_size)\n");
		} else {
			printer->Print(vars,
				"			.def(\"has_$name$\", &$classname$::has_$name$)\n");
		}

		printer->Print(vars,
			"			.def(\"clear_$name$\", &$classname$::clear_$name$)\n"
			"\n");

		field_generators_.get(field).GenerateLuaBindCode(printer);
	}

	printer->Print(
		"	];\n"
		"\n");

	printer->Print(
		"	LUA_CONST_START($classname$, L)\n",
		"classname", classname_);
	for (int i = 0; i < descriptor_->field_count(); i++) {
		const FieldDescriptor* field = descriptor_->field(i);

		map<string, string> vars;
		SetCommonFieldVariables(field, &vars);
		vars["constant_name"] = FieldConstantName(field);

		printer->Print(vars,
			"	LUA_CONST($classname$, $constant_name$)\n");
	}
	printer->Print(
		"	LUA_CONST_END\n");

	printer->Print(
		"}\n"
		"#endif\n"
		"\n");
}
// end

// ----------------------------------------------------
// EnumGenerator
// begin
void EnumGenerator::GenerateLuaBindDefinition(io::Printer* printer) {
	map<string, string> vars;
	vars["classname"] = classname_;
	vars["short_name"] = descriptor_->name();

    const EnumValueDescriptor* min_value = descriptor_->value(0);
	const EnumValueDescriptor* max_value = descriptor_->value(0);

	printer->Print(vars,
		"\n"
		"#ifdef LUABIND_API\n"
		"inline void $classname$_RegisterToLua(lua_State* L) { \n"
		"	module(L) [\n"
		"		class_<$classname$>(\"$classname$\")\n"
		"			.enum_(\"constants\") [\n"

		);

	for (int i = 0; i < descriptor_->value_count(); i++) {
		vars["name"] = descriptor_->value(i)->name();
		vars["number"] = SimpleItoa(descriptor_->value(i)->number());
		vars["prefix"] = (descriptor_->containing_type() == NULL) ? "" : classname_ + "_";

		if (i > 0) printer->Print(",\n");
			printer->Print(vars,
				"				value(\"$name$\", $number$)");

		if (descriptor_->value(i)->number() < min_value->number()) {
			min_value = descriptor_->value(i);
		}
		if (descriptor_->value(i)->number() > max_value->number()) {
			max_value = descriptor_->value(i);
		}
	}
	printer->Print(
		"\n"
		"			],\n");

	vars["min_name"] = min_value->name();
	vars["max_name"] = max_value->name();

	printer->Print(vars,
		"\n"
		"		def(\"$classname$_IsValid\", $classname$_IsValid)");

	if (HasDescriptorMethods(descriptor_->file())) {
		printer->Print(vars,
			",\n"
			"\n"
			//"		def(\"$classname$_descriptor\", $classname$_descriptor),\n"
			"		def(\"$classname$_Name\", $classname$_Name),\n"
			"		def(\"$classname$_Parse\", $classname$_Parse)\n");
	} else {
		printer->Print(vars,
			",\n");
	}

	printer->Print(vars,
		"	];\n"
		"\n"
		"	luabind::object global = luabind::globals(L);\n"
		"	global[\"$prefix$MIN\"] = $prefix$$short_name$_MIN;\n"
		"	global[\"$prefix$MAX\"] = $prefix$$short_name$_MAX;\n"
		"	global[\"$prefix$ARRAYSIZE\"] = $prefix$$short_name$_ARRAYSIZE;\n"
		"}\n"
		"#endif\n"
		"\n");
}

void EnumGenerator::GenerateLuaBindMethods(io::Printer* printer) {
}

void EnumGenerator::GenerateLuaBindCode(io::Printer* printer) {
	printer->Print("	$classname$_RegisterToLua(L);\n", "classname", classname_);
}
// end

// ----------------------------------------------------
// MessageFieldGenerator
void MessageFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"$name$\", &$classname$::$name$)\n"
		"			.def(\"mutable_$name$\", &$classname$::mutable_$name$)\n"
		"			.def(\"release_$name$\", &$classname$::release_$name$)\n"
		"\n");
}

// ----------------------------------------------------
// RepeatedMessageFieldGenerator
void RepeatedMessageFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"get_$name$\", (const $type$& ($classname$::*)(int) const$deprecation$)&$classname$::$name$)\n"
		"			.def(\"get_mutable_$name$\", ($type$* ($classname$::*)(int)$deprecation$)&$classname$::mutable_$name$)\n"
		"			.def(\"add_$name$\", &$classname$::add_$name$)\n"
		"\n"
		"			.def(\"$name$\", (const ::google::protobuf::RepeatedPtrField< $type$ >& ($classname$::*)() const$deprecation$)&$classname$::$name$)\n"
		"			.def(\"mutable_$name$\", (::google::protobuf::RepeatedPtrField< $type$ >* ($classname$::*)()$deprecation$)&$classname$::mutable_$name$)\n"
		"\n");
}

// ----------------------------------------------------
// StringFieldGenerator
void StringFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	if (descriptor_->options().ctype() == FieldOptions::STRING) {
		printer->Print(variables_,
			"			.def(\"$name$\", &$classname$::$name$)\n"
			"			.def(\"set_$name$\", (void ($classname$::*)(const ::std::string&)$deprecation$)&$classname$::set_$name$)\n"
			"			.def(\"mutable_$name$\", &$classname$::mutable_$name$)\n"
			"			.def(\"release_$name$\", &$classname$::release_$name$)\n"
			"\n");
	}
}

// ----------------------------------------------------
// RepeatedStringFieldGenerator
void RepeatedStringFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"get_$name$\", (const ::std::string& ($classname$::*)(int) const$deprecation$)&$classname$::$name$)\n"
		"			.def(\"set_$name$\", (void ($classname$::*)(int, const ::std::string&)$deprecation$)&$classname$::set_$name$)\n"
		"			.def(\"add_$name$\", (void ($classname$::*)(const ::std::string&)$deprecation$)&$classname$::add_$name$)\n"
		"\n");
}

// ----------------------------------------------------
// EnumFieldGenerator
void EnumFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"$name$\", &$classname$::$name$)\n"
		"			.def(\"set_$name$\", &$classname$::set_$name$)\n"
		"\n");
}

// ----------------------------------------------------
// RepeatedEnumFieldGenerator
void RepeatedEnumFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	assert(false);
}

// ----------------------------------------------------
// PrimitiveFieldGenerator
void PrimitiveFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"$name$\", &$classname$::$name$)\n"
		"			.def(\"set_$name$\", &$classname$::set_$name$)\n"
		"\n");
}

// ----------------------------------------------------
// RepeatedPrimitiveFieldGenerator
void RepeatedPrimitiveFieldGenerator::GenerateLuaBindCode(io::Printer* printer) const {
	printer->Print(variables_,
		"			.def(\"get_$name$\", ($type$ ($classname$::*)(int) const$deprecation$)&$classname$::$name$)\n"
		"			.def(\"set_$name$\", &$classname$::set_$name$)\n"
		"			.def(\"add_$name$\", &$classname$::add_$name$)\n"
		"			.def(\"$name$\", (const ::google::protobuf::RepeatedField< $type$ >& ($classname$::*)() const$deprecation$)&$classname$::$name$)\n"
		"			.def(\"mutable_$name$\", &$classname$::mutable_$name$)\n"
		"\n");
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf

}  // namespace google
