#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#define SKIP_PEER_VERIFICATION = TRUE;
// libcurl result containing html text
struct MemoryStruct {
  char *memory;
  size_t size;
};
// libcurl callback function
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  // full size
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  // allocate required space
  char *temp = realloc(mem->memory, mem->size + realsize + 1);
  if(temp == NULL) {
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
char* GetDomain(char * link){
  int i; 
  int counter;
  for (i = 0; i < strlen(link); i = i + 1){
    if(link[i] == '/') counter = counter + 1; 
    if(link[i] == '\0') counter = 3; 
    if(counter == 3){
      break; 
    }
  }
  char* result = malloc(i+1); 
  memcpy(result,&link[0],i);
  result[i] = '\0'; 
  
  return result;
}// END GetDomain

void ExtractLinks(char* html, char* filter, char* folder){
  int index = 0; 
  int linkLength=0; 
  char* currentLink; 

}
int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode res;
  
  char* folder; 
  char* filter; 

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc in callback func */ 
  chunk.size = 0;    /* no data at this point */ 
  //init curl library globally
  curl_global_init(CURL_GLOBAL_DEFAULT);

  /* init the curl session */ 
  curl = curl_easy_init();
  /* send all data to this function  */ 
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
   /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

/*
  switch(argc){
  case 1: help();
    break;
  case 2: run_default(argv[1]); //  link
    break;
  case 3: run(argv[1],argv[2]); //  link, filter,
    break;
  case 4: run(argv[1],argv[2],argv[3]); // website,filter,folder
    break;
  default:
      printf("The number of arguments is incorrect.")
      help();
    break;
  }
*/

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");


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
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      exit(EXIT_FAILURE); 
    }
    else {
     // Download(chunk.memory,filter,folder); 
      char* stackoverflow = "https://stackoverflow.com";
      char* domain; 
      domain = GetDomain(stackoverflow); 
      printf("%s",domain); 
     // printf("Printing HTML: \n"); 
     // printf("%s",chunk.memory); 
    }

  }

  
  /* always cleanup */
  curl_easy_cleanup(curl);
  free(chunk.memory);
  curl_global_cleanup();

  return 0;
}