/**
 * @file crc15.c
 * @author Sergey Kalinichenko (sergey.kalinichenko@yaelev.se)
 * @brief C Assignment 1
 *        Make a program to calculate and verify the CRC checksum of a message according to the below requirements. 
 *        Download the crc15.c file and develop the program in the file.
 * Requirements
 *  1. The message is an ​N​ element array of ​unsigned char​ (uint8_t)
 *  2. The minimum length of the message is ​1​ character.
 *  3. The maximum length of the message is ​14 ​characters.
 *  4. The CRC-15 polynomial is ​0xC599 ​(1100010110011001)
 *  5. The message is processed from the ​first ​element to the ​last ​element in the array
 *  6. Every byte is processed from the ​LSB ​to the ​MSB
 *  7. No magic number!
 * 
 * @version 0.6
 * @date 2021-02-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdint.h>
//#include <string.h>

#define POLYNOMIAL 0xC599U // The CAN protocol uses the CRC-15 with this polynomial

#define N_BITS_IN_BYTE 8
#define MASK16_W_ONLY_MSB_EQ_1 0x8000U
#define MASK8_W_ONLY_MSB_EQ_1 0x80U
#define MASK8_W_ONLY_LSB_EQ_1 0x01U

#define BYTES_TO_BITS(x) ((x) << 3)
#define IS_MSB_OF_16_BIT_VALUE_EQ_1(x) ((x) >= 0x8000U)
#define GET_LSB_OF_BYTE(x) ((x)&0x01U)
//#define GET_MSB_OF_BYTE(x) ((x)&0x80U)

#define RESET 1
#define NO_RESET 0
#define REVERSE_BIT_ORDER 1
#define REGULAR_BIT_ORDER 0
#define EXTRA_BIT_ADDED 1
#define EXTRA_BIT_NOT_ADDED 0

/**
 * @brief Printing a 16-bit number to stdout in binary form.
 * 
 * @param num - a 16-bit number
 * @return uint16_t (the input value)
 */
uint16_t prn16bin(const uint16_t num);

/**
 * @brief Calculating CRC-15.
 * 
 * @param arr - array of bytes the CRC is calculated for; 2 extra zero bytes at the end should be already present !!
 * @param n_arr - the array size WITHOUT 2 extra zero bytes
 * @param rev - if 1, bits in the array bytes are processed in the reverse order (from LSB to MSB)
 * @param xtra_bit - if 1, one extra zero bit is added to data before calc. of CRC-15
 * @return uint16_t (15-bit CRC value as a 16-bit value with MSB == 0)
 */
uint16_t crc_15(const uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit);

/**
 * @brief Adding checksum to array of bytes.
 * 
 * @param crc15 - 15-bit checksum (16-bit value with MSB == 0)
 * @param arr - the array the checksum is added to
 * @param n_arr - array size before checksum is added
 * @param rev - if 1, bits in both checksum bytes are added in the reverse order
 * @param xtra_bit - if 1, considering an extra zero bit added to data before calc. of CRC-15
 * @return int (0)
 */
int checksum_15(const uint16_t crc15, uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit);

/**
 * @brief Checking the integrity of a checksummed data array and reporting the result to stdin. 
 * 
 * @param arr - checksummed array
 * @param n_arr - array size without checksum
 * @param rev - if 1, bits in the array bytes are processed in the reverse order
 * @param xtra_bit - if 1, one extra zero bit is added to data before calc. of CRC-15
 * @return uint8_t (1 if CRC is OK, 0 otherwise)
 */
static inline uint8_t check_crc(const uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit);

/**
 * @brief Input of an alternative data to calculate CRC.
 * 
 * @param arr - array for data input
 * @param n_arr - max. data length (in bytes), so there should be extra 2 pos. in the array for "\n\0" / CRC
 * @return uint8_t (actual data length)
 */
static inline uint8_t input_alt_message(const uint8_t *arr, const uint8_t n_arr);

/**
 * @brief Calculating CRC and checking data integrity.
 * 
 * @return int (0)
 */
int main(void)
{
    uint8_t message[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0, 0}; // 15 zeros have been appended to the message

    // Calculate the CRC and Checksum the message

    uint16_t crc15;
    uint8_t input_message[sizeof(message)]; // alt. message may be input instead of the default one;
                                            //  max. num. of data bytes in the array is defined according to
                                            //  the default array message[]

    uint8_t len_of_input;
    uint8_t len_of_data = sizeof(message) - sizeof(crc15); // actual num. of data bytes in the array;
                                                           //  default array is message[]
    uint8_t *data_ptr = message;                           // default data to check CRC is message[]

    (void)printf("\nDefault message: \"%.*s\"\n", len_of_data, message);
    if ((len_of_input = input_alt_message(input_message, sizeof(message) - sizeof(crc15))) > 0) // alt. data may be input
    {
        len_of_data = len_of_input;
        data_ptr = input_message;
    }

    crc15 = crc_15(data_ptr, len_of_data, REVERSE_BIT_ORDER, EXTRA_BIT_ADDED);
    (void)printf("CRC-15:  0x%x / 0b", crc15), (void)prn16bin(crc15), (void)printf("\n\n");
    (void)checksum_15(crc15, data_ptr, len_of_data, REVERSE_BIT_ORDER, EXTRA_BIT_ADDED);

    (void)check_crc(data_ptr, len_of_data, REVERSE_BIT_ORDER, EXTRA_BIT_ADDED);

    // Messing up the message:
    data_ptr[1] = 'a'; //     message[1] = 'a';
    (void)printf("\nThe message has been changed. ");
    // Validate the message.
    // If the remainder is zero print "The data is OK\n";
    // otherwise print "The data is not OK\n"

    (void)check_crc(data_ptr, len_of_data, REVERSE_BIT_ORDER, EXTRA_BIT_ADDED);

    return 0;
}

