#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Callback function to write data received from the web to a file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Sai số lượng đối số truyền vào. Nhập lại theo cú pháp ./crawler IP\n");
        return 1;
    }

    char url[256];
    sprintf(url, "http://%s", argv[1]); // Tạo URL từ địa chỉ IP

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Không thể khởi tạo cURL.\n");
        return 1;
    }

    // Tạo các tệp .csv để lưu dữ liệu
    FILE *linksFile = fopen("links.csv", "w");
    FILE *textFile = fopen("text.csv", "w");
    FILE *videosFile = fopen("videos.csv", "w");

    if (!linksFile || !textFile || !videosFile) {
        fprintf(stderr, "Không thể tạo hoặc mở các tệp .csv.\n");
        return 1;
    }

    // Thiết lập cURL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, linksFile);

    // Tải trang web
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Lỗi trong quá trình tải trang: %s\n", curl_easy_strerror(res));
        return 1;
    }

    // TODO: Phân tích nội dung trang web để tìm các liên kết, văn bản và video

    // Sắp xếp các tệp theo thứ tự alphabet (đây chỉ là một ví dụ cơ bản)
    system("sort links.csv -o links.csv");
    system("sort text.csv -o text.csv");
    system("sort videos.csv -o videos.csv");

    // Đóng các tệp
    fclose(linksFile);
    fclose(textFile);
    fclose(videosFile);

    // Cleanup cURL
    curl_easy_cleanup(curl);

    return 0;
}
