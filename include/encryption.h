#pragma once
#include <uefi.h>
/*---------------------defines-----------------------*/
#define ROUNDS 10 //key rounds 
#define ROW 4
/*---------------------defines-----------------------*/



/*---------------------consts------------------------*/
extern const uint8_t sub_box[];
extern const uint8_t invert_sub_box[];
extern const uint8_t round_consts[];
/*---------------------consts------------------------*/



/*----------------------func-------------------------*/
//helpers
void MatrixToArr(uint8_t matrix[ROW][ROW], uint8_t arr[]);
void ArrToMatrix(uint8_t arr[], uint8_t matrix[ROW][ROW]);
void GroupKeys(uint8_t matrix[][ROW], uint8_t keys[ROUNDS+1][ROW][ROW]);

uint8_t XorTime(uint8_t a);
void XorArr(uint8_t a[], uint8_t b[], uint8_t ans[]);

void Rot(uint8_t s[], uint8_t k);
void Rev(uint8_t s[], uint8_t h, uint8_t l);
void DeepCop(uint8_t* a, uint8_t b[]);
void MatrixCop(uint8_t a[][ROW], uint8_t b[][ROW]);

//enc dec func
void ExpandKey(uint8_t key[], uint8_t word[ROW *(ROUNDS + 1)][ROW]);
void AddRoundKey(uint8_t state[][ROW], uint8_t key[][ROW]);

void ShiftRows(uint8_t state[][ROW]);
void InvShiftRows(uint8_t state[][ROW]);

void MixCol(uint8_t matrix[][ROW]);
void InvMixCol(uint8_t s[][ROW]);

void MixSingleCol(uint8_t col[]);

void SubBytes(uint8_t s[][ROW]);
void InvertSubBytes(uint8_t s[][ROW]);


void Enc(uint8_t key[], uint8_t text[]);
