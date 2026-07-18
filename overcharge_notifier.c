#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <windows.h>

#define SMTP_SERVER "smtps://smtp.gmail.com:465"

char *SENDER_EMAIL_;
char *RECEIVER_EMAIL_;
char *SENDER_PASSWORD_;

char EMAIL_DATA_[1024];
char MAIL_FROM_[256];
char MAIL_RECIPIENT_[256];

struct UploadStatus
{
    size_t bytes_read;
};

static size_t payload_source(char *buffer, size_t size, size_t nmemb, void *user_pointer)
{
    struct UploadStatus *upload = (struct UploadStatus *)user_pointer;
    size_t buffer_size = size * nmemb;
    size_t data_length = strlen(EMAIL_DATA_);

    if (upload->bytes_read >= data_length)
        return 0;

    size_t remaining = data_length - upload->bytes_read;

    size_t copy_size = remaining < buffer_size ? remaining : buffer_size;

    memcpy(buffer, EMAIL_DATA_ + upload->bytes_read, copy_size);

    upload->bytes_read += copy_size;

    return copy_size;
}

void send_email()
{
    CURL *curl = NULL;
    CURLcode result;

    struct curl_slist *recipients = NULL;
    struct UploadStatus upload = {0};

    snprintf(
        EMAIL_DATA_,
        sizeof(EMAIL_DATA_),
        "To: %s\r\n"
        "From: Charge Notifier <%s>\r\n"
        "Subject: Battery reached 90%%\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "Your laptop battery has reached 90%%.\r\n"
        "Please unplug the charger.\r\n",
        RECEIVER_EMAIL_,
        SENDER_EMAIL_);

    snprintf(MAIL_FROM_, sizeof(MAIL_FROM_), "<%s>", SENDER_EMAIL_);
    snprintf(MAIL_RECIPIENT_, sizeof(MAIL_RECIPIENT_), "<%s>", RECEIVER_EMAIL_);

    result = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (result != CURLE_OK)
    {
        fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(result));
        return;
    }

    curl = curl_easy_init();

    if (curl == NULL)
    {
        fprintf(stderr, "Could not initialize libcurl.\n");
        curl_global_cleanup();
        return;
    }

    recipients = curl_slist_append(recipients, MAIL_RECIPIENT_);

    if (recipients == NULL)
    {
        fprintf(stderr, "Could not create recipient list.\n");
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, SMTP_SERVER);
    curl_easy_setopt(curl, CURLOPT_USERNAME, SENDER_EMAIL_);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, SENDER_PASSWORD_);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, MAIL_FROM_);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK)
        fprintf(stderr, "Email failed: %s\n", curl_easy_strerror(result));
    else
        printf("Email sent successfully.\n");

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

int main(void)
{
    /*
     * Compile:
     * gcc overcharge_notifier.c -o a.exe -lcurl -lcjson
     */

    SYSTEM_POWER_STATUS status;
    int interval = 660000;

    FILE *fp = fopen("./config.json", "rb");

    if (fp == NULL)
    {
        printf("Cannot open config.json\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END);

    long length = ftell(fp);

    if (length < 0)
    {
        printf("Cannot determine file size\n");
        fclose(fp);
        return 1;
    }

    rewind(fp);

    char *data = malloc((size_t)length + 1);

    if (data == NULL)
    {
        printf("Memory allocation failed\n");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(data, 1, (size_t)length, fp);

    data[bytes_read] = '\0';

    fclose(fp);

    cJSON *json = cJSON_Parse(data);

    if (json == NULL)
    {
        const char *error = cJSON_GetErrorPtr();
        printf("Invalid JSON: %s\n", error != NULL ? error : "unknown error");
        free(data);
        return 1;
    }

    cJSON *sender_email = cJSON_GetObjectItemCaseSensitive(json, "sender_email");

    cJSON *receiver_email = cJSON_GetObjectItemCaseSensitive(json, "receiver_email");

    cJSON *sender_password = cJSON_GetObjectItemCaseSensitive(json, "sender_password");

    if (!cJSON_IsString(sender_email) || !cJSON_IsString(receiver_email) || !cJSON_IsString(sender_password))
    {
        printf("Missing or invalid configuration values\n");
        cJSON_Delete(json);
        free(data);

        return 1;
    }

    SENDER_EMAIL_ = malloc(strlen(sender_email->valuestring) + 1);

    RECEIVER_EMAIL_ = malloc(strlen(receiver_email->valuestring) + 1);

    SENDER_PASSWORD_ = malloc(strlen(sender_password->valuestring) + 1);

    if (SENDER_EMAIL_ == NULL || RECEIVER_EMAIL_ == NULL || SENDER_PASSWORD_ == NULL)
    {
        printf("Memory allocation failed\n");

        free(SENDER_EMAIL_);
        free(RECEIVER_EMAIL_);
        free(SENDER_PASSWORD_);

        cJSON_Delete(json);
        free(data);

        return 1;
    }

    strcpy(SENDER_EMAIL_, sender_email->valuestring);

    strcpy(RECEIVER_EMAIL_, receiver_email->valuestring);

    strcpy(SENDER_PASSWORD_, sender_password->valuestring);

    printf("Sender: %s\n", SENDER_EMAIL_);

    printf("Receiver: %s\n", RECEIVER_EMAIL_);

    cJSON_Delete(json);
    free(data);

    while (GetSystemPowerStatus(&status))
    {
        int percentage = status.BatteryLifePercent;

        printf("Battery: %d%%\n", percentage);

        if (percentage >= 40 && percentage < 60)
            interval = 480000;
        else if (percentage >= 60 && percentage < 80)
            interval = 300000;
        else if (percentage >= 80 && percentage <= 92)
            interval = 120000;
        else if (percentage > 92)
            send_email();
        printf("Hibernating...\n");
        Sleep(interval);
    }

    free(SENDER_EMAIL_);
    free(RECEIVER_EMAIL_);
    free(SENDER_PASSWORD_);

    return 0;
}