#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define SKIP_PEER_VERIFICATION 1
#define LINKS_CAPACITY 256
#define MAX_LINK_LENGTH 1024
#define FOLDER_LENGTH 512
#define FILTER_LENGTH 16
#define FOLDER_LENGTH 512



struct StringStruct {
	char *memory;
	size_t size;
};

// variables:
const char* const _linkTokens[] = { "href='","href=\"","src='","src=\"" };
char _links[LINKS_CAPACITY][MAX_LINK_LENGTH + 1];
int  _linksCount = 0;
char _mainWebsiteLink[MAX_LINK_LENGTH];
char _filter[FILTER_LENGTH];
char _folder[FOLDER_LENGTH];

/* LibCurl callback method for writing to files: */
static size_t ToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

/*libcurl callback function for getting html content*/
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	// full size
	size_t realsize = size * nmemb;
	struct StringStruct *mem = (struct StringStruct *)userp;
	// allocate required space
	char *temp = realloc(mem->memory, mem->size + realsize + 1);
	if (temp == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	// set allocated memory pointer to global variable
	mem->memory = temp;
	// copy data to the allocated space
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	// update counter
	mem->size += realsize;
	// add the end of string character
	mem->memory[mem->size] = '\0';
	// return written size
	return realsize;
} // End WriteMemoryCallback
/*Returns last char in string*/
char GetLastChar(char* str)
{
	int len = GetLength(str);
	char last = str[len - 1];
	return last;
}
/*Checks if character is space, tabulator or new line*/
int IsWhiteSpace(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}

int GetLength(char* ptr) {

	return strlen(ptr);

}
char* GetDomain(char * link) {// \/
	int i;
	int counter;
	for (i = 0; i < strlen(link); i = i + 1) {
		if (link[i] == '/') counter = counter + 1;
		if (link[i] == '\0') counter = 3;
		if (counter == 3) {
			break;
		}
	}
	char* result = malloc((i + 1) * sizeof(char));
	memcpy(result, &link[0], i);
	result[i] = '\0';
	return result;
}// END GetDomain

/*Returns full path to directory without any file name*/
char * GetFolder(char * path, char separator)
{
	int max = GetLength(path);
	char * folder = malloc(sizeof(char*)*max);
	int end = 0;
	for (int i = max - 1; i > 0; i = i - 1)
	{
		if (path[i] == separator) {
			end = i;
			break;
		}
	}
	int j = 0;
	while (j < end) {
		folder[j] = path[j];
		j = j + 1;
	}
	folder[j] = '\0';

	return folder;
}
// While comparing ignores white spaces of the main string
//Returns -1 if comparison failed otherwise final index position
int StartsWithIgnoringCaseAndSpaces(char * str, char * substr, int startAt)
{
	int max = GetLength(substr);
	int count = 0;

	for (int i = startAt; i < strlen(str); i = i + 1) {
		if (IsWhiteSpace(str[i])){

                continue;
		}
		if (tolower(str[i]) == tolower(substr[count])) {
			count = count + 1;
		}
		else return -1;

		if (count == max) return i;

	}
	return -1;
}
// count the number of characters up to specified one
int CountFromTo(char* str, int startAt, char endChar) {
	int i = startAt;
	if (i >= strlen(str) || i < 0) return -1;
	int count = 0;
	int substrLength = strlen(str) - startAt - 1;
	while (str[i] != endChar && i <= startAt + substrLength) {
		count = count + 1;
		i = i + 1;
		if (i > startAt + substrLength) return -1;
	}
	return count;
}
/*Returns substring of given string from given index to (excluding) ending char*/
char* GetSubstringFromToExcluding(char * str, int startAt, char endChar) {
	if (startAt >= strlen(str) || startAt < 0) return NULL;
	int count = 0;
	int i = startAt;

	int length = CountFromTo(str, startAt, endChar);
	if (length < 0) return NULL;
	char* link = malloc(length * sizeof(char*));
	while (str[i] != endChar) {
		link[count] = str[i];
		count = count + 1;
		i = i + 1;
	}
	link[count] = '\0';
	return link;
}
/*Checks if string ends with given substring*/
int EndsWith(char* str, char* filter) {

	if (filter == NULL || filter == NULL) return 0;
	int filterLen = GetLength(filter);
	int strLen = GetLength(str);

	if (filterLen > strLen || strLen == 0 || filterLen == 0) return 0;

	int index = strLen - filterLen;
	int res = StartsWithIgnoringCaseAndSpaces(str, filter, index);
	if (res < 0) return 0;
	else return 1;
}
void printLinks(){
    printf("\nLinks count: %d\n", _linksCount);
for(int i = 0; i < _linksCount; i = i + 1){
    printf("%s\n",_links[i]);
    }
}
/*Extracts links to global variable */
void ExtractLinks(char* html, char* filter) {
	_linksCount = 0;
	int maxLinksAmount = LINKS_CAPACITY;
	int maxLinkLength = MAX_LINK_LENGTH;
	int htmlLength = GetLength(html);
	int filterLength = GetLength(filter);
	int count = 0;
	if (htmlLength < filterLength) return;
    int tokensCount = sizeof(_linkTokens) / sizeof(_linkTokens[0]);

	for (int i = 0; i < htmlLength; i = i + 1) { //iterate through html
        char tempCharCurr = html[i];
		if (maxLinksAmount <= count) break;
		int isLink = -1;
		int j = 0;

		for (j = 0; j < tokensCount; j = j + 1) {//4
			// check for href or src elements
			isLink = StartsWithIgnoringCaseAndSpaces(html, _linkTokens[j], i);
			// break if found
			if (isLink >= 0) break;
		}
		// continue if not found
		if (isLink < 0) continue;

		// link found:
		char endChar = GetLastChar(_linkTokens[j]);

		char * link = GetSubstringFromToExcluding(html, isLink + 1, endChar);
		// maintain i var
        int linkLen = GetLength(link);
        // go to end of found link
		i = isLink + 1 + linkLen;
		if(link[0] == '#'){
                free(link);
                continue;

        }
		// check extension
		int isSearched = 1;
		if(filterLength > 0){
         isSearched = EndsWith(link, filter);
		}
        if (!isSearched) {
			free(link);
			continue;
        }
		strcpy(_links[count], link);
		free(link);

		count = count + 1;
		_linksCount = _linksCount + 1;

	}
}
/*Returns substring from given string starting from last occurrence of given character*/
char * GetLastSeparatedItem(char * str, char separator) {
	int length = GetLength(str);
	int index = -1;
	// Get index of last char in string
	for (int i = length - 1; i >= 0; i = i - 1) {
		if (str[i] == separator) {
			index = i;
			break;
		}

	}
	if (index < 0)return NULL;
	index = index + 1;
	int count = length - index;
	char* item = malloc((count + 1) * sizeof(char*));

	for (int i = 0; i < count; i = i + 1) {
		item[i] = str[index];
		index = index + 1;
	}
	item[count] = '\0';
	return item;
}
/*Returns substring of given string*/
char * GetSubstring(char* str, int startAt, int endAt) {
	int strLen = GetLength(str);
	if (startAt > endAt || strLen == 0) return NULL;
	if (strLen <= endAt) return NULL;

	int count = endAt - startAt + 1;

	char* sub = malloc(count + 1);
	int subIndex = 0;
	for (int i = startAt; i <= endAt; i = i + 1)
	{
		sub[subIndex] = str[i];
		subIndex = subIndex + 1;
	}
	sub[count] = '\0';
	return sub;
}

