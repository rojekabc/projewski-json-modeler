#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <tools/array.h>
#include <tools/mystr.h>
#include "jsonModeler.h"

FILE* headerFile;
FILE* codeFile;

enum JsonType {string, boolean, array, number, object};

typedef struct JsonElement {
	enum JsonType type;
	char* name;
	char* containedModel;
} JsonElement;

JsonElement* END_FIELD = NULL;


void JSON_MODEL_INT(const char* modelName, ...) {
	va_list vaList;
	unsigned char wasPrevious=0;
	JsonElement* element;
	va_start(vaList, modelName);
	// --- create the header file ---
	// create structures
	fprintf(headerFile, "typedef struct %s {\n", modelName);
	while ( (element = va_arg(vaList, JsonElement*)) != END_FIELD ) {
		switch (element->type) {
			case string:
				fprintf(headerFile, "\tchar* %s;\n", element->name);
				break;
			case number:
				fprintf(headerFile, "\tint %s;\n", element->name);
				break;
			case boolean:
				fprintf(headerFile, "\tunsigned char %s;\n", element->name);
				break;
			case array: {
				if ( goc_stringEquals( element->containedModel, "string" )) {
					fprintf(headerFile, "\tchar** %s;\n", element->name);
					fprintf(headerFile, "\tint %sSize;\n", element->name);
				} else if ( goc_stringEquals( element->containedModel, "number" )) {
					fprintf(headerFile, "\tint* %s;\n", element->name);
					fprintf(headerFile, "\tint %sSize;\n", element->name);
				} else if ( goc_stringEquals( element->containedModel, "boolean" )) {
					fprintf(headerFile, "\tunsigned char* %s;\n", element->name);
					fprintf(headerFile, "\tint %sSize;\n", element->name);
				} else {
					fprintf(headerFile, "\tstruct %s** %s;\n", element->containedModel, element->name);
					fprintf(headerFile, "\tint %sSize;\n", element->name);
				}
				break;
				}
			case object:
				fprintf(headerFile, "\tstruct %s* %s;\n", element->containedModel, element->name);
				break;
		}
	}
	fprintf(headerFile, "} %s;\n", modelName);

	// --- create the body file ---
	// malloc function
	fprintf(headerFile, "struct %s* malloc%s();\n", modelName, modelName);
	fprintf(codeFile, "struct %s* malloc%s() {\n", modelName, modelName);
	fprintf(codeFile, "\tALLOC(%s, result);\n", modelName);
	fprintf(codeFile, "\treturn result;\n");
	fprintf(codeFile, "}\n");

	// free function
	fprintf(headerFile, "void free%s(struct %s* object);\n", modelName, modelName);
	fprintf(codeFile, "void free%s(struct %s* object) {\n", modelName, modelName);
	fprintf(codeFile, "\tif (object) {\n");
	va_start(vaList, modelName);
	while ( (element = va_arg(vaList, JsonElement*)) != END_FIELD ) {
		switch (element->type) {
			case string:
				fprintf(codeFile, "\t\tif (object->%s) free(object->%s);\n", element->name, element->name);
				break;
			case number:
				break;
			case boolean:
				break;
			case array:
				if ( goc_stringEquals(element->containedModel, "string" )) {
					fprintf(codeFile, "\t\tfor (int i=0; i<object->%sSize; i++) {\n", element->name);
					fprintf(codeFile, "\t\t\tfree(object->%s[i]);\n", element->name);
					fprintf(codeFile, "\t\t}\n");
					fprintf(codeFile, "\t\tobject->%s = goc_tableClear(object->%s, &object->%sSize);\n",
							element->name, element->name, element->name);
				} else if ( goc_stringEquals(element->containedModel, "number" )) {
					fprintf(codeFile, "\t\tfree(object->%s);\n", element->name);
					fprintf(codeFile, "\t\tobject->%sSize = 0;\n", element->name);
				} else if ( goc_stringEquals(element->containedModel, "boolean" )) {
					fprintf(codeFile, "\t\tfree(object->%s);\n", element->name);
					fprintf(codeFile, "\t\tobject->%sSize = 0;\n", element->name);
				} else {
					fprintf(codeFile, "\t\tfor (int i=0; i<object->%sSize; i++) {\n", element->name);
					fprintf(codeFile, "\t\t\tfree%s(object->%s[i]);\n", element->containedModel, element->name);
					fprintf(codeFile, "\t\t}\n");
					fprintf(codeFile, "\t\tobject->%s = goc_tableClear(object->%s, &object->%sSize);\n",
							element->name, element->name, element->name);
				}
				break;

			case object:
				fprintf(codeFile, "\t\tfree%s(object->%s);\n", element->containedModel, element->name);
				break;
		}
	}
	fprintf(codeFile, "\t\tfree(object);\n");
	fprintf(codeFile, "\t}\n");
	fprintf(codeFile, "}\n");

	// serialize function
	fprintf(headerFile, "void serialize%s(GOC_OStream* stream, struct %s* object);\n", modelName, modelName);
	fprintf(codeFile, "void serialize%s(GOC_OStream* stream, struct %s* object) {\n", modelName, modelName);
	fprintf(codeFile, "\tgoc_streamWriteText(stream, \"{\");\n");
	va_start(vaList, modelName);
	while ( (element = va_arg(vaList, JsonElement*)) != END_FIELD ) {
		if ( wasPrevious ) {
			fprintf(codeFile, "\tgoc_streamWriteText(stream, \",\");\n");
		}
		switch (element->type) {
			case string:
				fprintf(codeFile, "\tserializeJsonString(stream, %s, object );\n", element->name);
				break;
			case number:
				fprintf(codeFile, "\tserializeJsonNumber(stream, %s, object );\n", element->name);
				break;
			case boolean:
				fprintf(codeFile, "\tserializeJsonBoolean(stream, %s, object );\n", element->name);
				break;
			case array:
				fprintf(codeFile, "\tserializeJsonArray(stream, %s, %s, object);\n", element->name, element->containedModel);
				break;
			case object:
				fprintf(codeFile, "\tserializeJsonObject(stream, %s, %s, object);\n", element->name, element->containedModel);
				break;
		}
		wasPrevious = 1;
	}
	fprintf(codeFile, "\tgoc_streamWriteText(stream, \"}\");\n");
	fprintf(codeFile, "}\n");

	// deserialize from JSON function
	fprintf(headerFile, "struct %s* deserialize%sFromJson(json_t* jsonRoot);\n", modelName, modelName);
	fprintf(codeFile, "struct %s* deserialize%sFromJson(json_t* jsonRoot) {\n", modelName, modelName);
	fprintf(codeFile, "\tif (json_is_null(jsonRoot)) return NULL;\n");
	fprintf(codeFile, "\tALLOC(%s, object);\n", modelName);
	va_start(vaList, modelName);
	while ( (element = va_arg(vaList, JsonElement*)) != END_FIELD ) {
		switch (element->type) {
			case string:
				fprintf(codeFile, "\tdeserializeJsonString(jsonRoot, %s, object);\n", element->name);
				break;
			case number:
				fprintf(codeFile, "\tdeserializeJsonNumber(jsonRoot, %s, object);\n", element->name);
				break;
			case boolean:
				fprintf(codeFile, "\tdeserializeJsonBoolean(jsonRoot, %s, object);\n", element->name);
				break;
			case array:
				if ( goc_stringEquals( element->containedModel, "string" )) {
					fprintf(codeFile, "\tdeserializeJsonStringArray(jsonRoot, %s, object);\n", element->name);
				} else if ( goc_stringEquals( element->containedModel, "number" )) {
					fprintf(codeFile, "\tdeserializeJsonNumberArray(jsonRoot, %s, object);\n", element->name);
				} else if ( goc_stringEquals( element->containedModel, "boolean" )) {
					fprintf(codeFile, "\tdeserializeJsonBooleanArray(jsonRoot, %s, object);\n", element->name);
				} else {
					fprintf(codeFile, "\tdeserializeJsonObjectArray(jsonRoot, %s, %s, object);\n", element->name, element->containedModel);
				}
				break;
			case object:
				fprintf(codeFile, "\tdeserializeJsonObject(jsonRoot, %s, %s, object);\n", element->name, element->containedModel);
				break;
		}
	}
	fprintf(codeFile, "\treturn object;\n");
	fprintf(codeFile, "}\n");

	// deserialize from stream function
	fprintf(headerFile, "struct %s* deserialize%s(GOC_IStream* stream);\n", modelName, modelName);
	fprintf(codeFile, "struct %s* deserialize%s(GOC_IStream* stream) {\n", modelName, modelName);
	fprintf(codeFile, "\tjson_error_t jsonError;\n");
	fprintf(codeFile, "\tchar* text = goc_streamReadAsText(stream);\n");
	// fprintf(codeFile, "\tfprintf(stdout, \"I read %%s\\n\", text);\n");
	fprintf(codeFile, "\tjson_t* jsonRoot = json_loads(text, 0, &jsonError);\n");
	fprintf(codeFile, "\tfree(text);\n");
	fprintf(codeFile, "\tif (!jsonRoot) {\n");
	fprintf(codeFile, "\t\tfprintf(stderr, \"Error reading JSON on line %%d. Error %%s\\n\", jsonError.line, jsonError.text);\n");
	fprintf(codeFile, "\t\treturn NULL;\n");
	fprintf(codeFile, "\t}\n");
	fprintf(codeFile, "\treturn deserialize%sFromJson(jsonRoot);\n", modelName);
	fprintf(codeFile, "}\n");
}

