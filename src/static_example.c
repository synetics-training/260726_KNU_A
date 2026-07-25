/**
 * @file static_example.c
 * @brief 정적 분석 도구 실습용 예제
 *
 * 센서 데이터를 처리하는 여러 함수를 포함한다.
 * cppcheck와 lizard를 실행하여 코드 품질 문제를 찾아보세요.
 *
 * 실행 방법 (프로젝트 루트에서):
 *   python -m cppcheck --enable=all --check-level=exhaustive src/static_example.c
 *   python -m lizard -C 10 -L 80 src/static_example.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 마지막 처리 결과와 오류 횟수를 기록하는 전역 변수 */
int g_last_result = 0;
int g_error_count = 0;

/**
 * @brief 센서 ID에 따라 측정값을 가공하여 반환한다.
 *
 * sensor_id별 처리 방식:
 *   1번 센서: threshold 초과 여부와 mode에 따라 값을 변환
 *   2번 센서: 음수/영/양수에 따라 절대값, 기본값, 보정값을 반환
 *   3번 센서: mode별 상한값(100 또는 200)을 적용
 *
 * 처리 결과는 전역 변수 g_last_result에도 저장된다.
 * 결과가 음수이면 g_error_count가 증가한다.
 */
int process_sensor_data(int sensor_id, int value, int mode,
                        int calibrated, int threshold)
{
    int result = 0;
    int temp;

    if (sensor_id == 1) {
        if (value > threshold) {
            if (mode == 0) {
                result = value * 2;
            } else if (mode == 1) {
                result = value + 10;
            } else if (mode == 2) {
                if (calibrated) {
                    result = value - 5;
                } else {
                    result = value;
                }
            }
        } else {
            if (mode == 0) {
                result = 0;
            } else {
                result = -1;
            }
        }
    } else if (sensor_id == 2) {
        if (value < 0) {
            result = -value;
        } else if (value == 0) {
            result = threshold;
        } else {
            if (calibrated && mode > 0) {
                result = value * mode;
            } else {
                result = value;
            }
        }
    } else if (sensor_id == 3) {
        if (mode == 0 && value > 100) {
            result = 100;
        } else if (mode == 1 && value > 200) {
            result = 200;
        } else {
            result = value;
        }
    }

    temp = value + threshold;
    g_last_result = result;
    if (result < 0) g_error_count++;

    return result;
}

/**
 * @brief 캐시된 센서 데이터를 재처리한다.
 *
 * process_sensor_data의 sensor_id 1번과 2번 처리 로직을
 * 순차적으로 적용한다.
 * 먼저 threshold 기준으로 1번 센서 로직을 수행한 뒤,
 * 이어서 2번 센서 로직으로 결과를 덮어쓴다.
 */
int process_sensor_cached(int value, int mode, int calibrated, int threshold)
{
    int result = 0;

    /* 1번 센서 로직: threshold 기준 분기 */
    if (value > threshold) {
        if (mode == 0) {
            result = value * 2;
        } else if (mode == 1) {
            result = value + 10;
        } else if (mode == 2) {
            if (calibrated) {
                result = value - 5;
            } else {
                result = value;
            }
        }
    } else {
        if (mode == 0) {
            result = 0;
        } else {
            result = -1;
        }
    }

    /* 2번 센서 로직: 음수/영/양수 분기 */
    if (value < 0) {
        result = -value;
    } else if (value == 0) {
        result = threshold;
    } else {
        if (calibrated && mode > 0) {
            result = value * mode;
        } else {
            result = value;
        }
    }

    g_last_result = result;
    return result;
}

/**
 * @brief 접두사와 코드 번호를 결합하여 메시지를 출력한다.
 *
 * "PREFIX-0001" 형식의 문자열을 생성하여 출력한다.
 * buffer는 10바이트 크기로 할당되어 있다.
 */
void format_message(const char *prefix, int code)
{
    char buffer[10];
    sprintf(buffer, "%s-%04d", prefix, code);
    printf("Message: %s\n", buffer);
}

/**
 * @brief 정수 배열을 동적으로 생성하여 반환한다.
 *
 * size 크기의 배열을 할당한 뒤 짝수 값(0, 2, 4, ...)으로 초기화한다.
 * 이후 size*2 크기의 새 배열을 할당하여 반환한다.
 */
int* create_array(int size)
{
    int *arr = (int*)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }

    arr = (int*)malloc(size * 2 * sizeof(int));

    return arr;
}

/**
 * @brief 센서 데이터 배열의 내용을 출력한다.
 *
 * 첫 번째 요소를 먼저 출력한 뒤,
 * data가 NULL이 아니면 전체 요소를 순회하며 출력한다.
 */
void print_sensor_info(int *data, int count)
{
    printf("First value: %d\n", data[0]);

    if (data != NULL) {
        for (int i = 0; i < count; i++) {
            printf("  [%d] = %d\n", i, data[i]);
        }
    }
}

/**
 * @brief 두 정수의 비율을 백분율로 계산한다.
 *
 * numerator를 denominator로 나눈 뒤 100을 곱하여 반환한다.
 */
double calculate_ratio(int numerator, int denominator)
{
    double result = numerator / denominator;
    return result * 100.0;
}

/**
 * @brief 센서 상태를 갱신하고 출력한다.
 *
 * sensor_id가 양수이면 value 기준으로 활성 상태를 판정하여 출력하고,
 * 마지막에 최종 status 값을 출력한다.
 */
void update_sensor_status(int sensor_id, int value)
{
    int status = 0;
    if (sensor_id > 0) {
        int status = value > 100 ? 1 : 0;
        printf("Active: %d\n", status);
    }
    printf("Final status: %d\n", status);
}

/**
 * @brief 배열에서 target 값의 위치를 찾아 반환한다.
 *
 * 배열을 순회하며 일치하는 값을 찾으면 해당 인덱스를 반환한다.
 * 찾지 못하면 -1을 반환한다.
 * 찾은 인덱스는 전역 변수 g_last_result에도 저장된다.
 */
int find_value(int *data, int size, int target)
{
    for (int i = 0; i < size; i++) {
        if (data[i] == target) {
            g_last_result = i;
            goto done;
        }
    }
    return -1;
done:
    return g_last_result;
}

/**
 * @brief 각 함수를 호출하여 동작을 확인하는 메인 함수
 */
int main(void)
{
    int result = process_sensor_data(1, 50, 0, 1, 40);
    printf("Result: %d\n", result);

    int cached = process_sensor_cached(50, 0, 1, 40);
    printf("Cached: %d\n", cached);

    format_message("SENSOR", 12345);

    int *arr = create_array(5);
    print_sensor_info(arr, 5);
    free(arr);

    double ratio = calculate_ratio(7, 2);
    printf("Ratio: %.1f%%\n", ratio);

    update_sensor_status(1, 150);

    int values[] = {10, 20, 30, 40, 50};
    int idx = find_value(values, 5, 30);
    printf("Found at: %d\n", idx);

    return 0;
}
