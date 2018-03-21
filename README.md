# projewski-json-modeler
The application, which allows create and use JSON structures for the C language.

## Description
The application is created in C language as a JSON quick modeling tool, which allows generate header and code file with model structures and util function as malloc/free/serialize/deserialize.
Tool uses:
* jansson library (https://github.com/akheron/jansson) to deserialize JSON objects.
* projewski-libtool from okienkoc project (https://github.com/rojekabc/okienkoc/tree/master/projewski/libtool)

User of this tool should create simple file, where describes JSON objects.
```c
JSON_MODEL(User,
	JSON_STRING(name),
	JSON_STRING(password),
	JSON_BOOLEAN(access));

JSON_MODEL(Access,
	JSON_BOOLEAN(room),
	JSON_BOOLEAN(car));

JSON_MODEL(OneRequest,
	JSON_STRING(text),
	JSON_NUMBER(value),
	JSON_BOOLEAN(switcher),
	JSON_ARRAY(User, users),
	JSON_OBJECT(Access, access));
```
For such model we will get such C language structures.
```c
typedef struct User {
        char* name;
        char* password;
        unsigned char access;
} User;

typedef struct Access {
        unsigned char room;
        unsigned char car;
} Access;

typedef struct OneRequest {
        char* text;
        int value;
        unsigned char switcher;
        struct User** users;
        int usersSize;
        struct Access* access;
} OneRequest;
```
Also additional tool functions will be generated.
```c
struct User* mallocUser();
void freeUser(struct User* object);
void serializeUser(GOC_OStream* stream, struct User* object);
struct User* deserializeUserFromJson(json_t* jsonRoot);
struct User* deserializeUser(GOC_IStream* stream);

struct Access* mallocAccess();
void freeAccess(struct Access* object);
void serializeAccess(GOC_OStream* stream, struct Access* object);
struct Access* deserializeAccessFromJson(json_t* jsonRoot);
struct Access* deserializeAccess(GOC_IStream* stream);

struct OneRequest* mallocOneRequest();
void freeOneRequest(struct OneRequest* object);
void serializeOneRequest(GOC_OStream* stream, struct OneRequest* object);
struct OneRequest* deserializeOneRequestFromJson(json_t* jsonRoot);
struct OneRequest* deserializeOneRequest(GOC_IStream* stream);
```
Every malloc* operation allocates memory for structure.
Every free* operation frees memory used by structure and also calls free for every element in the structure (string, object and array).
Every serialize* operation do a serialize to JSON process.
Evry deserialize* operation deserialize from JSON (by default from input stream object) but it may be also from json_t* from jannson library.