// move back
char* UndoFolder(char* folder, int count, char separator) {
	int folderLen = GetLength(folder);
	if (folderLen <= 1 || count < 1) return folder;

	int slashes = 0;
	for (int i = folderLen - 1; i > 0; i = i - 1)
	{
		if (folder[i] == separator)
		{
			slashes = slashes + 1;
		}
		if (slashes == count)
		{
			char * result = GetSubstring(folder, 0, i - 1);
			return result;
		}
	}
	return folder;
}
char* CombinePath(char* folder, char* file) {
	// check if relative
	int isRelative = StartsWithIgnoringCaseAndSpaces(file, "../", 0);
	int undoCount = 0;
	// start iterating from
	int fileCombineFrom = 0;
	char * directory;
	char * filePart;
	//
	// if path to the file is relative undo the folder path
	if (isRelative >= 0) {
		int index = isRelative;
		undoCount = undoCount + 1;
		while ((index = StartsWithIgnoringCaseAndSpaces(file, "../", index + 1)) > 0) {
			undoCount = undoCount + 1;
		}
		// clear all '../../' occurrences
		int i = (undoCount * 3);
		filePart = GetSubstring(file, i, GetLength(file) - 1);
		directory = UndoFolder(folder, undoCount, '/');

		int dirLen = GetLength(directory);
		if (directory[dirLen - 1] == '/')
			directory[dirLen - 1] = 0;
	}
	else // setup if absolute
	{
		if (file[0] == '/')
			filePart = GetSubstring(file, 1, GetLength(file) - 1);
		else
			filePart = file;
		int dirLen = GetLength(folder);
		if (folder[dirLen - 1] == '/')
			directory = GetSubstring(folder, 0, dirLen - 2);
		else
			directory = folder;
	}
	// copy directory and filename into sigle string

	int combinedLength = GetLength(directory) + GetLength(filePart) + 2;
	char* path = malloc(combinedLength * sizeof(char*));

	int i = 0;
	while (directory[i] != '\0')
	{
		path[i] = directory[i];
		i = i + 1;
	}
	path[i] = '/';
	i = i + 1;
	int j = 0;
	while (filePart[j] != '\0') {
		path[i] = filePart[j];
		i = i + 1;
		j = j + 1;
	}
	path[i] = '\0';
	return path;
}
char* GetLine(char* display, char * arr) {
	printf("%s\n", display);
	gets(arr);
}
int Help() {
	GetLine("Website link: ", &_mainWebsiteLink);
	GetLine("Output directory: ", &_folder);

	char useExt[2];
	GetLine("Use file extension filter?(y/n)", &useExt);
	if (useExt[0] == 'y')
		GetLine("Filter: ", &_filter);
}
int Contains(char* str, char el){
    int len = GetLength(str);
    int result = 0;
    for(int i = 0; i < len; i = i + 1)
    {
        if(str[i] == el)
        {
            result = 1;
            break;
        }
    }
    return result;
}
int main(int argc, char *argv[])
{


	CURL *curl;
	CURLcode curlRes;

	struct StringStruct html;

	html.memory = malloc(1);  /* will be grown as needed by the realloc in callback func */
	html.size = 0;    /* no data at this point */
	//init curl library globally
	curl_global_init(CURL_GLOBAL_DEFAULT);

	/* init the curl session */
	curl = curl_easy_init();
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'html' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&html);
	/* some servers don't like requests that are made without a user-agent
	   field, so we provide one */
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    printf("%d",argc);

	switch (argc) {

	case 1:
		//TEMP DATA FOR TESTING
		strcpy(_mainWebsiteLink, "http://127.0.0.1:8080/");
		strcpy(_folder, "C:\\Users\\Luke\\Desktop\\temp");
		strcpy(_filter,".css");
		printf("\n%s",_mainWebsiteLink);
		printf("\n%s",_folder);

		break;
	case 3:
		strcpy(_mainWebsiteLink, argv[1]);
		strcpy(_folder, argv[2]);
		_filter[0] = 0;
		break;
	case 4:
		strcpy(_mainWebsiteLink, argv[1]);
		strcpy(_folder, argv[2]);
		strcpy(_filter, argv[3]);
		break;
	default:
		printf("The number of arguments is incorrect.");
		Help();
		break;
	}

	// check if curl initialization has completed correctly
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, &_mainWebsiteLink);


