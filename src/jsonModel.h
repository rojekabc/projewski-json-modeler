#include <stdio.h>
#include <tools/streamutil.h>
#include <tools/tablica.h>

#define serializeJsonString(ostream, name, object) \
	goc_streamWriteText(ostream, "\""); \
	goc_streamWriteText(ostream, #name); \
	goc_streamWriteText(ostream, "\":"); \
	if ( object->name ) { \
		goc_streamWriteText(ostream, "\""); \
		goc_streamWriteText(ostream, object->name); \
		goc_streamWriteText(ostream, "\""); \
	} else { \
		goc_streamWriteText(ostream, "null"); \
	}

#define serializeJsonNumber(ostream, name, object)  \
	goc_streamWriteText(ostream, "\""); \
	goc_streamWriteText(ostream, #name); \
	goc_streamWriteText(ostream, "\":"); \
	{ char buf[12]; snprintf(buf, 12, "%d", object->name); goc_streamWriteText(ostream, buf); }

#define serializeJsonBoolean(ostream, name, object)  \
	goc_streamWriteText(ostream, "\""); \
	goc_streamWriteText(ostream, #name); \
	goc_streamWriteText(ostream, "\":"); \
	goc_streamWriteText(ostream, object->name ? "true" : "false")

#define serializeJsonArray(ostream, name, type, object) \
	goc_streamWriteText(ostream, "\""); \
	goc_streamWriteText(ostream, #name); \
	goc_streamWriteText(ostream, "\":["); \
	for (int i=0; i<object->name##Size; i++) {\
		serialize##type(stream, object->name[i]); \
		if ( i < object->name##Size-1 ) goc_streamWriteText(ostream, ","); \
	}\
	goc_streamWriteText(ostream, "]");

#define serializeJsonObject(ostream, name, type, object) \
	goc_streamWriteText(ostream, "\""); \
	goc_streamWriteText(ostream, #name); \
	goc_streamWriteText(ostream, "\":"); \
	if (object->name != NULL ) \
		serialize##type(stream, object->name); \
	else \
		goc_streamWriteText(ostream, "null");

#define deserializeJsonString(jRoot, name, object) {\
	json_t* jsonElement = json_object_get(jRoot, #name); \
	object->name = json_is_null(jsonElement) ? NULL : strdup(json_string_value(jsonElement)); \
}

#define deserializeJsonNumber(jRoot, name, object)  \
	object->name = (int)json_integer_value(json_object_get(jRoot, #name))

#define deserializeJsonBoolean(jRoot, name, object)  \
	object->name = (int)json_boolean_value(json_object_get(jRoot, #name))

#define deserializeJsonStringArray(jRoot, name, object) { \
		json_t* array = json_object_get(jRoot, #name); \
		if ( !json_is_null( array ) ) { \
			size_t arraySize = json_array_size( array ); \
			for (size_t i = 0; i < arraySize; i++ ) { \
				json_t* arrayElement = json_array_get( array, i ); \
				object->name = goc_tableAdd(object->name, &(object->name ## Size), sizeof(char*)); \
				object->name[i] = strdup(json_string_value(arrayElement); \
			} \
		} \
}

#define deserializeJsonNumberArray(jRoot, name, object) { \
		json_t* array = json_object_get(jRoot, #name); \
		if ( !json_is_null( array ) ) { \
			size_t arraySize = json_array_size( array ); \
			object->name = malloc(sizeof(int) * arraySize); \
			object->name ## Size = arraySize; \
			for (size_t i = 0; i < arraySize; i++ ) { \
				json_t* arrayElement = json_array_get( array, i ); \
				object->name[i] = json_integer_value( arrayElement ); \
			} \
		} \
}

#define deserializeJsonStringizeJsonBooleanArray(jRoot, name, object) { \
		json_t* array = json_object_get(jRoot, #name); \
		if ( !json_is_null( array ) ) { \
			size_t arraySize = json_array_size( array ); \
			object->name = malloc(sizeof(unsigned char) * arraySize); \
			object->name ## Size = arraySize; \
			for (size_t i = 0; i < arraySize; i++ ) { \
				json_t* arrayElement = json_array_get( array, i ); \
				object->name[i] = json_boolean_value( arrayElement ); \
			} \
		} \
}

#define deserializeJsonObjectArray(jRoot, name, type, object) { \
		json_t* array = json_object_get(jRoot, #name); \
		if ( !json_is_null( array ) ) { \
			size_t arraySize = json_array_size( array ); \
			for (size_t i = 0; i < arraySize; i++ ) { \
				json_t* arrayElement = json_array_get( array, i ); \
				object->name = goc_tableAdd(object->name, &(object->name ## Size), sizeof(void*)); \
				object->name[i] = deserialize ## type ## FromJson( arrayElement ); \
			} \
		} \
}

#define deserializeJsonObject(jRoot, name, type, object) \
	object->name = deserialize ## type ## FromJson(json_object_get(jRoot, #name))
