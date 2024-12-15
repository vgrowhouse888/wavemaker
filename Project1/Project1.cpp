#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

// Helper function to handle response data from the server
size_t write_callback(void* ptr, size_t size, size_t nmemb, char* data) {
    strcat(data, ptr);
    return size * nmemb;
}

// Function to extract file ID from the Google Drive URL
char* extract_file_id(const char* url) {
    char* file_id = malloc(100);
    if (file_id != NULL) {
        sscanf(url, "https://drive.google.com/file/d/%99[^/]", file_id);
    }
    return file_id;
}

// Function to send POST request to Prodia API
void send_post_request(const char* url, const char* headers[], const char* json_data) {
    CURL* curl;
    CURLcode res;
    char response[10000] = { 0 }; // Allocate memory to store the response data

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* header_list = NULL;
        for (int i = 0; headers[i] != NULL; i++) {
            header_list = curl_slist_append(header_list, headers[i]);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else {
            printf("Response: %s\n", response);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(header_list);
    }

    curl_global_cleanup();
}

int main() {
    const char* imageUrlToTransform = "https://drive.google.com/file/d/1NyulaEmOS3S_wU67VZfj7QS1KHVGilc5/view?usp=sharing";

    // Extract file ID from the Google Drive URL
    char* file_id = extract_file_id(imageUrlToTransform);
    if (file_id == NULL) {
        fprintf(stderr, "Failed to extract file ID from URL.\n");
        return 1;
    }

    // Construct the direct download link
    char directDownloadLink[256];
    snprintf(directDownloadLink, sizeof(directDownloadLink), "https://drive.google.com/uc?export=download&id=%s", file_id);
    printf("Direct Download Link: %s\n", directDownloadLink);

    // Prepare the JSON body for the request
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "imageUrl", directDownloadLink);
    cJSON_AddStringToObject(json, "model", "amIReal_V41.safetensors [0a8a2e61]");
    cJSON_AddNumberToObject(json, "steps", 77);
    cJSON_AddNumberToObject(json, "width", 1024);
    cJSON_AddNumberToObject(json, "height", 1024);
    cJSON_AddNumberToObject(json, "cfg_scale", 1);
    cJSON_AddStringToObject(json, "sampler", "PLMS");
    cJSON_AddStringToObject(json, "prompt", "Future Dystopia");

    char* json_string = cJSON_PrintUnformatted(json);

    // Set up the request headers
    const char* headers[] = {
        "accept: application/json",
        "content-type: application/json",
        "X-Prodia-Key: b339ce99-02e2-4e3b-9fc5-0158613a7944",
        NULL
    };

    // Send POST request to the API
    send_post_request("https://api.prodia.com/v1/sd/transform", headers, json_string);

    // Clean up the JSON object
    free(json_string);
    cJSON_Delete(json);
    free(file_id);

    return 0;
}
