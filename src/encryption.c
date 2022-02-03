#include "encryption.h"

//helpers
//this func will convert matrix to arr 
void MatrixToArr(uint8_t matrix[ROW][ROW], uint8_t arr[])
{
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < ROW; i++)
    {
        for(j = 0;j < ROW; j++)
        {
            arr[i * ROW + j] = matrix[i][j];
        }
    }
}

//this will do the oppsite
void arrToMatrix(uint8_t arr[], uint8_t matrix[ROW][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;

    for(i=0; i < ROW; i++)
    {
        for(j=0; j < ROW; j++)
        {
            matrix[i][j] = arr[k];
            k++;
        }
    }
}

//to group keys for easy use
void groupKeys(uint8_t matrix[][ROW], uint8_t keys[ROUNDS+1][ROW][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;

    for(i = 0; i < ROUNDS+1; i++)
    {
        for(j = 0; j < ROW; j++)
        {
            for(k = 0; k < ROW; k++)
            {
                keys[i][j][k] = matrix[(i * ROW) + j][k];
            }
        }
    }
}

//xor help 
uint8_t xor_time(uint8_t a)
{
    uint8_t shift = a << 1;

    //if a is not 0x80(128)
    if(a & 0x80)
    {
        return ((shift ^ 0x1B) & 0xFF);
    }
    
    return shift;

}

void xor_arr(uint8_t a[], uint8_t b[], uint8_t ans[])
{
    uint8_t i = 0;

    for(i = 0; i < ROW; i++)
    {
        ans[i] = a[i] ^ b[i];
    }
}


//helps with shift rows to make it O(2n) and mudolar
void rot(uint8_t s[], uint8_t k)
{
    uint8_t size = ROW;

    rev(s, 0, size-k-1);
    rev(s, size-k, size-1);
    rev(s, 0, size-1);
}

//helps prev func
void rev(uint8_t s[], uint8_t l, uint8_t h)
{
    while(h > l)
        {
            //just simple fliping with o(1) aux space couse we go hard 
            s[h] += s[l];
            s[l] = s[h] - s[l];
            s[h] = s[h] - s[l];
            h--;
            l++;
        }
}

//problems with the rot func on uint8_t*
void deep_cop(uint8_t* a, uint8_t b[])
{
    uint8_t i = 0;
    
    for(i = 0; i < ROW; i++)
    {
        b[i] = a[i];
    }

}

//copy matrix
void matrix_cop(uint8_t a[][ROW], uint8_t b[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < ROW; i++)
    {
        for(j = 1; j < ROW; j++)
        {
            a[i][j] = b[i][j];
        }
    }
}

//enc dec func
void expandKey(uint8_t key[], uint8_t word[ROW *(ROUNDS + 1)][ROW])
{
    uint8_t i = ROW;
    uint8_t j = 0;

    //will use it just to make stuff easier
    uint8_t last[ROW];
    uint8_t tmp;

    arrToMatrix(key, word);
    
    for(i = ROW; i < ROW *(ROUNDS + 1); i++)
    {
        deep_cop(word[i-1], last);
        
        if(i % ROW == 0)
        {
            //just a rotation / a bug in rot func
            tmp = last[0];
            
            for(j = 0; j < ROW -1; j++)
            {
                last[j] = last[j+1];
            }

            last[j] = tmp;

            for(j = 0; j < ROW; j++)
            {
                //maybe i will split sbox func so i can use it also here
                last[j] = sub_box[last[j]];
            }

            last[0] ^= round_consts[i/ROW];
            
        }
        
        xor_arr(word[i-ROW], last, word[i]);

    }
    
}

void add_round_key(uint8_t state[][ROW], uint8_t key[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;
    
    for(i=0; i < ROW; i++)
    {
        for(j=0; j < ROW; j++)
        {
            state[i][j] = state[i][j] ^ key[i][j];
        }
    }
}

//just shift each row by it index
void shift_rows(uint8_t state[][ROW])
{
    uint8_t i = 1;
    uint8_t j = 0;

    uint8_t temp[ROW][ROW];

    //rotate
    for(i = 0; i < ROW; i++)
    {
        for(j = 1; j < ROW; j++)
        {
            temp[i][j] = state[(j+i) % ROW][j];
        }
    }
    matrix_cop(state, temp);
}

//the same but 3 more times
void inv_shift_rows(uint8_t state[][ROW])
{
    uint8_t i = 1;
    uint8_t j = 0;

    uint8_t temp[ROW][ROW];

    //rotate
    for(i = 0; i < ROW; i++)
    {
        for(j = 1; j < ROW; j++)
        {
            temp[(j+i) % ROW][j] = state[i][j];
        }
    }
    matrix_cop(state, temp);
}

//cool mix col shit
void mix_single_col(uint8_t col[])
{
    uint8_t i = 0;
    
    uint8_t fTmp = col[0];
    uint8_t first = col[0];

    for(i=1; i < ROW; i++)
    {
        fTmp ^= col[i];
    }

    for(i = 0; i < ROW - 1; i++)
    {
        col[i] ^= fTmp ^ xor_time(col[i] ^ col[i+1]);
    }

    col[i] ^= fTmp ^ xor_time(col[i] ^ first);
}

//mix all
void mix_col(uint8_t matrix[][ROW])
{
    uint8_t i = 0;
    
    for(i = 0; i < ROW; i++)
    {
        mix_single_col(matrix[i]);
    }
}

//xor trick couse why not 
void inv_mix_col(uint8_t s[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;
    //for better readablity 
    uint8_t u = 0;
    uint8_t v = 0;

    for(i = 0; i < ROW; i++)
    {
        //need to be changed for more modular coding(know only suppurt aes128)
        u = xor_time(xor_time(s[i][0] ^ s[i][2]));
        v = xor_time(xor_time(s[i][1] ^ s[i][3]));
        
        for(j = 0; j < ROW; j++)
        {
            //to skip modulo check 
            //first one will be even and second will be odd(no reading error becouse the matrix is always even)
            s[i][j++] ^= u;
            s[i][j] ^= v;
        }
    } 
    mix_col(s);
}


//sub bytes, its a global table only one len(128) atm
void sub_bytes(uint8_t s[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < ROW; i++)
    {
        for(j = 0; j < ROW; j++)
        {
            s[i][j] = sub_box[s[i][j]];
        }
    }
}

//invert
void invert_sub_bytes(uint8_t s[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < ROW; i++)
    {
        for(j = 0; j < ROW; j++)
        {
            s[i][j] = invert_sub_box[s[i][j]];
        }
    }
} 


void enc(uint8_t key[], uint8_t text[])
{
    //one odd round so we do it sepratliy
    uint8_t i = 0;
    
    uint8_t tmpKeys[ROW *(ROUNDS + 1)][ROW];
    uint8_t keys[ROUNDS+1][ROW][ROW];
    uint8_t state[ROW][ROW];

    uint8_t ciphertext[ROW*ROW];

    expandKey(key, tmpKeys);
    
    groupKeys(tmpKeys, keys);
    
    arrToMatrix(text, state);
    
    add_round_key(state, keys[0]);

    for(i = 0; i < ROUNDS -1; i++)
    {
        sub_bytes(state);
        shift_rows(state);
        mix_col(state);
        add_round_key(state, keys[i]);
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, keys[i]);

    MatrixToArr(state, key);

}