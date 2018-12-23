#include <stdio.h>
#include <curl/curl.h>
#define SKIP_PEER_VERIFICATION = TRUE;

// libcurl callback function
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
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

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

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
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}


