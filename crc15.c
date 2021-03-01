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
 * @version 0.3
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

/**
 * @brief Printing a 16-bit number to stdout in binary form.
 * 
 * @param num - a 16-bit number
 * @return uint16_t (the input value)
 */
uint16_t prn16bin(uint16_t num)
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

/**
 * @brief Providing next bit from the array of bytes.
 *        Bytes are processed sequentially. 
 *        Bits in every byte may be processed in regular or reversed order.
 * 
 * @param arr - array of bytes
 * @param reset - resetting to the beginning of the array
 * @param rev - if 1 then bits in every byte are processed in reversed order.
 * @return uint8_t (0 or 1)
 */
uint8_t next_bit(uint8_t *arr, uint8_t reset, uint8_t rev)
{
    static uint8_t *p_byte;
    static uint8_t mask;
    static uint8_t bit_out;

    if (reset)
    {
        p_byte = arr;
        mask = rev ? MASK8_W_ONLY_LSB_EQ_1 : MASK8_W_ONLY_MSB_EQ_1;
    }
    bit_out = (*p_byte & mask) ? 0x01U : 0x00U;
    if (!reset)
    {
        if (rev ? mask == MASK8_W_ONLY_MSB_EQ_1 : mask == MASK8_W_ONLY_LSB_EQ_1)
        {
            mask = rev ? MASK8_W_ONLY_LSB_EQ_1 : MASK8_W_ONLY_MSB_EQ_1;
            ++p_byte;
        }
        else
            mask = rev ? mask << 1 : mask >> 1;
    }

    return bit_out;
}
/* ALT:
uint8_t next_bit(uint8_t *arr, uint8_t reset, uint8_t rev)
{
    static uint8_t *p_byte;
    static uint8_t i_counter;
    static uint8_t buf_1B;
    static uint8_t bit_out;

    if (reset)
    {
        buf_1B = *arr;
        i_counter = 0;
        p_byte = arr;
        bit_out = rev ? GET_LSB_OF_BYTE(buf_1B) : GET_MSB_OF_BYTE(buf_1B);
    }
    else
    {
        bit_out = rev ? GET_LSB_OF_BYTE(buf_1B) : GET_MSB_OF_BYTE(buf_1B);
        if (++i_counter == N_BITS_IN_BYTE)
        {
            i_counter = 0;
            buf_1B = *(++p_byte);
        }
        else
            buf_1B = rev ? buf_1B >> 1 : buf_1B << 1;
    }

    return bit_out;
}*/

/**
 * @brief Calculating CRC-15.
 * 
 * @param arr - array of bytes the CRC is calculated for; 2 extra zero bytes at the end should be already present !!
 * @param n_arr - the array size WITHOUT 2 extra zero bytes
 * @param rev - if 1, bits in the array bytes are processed in the reverse order (from LSB to MSB)
 * @return uint16_t (15-bit CRC value as a 16-bit value with MSB == 0)
 */
uint16_t crc_15(uint8_t *arr, uint8_t n_arr, uint8_t rev)
{
    const uint16_t divisor = POLYNOMIAL;

    uint16_t buf_2B;
    uint16_t crc;

    const uint8_t n_bits_in_buf = BYTES_TO_BITS(sizeof(buf_2B));
    const uint8_t n_steps = BYTES_TO_BITS(n_arr + sizeof(crc));

    (void)next_bit(arr, RESET, rev);

    for (uint8_t i = 0; i < n_steps; ++i)
    {
        if (i >= n_bits_in_buf && IS_MSB_OF_16_BIT_VALUE_EQ_1(buf_2B))
            buf_2B = buf_2B ^ divisor;

        buf_2B = (buf_2B << 1) | (uint16_t)next_bit(arr, NO_RESET, rev);
    }

    return buf_2B >> 1;
}

/**
 * @brief Adding checksum to array of bytes.
 * 
 * @param crc15 - 15-bit checksum (16-bit value with MSB == 0)
 * @param arr - the array the checksum is added to
 * @param n_arr - array size before checksum is added
 * @param rev - if 1, bits in both checksum bytes are added in the reversed order
 * @return int (0)
 */
int checksum_15(uint16_t crc15, uint8_t *arr, uint8_t n_arr, uint8_t rev)
{
    const uint8_t n_bytes_in_crc = sizeof(crc15);

    union
    {
        uint16_t x16;
        uint8_t x8[sizeof(uint16_t) / sizeof(uint8_t)];
    } buf;

    buf.x16 = crc15 << 1;

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

/**
 * @brief Calculating checksum and checking data integrity.
 * 
 * @return int (0)
 */
int main(void)
{
    uint8_t message[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0, 0}; // 15 zeros have been appended to the message
    //uint8_t message[] = {'A', 'B', 0, 0}; // 15 zeros have been appended to the message

    // Calculate the CRC and Checksum the message

    uint16_t crc15;

    crc15 = crc_15(message, sizeof(message) - sizeof(crc15), REVERSE_BIT_ORDER);
    (void)checksum_15(crc15, message, sizeof(message) - sizeof(crc15), REVERSE_BIT_ORDER);

    printf("CRC-15 as 15-bit remainder (MSB == 0) before adding to the message:  0x%x / 0b", crc15), (void)prn16bin(crc15), printf("\n\n");

    printf("Checking message \"%.*s\":\n", sizeof(message) - sizeof(crc15), message);
    if ((crc15 = crc_15(message, sizeof(message) - sizeof(crc15), REVERSE_BIT_ORDER)))
        printf("CRC = %#x. The data is not OK\n", crc15);
    else
        printf("CRC = %#x. The data is OK\n", crc15);

    message[1] = 'a';
    // Validate the message.
    // If the remainder is zero print "The data is OK\n";
    // otherwise print "The data is not OK\n"

    printf("\nThe message is changed. Checking message \"%.*s\":\n", sizeof(message) - sizeof(crc15), message);
    if ((crc15 = crc_15(message, sizeof(message) - sizeof(crc15), REVERSE_BIT_ORDER)))
        printf("CRC = %#x. The data is not OK\n", crc15);
    else
        printf("CRC = %#x. The data is OK\n", crc15);

    return 0;
}