#ifdef SKIP_PEER_VERIFICATION
		/*
		 * If you want to connect to a site who isn't using a certificate that is
		 * signed by one of the certs in the CA bundle you have, you can skip the
		 * verification of the server's certificate. This makes the connection
		 * A LOT LESS SECURE.
		 *
		 * If you have a CA cert for the server stored someplace else than in the
		 * default bundle, then the CURLOPT_CAPATH option might come handy for
		 * you.
		 */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		/*
		 * If the site you're connecting to uses a different host name that what
		 * they have mentioned in their server certificate's commonName (or
		 * subjectAltName) fields, libcurl will refuse to connect. You can skip
		 * this check, but this will make the connection less secure.
		 */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

		/* Perform the request, res will get the return code */
		curlRes = curl_easy_perform(curl);
		/* Check for errors */
		if (curlRes != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRes));
			exit(EXIT_FAILURE);
		}
		else {
            printf("%s",html.memory);
			ExtractLinks(html.memory,&_filter);
			printf("\nLinks:");
			for(int i = 0; i < _linksCount; i = i + 1){
                printf("\n%s",_links[i]);
			}
			// array of pointers to files
			FILE* currFile;
			//todo: get file name;
			curl_easy_reset(curl);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ToFile);
			// iterate thougth extracted links
			// combine path
			for (int i = 0; i < _linksCount; i = i + 1) {
                char * name;
                if(Contains(_links[i],'/'))
                {
                     name = GetLastSeparatedItem(_links[i],'/');
                }
				else
                {
                    name = _links[i];
                }
				int isAbsolutePath = StartsWithIgnoringCaseAndSpaces(_links[i], "http", 0);
				char* localPath = CombinePath(_folder, name);
				// create absolute path if it is relative
				if (isAbsolutePath < 0) {

					char* websiteDir = GetFolder(_mainWebsiteLink,'/');
					char* link = CombinePath(websiteDir, _links[i]);
					printf("\nCombined path: %s",link);
					free(_links[i]);
					strcpy(_links[i], link);
					//free local pointers
					free(websiteDir);
					free(link);
				}// end if isAbsolutePath

				currFile = fopen(localPath, "wb+");
				if (currFile) {
					curl_easy_setopt(curl, CURLOPT_URL, _links[i]);

					/* write the page body to this file handle */
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, currFile);

					printf("\nPerforming download... %s",_links[i]);
					/* get it! */
					curl_easy_perform(curl);
					/* close the header file */
					fclose(currFile);
                    printf("\nSaved: %s\n",localPath);
                    free(currFile);
				}
				free(name);
				free(localPath);

			}
		}
	}
	/* always cleanup */
	curl_easy_cleanup(curl);
	free(html.memory);

	curl_global_cleanup();

	return 0;
}

