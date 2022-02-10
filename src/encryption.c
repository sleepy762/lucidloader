#include "Encryption.h"

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
void ArrToMatrix(uint8_t arr[], uint8_t matrix[ROW][ROW])
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
void GroupKeys(uint8_t matrix[][ROW], uint8_t keys[ROUNDS+1][ROW][ROW])
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
uint8_t XorTime(uint8_t a)
{
    uint8_t shift = a << 1;

    //if a is not 0x80(128)
    if(a & 0x80)
    {
        return ((shift ^ 0x1B) & 0xFF);
    }
    
    return shift;

}

void XorArr(uint8_t a[], uint8_t b[], uint8_t ans[])
{
    uint8_t i = 0;

    for(i = 0; i < ROW; i++)
    {
        ans[i] = a[i] ^ b[i];
    }
}


//helps with shift rows to make it O(2n) and mudolar
void Rot(uint8_t s[], uint8_t k)
{
    uint8_t size = ROW;

    Rev(s, 0, size-k-1);
    Rev(s, size-k, size-1);
    Rev(s, 0, size-1);
}

//helps Rev func
void Rev(uint8_t s[], uint8_t l, uint8_t h)
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

//problems with the Rot func on uint8_t*
void DeepCop(uint8_t* a, uint8_t b[])
{
    uint8_t i = 0;
    
    for(i = 0; i < ROW; i++)
    {
        b[i] = a[i];
    }

}

//copy matrix
void MatrixCop(uint8_t a[][ROW], uint8_t b[][ROW])
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

//Enc dec func
void ExpandKey(uint8_t key[], uint8_t word[ROW *(ROUNDS + 1)][ROW])
{
    uint8_t i = ROW;
    uint8_t j = 0;

    //will use it just to make stuff easier
    uint8_t last[ROW];
    uint8_t tmp;

    ArrToMatrix(key, word);
    
    for(i = ROW; i < ROW *(ROUNDS + 1); i++)
    {
        DeepCop(word[i-1], last);
        
        if(i % ROW == 0)
        {
            //just a Rotation / a bug in Rot func
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
        
        XorArr(word[i-ROW], last, word[i]);

    }
    
}

void AddRoundKey(uint8_t state[][ROW], uint8_t key[][ROW])
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
void ShiftRows(uint8_t state[][ROW])
{
    uint8_t i = 1;
    uint8_t j = 0;

    uint8_t temp[ROW][ROW];

    //Rotate
    for(i = 0; i < ROW; i++)
    {
        for(j = 1; j < ROW; j++)
        {
            temp[i][j] = state[(j+i) % ROW][j];
        }
    }
    MatrixCop(state, temp);
}

//the same but 3 more times
void InvShiftRows(uint8_t state[][ROW])
{
    uint8_t i = 1;
    uint8_t j = 0;

    uint8_t temp[ROW][ROW];

    //Rotate
    for(i = 0; i < ROW; i++)
    {
        for(j = 1; j < ROW; j++)
        {
            temp[(j+i) % ROW][j] = state[i][j];
        }
    }
    MatrixCop(state, temp);
}

//cool mix col shit
void MixSingleCol(uint8_t col[])
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
        col[i] ^= fTmp ^ XorTime(col[i] ^ col[i+1]);
    }

    col[i] ^= fTmp ^ XorTime(col[i] ^ first);
}

//mix all
void MixCol(uint8_t matrix[][ROW])
{
    uint8_t i = 0;
    
    for(i = 0; i < ROW; i++)
    {
        MixSingleCol(matrix[i]);
    }
}

//xor trick couse why not 
void InvMixCol(uint8_t s[][ROW])
{
    uint8_t i = 0;
    uint8_t j = 0;
    //for better readablity 
    uint8_t u = 0;
    uint8_t v = 0;

    for(i = 0; i < ROW; i++)
    {
        //need to be changed for more modular coding(know only suppurt aes128)
        u = XorTime(XorTime(s[i][0] ^ s[i][2]));
        v = XorTime(XorTime(s[i][1] ^ s[i][3]));
        
        for(j = 0; j < ROW; j++)
        {
            //to skip modulo check 
            //first one will be even and second will be odd(no reading error becouse the matrix is always even)
            s[i][j++] ^= u;
            s[i][j] ^= v;
        }
    } 
    MixCol(s);
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
void InvertSubBytes(uint8_t s[][ROW])
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


void Enc(uint8_t key[], uint8_t text[])
{
    //one odd round so we do it sepratliy
    uint8_t i = 0;
    
    uint8_t tmpKeys[ROW *(ROUNDS + 1)][ROW];
    uint8_t keys[ROUNDS+1][ROW][ROW];
    uint8_t state[ROW][ROW];

    uint8_t ciphertext[ROW*ROW];

    ExpandKey(key, tmpKeys);
    
    GroupKeys(tmpKeys, keys);
    
    ArrToMatrix(text, state);
    
    AddRoundKey(state, keys[0]);

    for(i = 0; i < ROUNDS -1; i++)
    {
        sub_bytes(state);
        ShiftRows(state);
        MixCol(state);
        AddRoundKey(state, keys[i]);
    }

    sub_bytes(state);
    ShiftRows(state);
    AddRoundKey(state, keys[i]);

    MatrixToArr(state, key);

}