JsonElement* createField(const char* name, enum JsonType type) {
	JsonElement* result = malloc(sizeof(struct JsonElement));
	memset(result, 0, sizeof(struct JsonElement));
	result->name = strdup(name);
	result->type = type;
	return result;
}

JsonElement* createContainer(const char* modelName, const char *name, enum JsonType type) {
	JsonElement* result = createField(name, type);
	result->containedModel = strdup(modelName);
	return result;
}

#define JSON_MODEL(modelName, fields...) JSON_MODEL_INT(#modelName, fields, END_FIELD)
#define JSON_STRING(name) createField(#name, string)
#define JSON_BOOLEAN(name) createField(#name, boolean)
#define JSON_NUMBER(name) createField(#name, number)
#define JSON_ARRAY(modelName, name) createContainer(#modelName, #name, array)
#define JSON_OBJECT(modelName, name) createContainer(#modelName, #name, object)

void printInformation(FILE* file) {
	fprintf(file, "/**         WARNING!        */\n");
	fprintf(file, "/** THIS FILE IS GENERATED! */\n");
	fprintf(file, "/** PLEASE,  NOT MODIFY IT! */\n");
}

int main() {
	headerFile = fopen(OUT_HEADER_FILENAME, "wb");
	codeFile = fopen(OUT_CODE_FILENAME, "wb");

	// append headers to files
	printInformation(headerFile);
	printInformation(codeFile);

	fprintf(headerFile, "#include <tools/istream.h>\n");
	fprintf(headerFile, "#include <tools/ostream.h>\n");
	fprintf(headerFile, "#include <tools/streamutil.h>\n");
	fprintf(headerFile, "#include <jansson.h>\n");

	fprintf(codeFile, "#include <string.h>\n");
	fprintf(codeFile, "#include <tools/malloc.h>\n");
	fprintf(codeFile, "#include <jsonModeler/jsonModel.h>\n");
	fprintf(codeFile, "#include \"%s\"\n", OUT_HEADER_FILENAME);

	//object modeling. TODO: array model []
	
#include MODEL_FILENAME

	fclose(headerFile);
	fclose(codeFile);
}
