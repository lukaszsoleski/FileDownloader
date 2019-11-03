### Program workflow
A console program that starts with writing a greeting on the standard output, then asking for the required arguments: page address, storage location and optional file extension for filtering. The user has the option to directly start downloading files giving the required arguments at startup. 

The content of the given website is saved in the global char pointer variable. The function scans its contents for the src and href tags. Depending on whether the content of the tag contains a specified extension, it will be saved to the global variable. 

Additionally, it is checked if the link is not a relative path to the file. Otherwise, the combine path function is executed to find the absolute path to the resource on the server. The application uses the [LibCurl](https://curl.haxx.se/libcurl/) library's 'EASY' interface to support HTTP. 
