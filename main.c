#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define SKIP_PEER_VERIFICATION 1
#define LINKS_CAPACITY 256

const char* const tokens [] = {"href='","href=\"","src='","src\""};
int link_count; 
struct MemoryStruct {
  char *memory;
  size_t size;
};
static size_t ToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}
// libcurl callback function for getting html content
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
int GetLength(char* ptr){
  int count = 0; 
  int i = 0; 
  while(ptr[i] != '\0')
  {
      count = count + 1;
      i = i + 1; 
  }  
  return count; 
}
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
  char* result = malloc((i+1)* sizeof(char));
  memcpy(result,&link[0],i);
  result[i] = '\0';
  return result;
}// END GetDomain
int IsSpace(char c){
  return c == ' ' || c == '\t' || c == '\n';
}

// ignore spaces and case
// return end index pos or -1
int StartsWith(char * str, char * substr, int startAt)
{

  int max = strlen(substr);
  int count = 0;
  for (int i = startAt; i < strlen(str); i = i + 1) {
    if(!IsSpace(str[i])){

      if(tolower(str[i]) == tolower(substr[count])){
        count = count + 1;
      }
      else return -1;
    }
    if (count == max) return i;
  }
}
int GetLinkLength(char*html,int index, char endchar){
  int i = index; 
  int count = 0; 
  while(html[i] != endchar){
    count = count + 1; 
    i = i + 1; 
  }
  return count; 
}
// returns link position. 
char* GetLink(char * html, int index, char endChar){
  int count = 0;
  int i = index;  
  int length = GetLinkLength(html,index,endchar); 

  char* link = malloc(length * sizeof(char)); 
  while(html[i] != endchar){
        link[count] = html[i]; 
        count = count + 1; 
         i = i + 1; 
  }
  link[count] = '\0'; 
  return link; 
}
int Filter(char* link, char* filter){
  
  int filterLen = GetLength(filter); 
  if(filterLen == 0) return 1; 

  int index = GetLength(link) - filterLen - 1; 
  int res = StartsWith(link,filter,index);
  return res; 

}
void ExtractLinks(char* html, char* filter, char** out){
  int maxLinksAmount = LINKS_CAPACITY;
  int currentLength = 0; 
  int currentBeginsAt = 0; 
  int currentEndsAt = 0; 
  int count = 0; 
  for(int i =0; i < strlen(html); i = i +1){
      int isLink = -1; 
      for(int j = 0; j < sizeof(tokens)/sizeof(tokens[0]); j = j + 1){
          isLink = StartsWith(html,tokens[j],i); 
          if(i > 0) break;
      }
      if(i < 0) continue; 
      
      // else :      
      char * link = GetLink(html,i,html[i-1]); 
      int isSearched = Filter(link,filter); 
      if(!isSearched) {
        free(link); 
        continue; 
      }
      out[i] = link;
      int len = GetLength(link);
      i = i + len; 

      count = count + 1; 

    }
    link_count = count; 
}

int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode res;
  char** links; 
  links = malloc(LINKS_CAPACITY * sizeof(char*));
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
      char* stackoverflow = "https://stackoverflow.com/questions/6803387/why-can-a-string-be-assigned-to-a-char-pointer-but-not-to-a-char-array";
      char* domain;
      domain = GetDomain(stackoverflow);
      printf("%s\n",domain);
      char* mainstr = " < href  = 'http:stackoverflow.com'";
      char * sub = "HReF='";
      int res = StartsWith(mainstr,sub,2);
      ExtractLinks(html,filter,links);
      FILE* files[link_count]; 
      //todo: get file name; 
      curl_easy_reset(curl); 
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ToFile);

    
      for(int i = 0; i < link_count; i = i + 1){
        char* name = GetFileNameFromLink(links[i]); 
        char* path = CombinePath(folder,name); 
        
        files[i] = fopen(path,"wb");
        if(files[i]){
          curl_easy_setopt(curl, CURLOPT_URL, links[i]);

          /* write the page body to this file handle */ 
         curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, files[i]);
           /* get it! */ 
         curl_easy_perform(curl_handle);

          /* close the header file */ 
         fclose(pagefile);
 
        }
      }
    }

  }
  /* always cleanup */
  curl_easy_cleanup(curl);
  free(chunk.memory);
  curl_global_cleanup();

  return 0;
}
