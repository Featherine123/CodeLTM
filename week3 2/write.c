#include <stdio.h>

int main() {
    // Mở tệp CSV để ghi (tạo hoặc mở lại tệp đã tồn tại)
    FILE *csvFile = fopen("mydata.csv", "a");

    if (csvFile == NULL) {
        perror("Không thể mở tệp CSV");
        return 1;
    }

    // Ghi dữ liệu vào tệp CSV
    fprintf(csvFile, "Header1, Header2, Header3\n");
    fprintf(csvFile, "Data1, Data2, Data3\n");
    fprintf(csvFile, "MoreData1, MoreData2, MoreData3\n");

    // Đóng tệp CSV
    fclose(csvFile);

    return 0;
}
