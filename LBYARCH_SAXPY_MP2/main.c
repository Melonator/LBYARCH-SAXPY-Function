#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define size1 1048576 //2^20
#define size2 16777216 //2^24
#define size3 268435456 //2^28
size_t custom = 0;
extern void process_array_asm(float *X, float *Y, float *Z, const float a, const size_t size);

void init_float_array(float *arr, size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        arr[i] = ((float)rand() / RAND_MAX) * 10.0;
    }
}

void process_array_c(float* X, float* Y, float* Z, const float a, const size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        Z[i] = a * X[i] + Y[i];
    }
}

int is_array_equal(float *arr1, float *arr2, const size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        if (arr1[i] != arr2[i]) {
            return 0;
        }
    }
    return 1;
}

void display_n_elements(float* arr, int n, char arrName) {
    int i;
    if (custom != 0 && n > custom) {
        n = custom;
    }

    printf("\n<The first %d elements of array %c>\n", n, arrName);
    for (i = 0; i < n; i++) {
        if (i != n - 1) {
			printf("%f, ", arr[i]);
        }
        else {
            printf("%f ", arr[i]);
        }
    }
}

void write_time_to_csv(const double cTimings[], const double asmTimings[], const int exponent, const int isDebugMode) {
	char fileName[100];

	if (isDebugMode)
        if(custom == 0)
			sprintf_s(fileName, sizeof(fileName), "Comparison_Analysis_Debug_2^%d.csv", exponent);
        else
			sprintf_s(fileName, sizeof(fileName), "Comparison_Analysis_Debug_%d.csv", custom);
	else
        if(custom == 0)
			sprintf_s(fileName, sizeof(fileName), "Comparison_Analysis_Release_2^%d.csv", exponent);
        else 
			sprintf_s(fileName, sizeof(fileName), "Comparison_Analysis_Release_%d.csv", custom);

    FILE* file = NULL;
    errno_t err = fopen_s(&file, fileName, "w");
    if (err != 0) {
        printf("Error opening file: %d\n", err);
        return;
    }

    if (custom == 0) 
		fprintf(file, "Vector Size (2^%d),Iteration,C Timing (s),Assembly Timing (s)\n", exponent);
    else
		fprintf(file, "Vector Size %d,Iteration,C Timing (s),Assembly Timing (s)\n", custom);

	for (int i = 0; i < 30 ; ++i) {
		if (i == 0) {
            if (custom == 0)
				fprintf(file, "%d,", 1 << exponent);
            else 
				fprintf(file, "%d,", custom);

		}
		else {
			fprintf(file, ",");
		}
		fprintf(file, "%d,%f,%f\n", i + 1, cTimings[i], asmTimings[i]);
	}

	fprintf(file, "\n, , Average Time (s): %f, Average Time (s): %f\n", cTimings[30], asmTimings[30]);

	fclose(file);
}

int main() {
    srand(time(NULL));

    printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("SAXPY Function Program\n");
    printf("Z = a * X + Y\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

    printf("Choose one size for X, Y, and Z\n");
    printf("1) 2^20\n2) 2^24\n3) 2^28\n4) Custom input\n5) Enter anything but 1,2,3 to exit\n>> ");
    char choice = getchar();
    size_t size = 0;
    int exponent = 0;
    int isDebugMode = 0;
    switch (choice) {
    case '1':
        size = size1;
        exponent = 20;
        break;
    case '2': 
        size = size2;
        exponent = 24;
        break;
    case '3':
        size = size3;
        exponent = 28;
        break;
    case '4':
        printf("Enter only positive integers...\n>>");
        scanf_s("%d", &custom);
        if (custom < 1) {
            printf("That's not positive!");
            return;
        }
        size = custom;
        break;
    default:
        printf("Exiting...\n");
        return 0;
    }

    float a = 0.0f;
    printf("Enter the value for A\n>>");
    scanf_s("%f", &a);

    #ifdef _DEBUG
		isDebugMode = 1;
	    printf("<The program is in debug mode>\n");
    #else
		isDebugMode = 0;
	    printf("<The program is in release mode>\n");
    #endif



    double* cTimings = (double*)malloc(31 * sizeof(double));
    double* asmTimings = (double*)malloc(31 * sizeof(double));

    printf("\nGenerating array of size %zu with random floating point numbers...\n", size);
    float* X = (float*)malloc(size * sizeof(float));
    float* Y = (float*)malloc(size * sizeof(float));
    float* Z1 = (float*)malloc(size * sizeof(float));
    float* Z2 = (float*)malloc(size * sizeof(float));
    init_float_array(X, size);
    init_float_array(Y, size);
    printf("Done generating array!\n");

    display_n_elements(X, 10, 'X');
    display_n_elements(Y, 10, 'Y');

    //Assembly benchamrk!
    int i;
    double totalTimeAsm = 0;
    double elapsedTime = 0;

    printf("\n\nProcessing array via assembly...\n");
    for (i = 0; i < 30; i++) {
        clock_t start_time = clock();
        process_array_asm(X, Y, Z1, a, size);
        clock_t end_time = clock();
        elapsedTime = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        asmTimings[i] = elapsedTime;
        totalTimeAsm += elapsedTime;
    }
    totalTimeAsm /= 30;
    asmTimings[30] = totalTimeAsm;
    display_n_elements(Z1, 10, 'Z');

    //C benchmark!
    double totalTimeC = 0;
    elapsedTime = 0;
    printf("\n\nProcessing array via C...\n");
    for (i = 0; i < 30; i++) {
        clock_t start_time = clock();
        process_array_c(X, Y, Z2, a, size);
        clock_t end_time = clock();
        elapsedTime = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        cTimings[i] = elapsedTime;
        totalTimeC += elapsedTime;
    }
    totalTimeC /= 30;
    cTimings[30] = totalTimeC;
    display_n_elements(Z2, 10, 'Z');

    printf("\n\nAverage time of %zu elements in assembly = %lf seconds\n", size, totalTimeAsm);
    printf("Average time of %zu elements in C = %lf seconds\n", size, totalTimeC);

    printf("Running verification...\n");
    printf("Checking if both the Assembly and C version produces the same array...\n");

    int isArrayEqual = is_array_equal(Z1, Z2, size);
    if (isArrayEqual) {
		printf("Both arrays are equal!\n");
    }
    else {
		printf("Both arrays are not equal...\n");
    }

    free(X);
    free(Y);
    free(Z1);
    free(Z2);

    write_time_to_csv(cTimings, asmTimings, exponent, isDebugMode);
    free(asmTimings);
    free(cTimings);

    return 0;
}