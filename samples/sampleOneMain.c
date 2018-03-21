#include "sampleOne.h"
#include <tools/memostream.h>
#include <tools/memistream.h>
#include <string.h>
#include <stdio.h>
#include <tools/tablica.h>

/**
 * Create User structure.
 */
struct User* createUser(const char* name, const char* password, unsigned char access) {
	struct User* result = mallocUser();
	result->name = name ? strdup(name) : NULL;
	result->password = password ? strdup(password) : NULL;
	result->access = access;
	return result;
}

int main() {
	// Create the OneRequest structure with all fields
	OneRequest* request = mallocOneRequest();
	request->text = strdup("First request");
	request->value = 12345;
	request->switcher = 1;
	request->users = goc_tableAdd(request->users, &request->usersSize, sizeof(void*));
	request->users[0] = createUser("Piotr", "Piotr password", 1);
	request->users = goc_tableAdd(request->users, &request->usersSize, sizeof(void*));
	request->users[1] = createUser("Guest", NULL, 0);
	request->access = mallocAccess();
	request->access->room = 1;
	request->access->car = 0;

	// Serialize to JSON and print out the structure of OneRequest
	GOC_OStream* ostream = goc_memOStreamOpen();
	serializeOneRequest(ostream, request);
	printf("Serialized JSON after creation\n");
	printf("%.*s\n", goc_memOStreamSize(ostream), goc_memOStreamGet(ostream));
	freeOneRequest(request);

	// Do a deserialization on base of serialized JSON string
	GOC_IStream* istream = goc_memIStreamOpen(goc_memOStreamGet(ostream), goc_memOStreamSize(ostream));
	goc_osClose(ostream);
	request = deserializeOneRequest(istream);
	goc_isClose(istream);

	// Serialize to JSON readed object and print out result to see if it is same
	ostream = goc_memOStreamOpen();
	serializeOneRequest(ostream, request);
	printf("Serialized JSON after deserialization\n");
	printf("%.*s\n", goc_memOStreamSize(ostream), goc_memOStreamGet(ostream));
	goc_osClose(ostream);
	freeOneRequest(request);
	return 0;
}