uint16_t crc_15(const uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit)
{
    const uint16_t divisor = POLYNOMIAL;

    uint16_t buf_2B;
    uint16_t crc;
    uint8_t *p_byte = arr;
    uint8_t mask = rev ? MASK8_W_ONLY_LSB_EQ_1 : MASK8_W_ONLY_MSB_EQ_1;

    const uint8_t n_bits_in_buf = BYTES_TO_BITS(sizeof(buf_2B));
    const uint8_t n_steps = BYTES_TO_BITS(n_arr + sizeof(crc));

    for (uint8_t i = 0; i < n_steps + xtra_bit; ++i)
    {
        if (i >= n_bits_in_buf && IS_MSB_OF_16_BIT_VALUE_EQ_1(buf_2B))
            buf_2B = buf_2B ^ divisor;

        buf_2B = (buf_2B << 1) | ((*p_byte & mask) ? 0x0001U : 0x0000U);

        if (rev ? mask == MASK8_W_ONLY_MSB_EQ_1 : mask == MASK8_W_ONLY_LSB_EQ_1)
        {
            mask = rev ? MASK8_W_ONLY_LSB_EQ_1 : MASK8_W_ONLY_MSB_EQ_1;
            ++p_byte;
        }
        else
            mask = rev ? mask << 1 : mask >> 1;
    }

    return buf_2B >> 1;
}

int checksum_15(const uint16_t crc15, uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit)
{
    const uint8_t n_bytes_in_crc = sizeof(crc15);

    union
    {
        uint16_t x16;
        uint8_t x8[sizeof(uint16_t) / sizeof(uint8_t)];
    } buf;

    buf.x16 = xtra_bit ? crc15 : crc15 << 1;

    if (rev)
        for (uint8_t i = 0; i < n_bytes_in_crc; ++i)
            for (uint8_t j = 0; j < N_BITS_IN_BYTE; ++j)
            {
                arr[n_arr + i] = (arr[n_arr + i] << 1) | GET_LSB_OF_BYTE(buf.x8[1 - i]);
                buf.x8[1 - i] >>= 1;
            }
    else
    {
        arr[n_arr] = buf.x8[1];
        arr[n_arr + 1] = buf.x8[0];
    }

    return 0;
}

static inline uint8_t check_crc(const uint8_t *arr, const uint8_t n_arr, const uint8_t rev, const uint8_t xtra_bit)
{
    uint16_t crc15;
    uint8_t crc_ok;

    (void)printf("Checking message \"%.*s\" + CRC:\n", n_arr, arr);
    if ((crc15 = crc_15(arr, n_arr, rev, xtra_bit)))
    {
        (void)printf("Checksum = %#x. The data is not OK\n", crc15);
        crc_ok = 0;
    }
    else
    {
        (void)printf("Checksum = %#x. The data is OK\n", crc15);
        crc_ok = 1;
    }

    return crc_ok;
}

static inline uint8_t input_alt_message(const uint8_t *arr, const uint8_t n_arr)
{
    uint8_t *ptr = arr;

    (void)printf("Input a new message(max. % d characters will be accepted) or just press ENTER > ", n_arr);
    (void)fgets((char *)arr, n_arr + 2, stdin); // "\n\0" are input in any case and need 2 extra pos. in the array
                                                //  besides the actual data (these pos. are used also for the checksum)
    while (*(ptr++) != '\n')
        ;
    *(--ptr) = '\0'; // changing '\n' to '\0', the next char. is also '\0' because of string input

    return (uint8_t)(ptr - arr);
}

uint16_t prn16bin(const uint16_t num)
{
    const uint8_t n_bits_in_x = BYTES_TO_BITS(sizeof(num));
    const uint8_t n_bits_in_group = 4;
    uint16_t mask = MASK16_W_ONLY_MSB_EQ_1;

    for (int8_t i = n_bits_in_x - 1; i >= 0; --i)
    {
        printf("%d%s", num & mask ? 1 : 0, (i > 0 && i % n_bits_in_group == 0) ? "_" : "");
        mask >>= 1;
    }

    return num;
}
