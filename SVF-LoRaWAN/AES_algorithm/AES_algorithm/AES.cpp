#include "AES.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
using namespace std;

typedef struct {
    uint32_t eK[44], dK[44];    // encKey, decKey
    int Nr; // 10 rounds
}AesKey;

typedef unsigned char uint8_t;
#define BLOCKSIZE 16  //AES-128���鳤��Ϊ16�ֽ�

// uint8_t y[4] -> uint32_t x
#define LOAD32H(x, y) \
  do { (x) = ((uint32_t)((y)[0] & 0xff)<<24) | ((uint32_t)((y)[1] & 0xff)<<16) | \
             ((uint32_t)((y)[2] & 0xff)<<8)  | ((uint32_t)((y)[3] & 0xff));} while(0)

// uint32_t x -> uint8_t y[4]
#define STORE32H(x, y) \
  do { (y)[0] = (uint8_t)(((x)>>24) & 0xff); (y)[1] = (uint8_t)(((x)>>16) & 0xff);   \
       (y)[2] = (uint8_t)(((x)>>8) & 0xff); (y)[3] = (uint8_t)((x) & 0xff); } while(0)
// ��uint32_t x����ȡ�ӵ�λ��ʼ�ĵ�n���ֽ�
#define BYTE(x, n) (((x) >> (8 * (n))) & 0xff)

/* used for keyExpansion */
// �ֽ��滻Ȼ��ѭ������1λ
#define MIX(x) (((S[BYTE(x, 2)] << 24) & 0xff000000) ^ ((S[BYTE(x, 1)] << 16) & 0xff0000) ^ \
                ((S[BYTE(x, 0)] << 8) & 0xff00) ^ (S[BYTE(x, 3)] & 0xff))

// uint32_t xѭ������nλ
#define ROF32(x, n)  (((x) << (n)) | ((x) >> (32-(n))))

// uint32_t xѭ������nλ
#define ROR32(x, n)  (((x) >> (n)) | ((x) << (32-(n))))

/* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
// AES-128�ֳ���
static const uint32_t rcon[10] = {
        0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL, 0x10000000UL,
        0x20000000UL, 0x40000000UL, 0x80000000UL, 0x1B000000UL, 0x36000000UL
};

//define rootKey for every end-device
unsigned char key[4][16] = {
       {
           0x3d, 0x2e, 0x6d, 0xe2, 0xa1, 0x25, 0x17, 0xba,
           0xc5, 0xb3, 0x1b, 0xbd, 0x0e, 0x7e, 0x3b, 0x54
       },
       {
           0x3f, 0x26, 0x6a, 0xe2, 0xa1, 0x3c, 0x27, 0xc3,
           0x93, 0xb3, 0x1b, 0x9a, 0x5e, 0x7b, 0x3b, 0x25
       },
       {
           0x4d, 0x5f, 0xd3, 0xab, 0xc8, 0x25, 0x17, 0xba,
           0xc5, 0xb3, 0x1b, 0xbd, 0x0e, 0x7e, 0x3b, 0x54
       },
       {
           0x3f, 0x26, 0x6a, 0xe2, 0xa1, 0x3c, 0x27, 0xc3,
           0x93, 0xb3, 0x1b, 0x9a, 0x5e, 0x7b, 0x3b, 0x25
       }
};

// S��
unsigned char S[256] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

//��S��
unsigned char inv_S[256] = {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
        0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
        0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
        0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
        0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
        0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
        0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
        0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
        0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
        0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
        0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

/* copy in[16] to state[4][4] */
int loadStateArray(uint8_t(*state)[4], const uint8_t* in) {
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[j][i] = *in++;
        }
    }
    return 0;
}

/* copy state[4][4] to out[16] */
int storeStateArray(uint8_t(*state)[4], uint8_t* out) {
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            *out++ = state[j][i];
        }
    }
    return 0;
}

//��Կ��չ
int keyExpansion(const uint8_t* key, uint32_t keyLen, AesKey* aesKey) {

    if (NULL == key || NULL == aesKey) {
        printf("keyExpansion param is NULL\n");
        return -1;
    }

    if (keyLen != 16) {
        printf("keyExpansion keyLen = %d, Not support.\n", keyLen);
        return -1;
    }

    uint32_t* w = aesKey->eK;  //������Կ
    uint32_t* v = aesKey->dK;  //������Կ

    /* keyLen is 16 Bytes, generate uint32_t W[44]. */

    /* W[0-3] */
    int i;
    for (i = 0; i < 4; ++i) {
        LOAD32H(w[i], key + 4 * i);
    }

    /* W[4-43] */
    for (i = 0; i < 10; ++i) {
        w[4] = w[0] ^ MIX(w[3]) ^ rcon[i];
        w[5] = w[1] ^ w[4];
        w[6] = w[2] ^ w[5];
        w[7] = w[3] ^ w[6];
        w += 4;
    }

    w = aesKey->eK + 44 - 4;
    //������Կ����Ϊ������Կ����ĵ��򣬷���ʹ�ã���ek��11�����������з����dk��Ϊ������Կ
    //��dk[0-3]=ek[41-44], dk[4-7]=ek[37-40]... dk[41-44]=ek[0-3]
    int j;
    for (j = 0; j < 11; ++j) {

        for (i = 0; i < 4; ++i) {
            v[i] = w[i];
        }
        w -= 4;
        v += 4;
    }

    return 0;
}

// ����Կ��
int addRoundKey(uint8_t(*state)[4], const uint32_t* key) {
    uint8_t k[4][4];
    int i;
    int j;
    /* i: row, j: col */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            k[i][j] = (uint8_t)BYTE(key[j], 3 - i);  /* �� uint32 key[4] ��ת��Ϊ���� uint8 k[4][4] */
            state[i][j] ^= k[i][j];
        }
    }

    return 0;
}

//�ֽ��滻
int subBytes(uint8_t(*state)[4]) {
    /* i: row, j: col */
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = S[state[i][j]]; //ֱ��ʹ��ԭʼ�ֽ���ΪS�������±�
        }
    }

    return 0;
}

//���ֽ��滻
int invSubBytes(uint8_t(*state)[4]) {
    /* i: row, j: col */
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = inv_S[state[i][j]];
        }
    }
    return 0;
}

//����λ
int shiftRows(uint8_t(*state)[4]) {
    uint32_t block[4] = { 0 };

    /* i: row */
    int i;
    for (i = 0; i < 4; ++i) {
        //������ѭ����λ���Ȱ�һ��4�ֽ�ƴ��uint_32�ṹ����λ����ת�ɶ�����4���ֽ�uint8_t
        LOAD32H(block[i], state[i]);
        block[i] = ROF32(block[i], 8 * i);
        STORE32H(block[i], state[i]);
    }
    return 0;
}

//������λ
int invShiftRows(uint8_t(*state)[4]) {
    uint32_t block[4] = { 0 };
    /* i: row */
    int i;
    for (i = 0; i < 4; ++i) {
        LOAD32H(block[i], state[i]);
        block[i] = ROR32(block[i], 8 * i);
        STORE32H(block[i], state[i]);
    }
    return 0;
}

/* Galois Field (256) Multiplication of two Bytes */
// ���ֽڵ�٤�޻���˷�����
uint8_t GMul(uint8_t u, uint8_t v) {
    uint8_t p = 0;
    int i;
    for (i = 0; i < 8; ++i) {
        if (u & 0x01) {    //
            p ^= v;
        }

        int flag = (v & 0x80);
        v <<= 1;
        if (flag) {
            v ^= 0x1B; /* x^8 + x^4 + x^3 + x + 1 */
        }
        u >>= 1;
    }
    return p;
}

// �л��
int mixColumns(uint8_t(*state)[4]) {
    uint8_t tmp[4][4];
    uint8_t M[4][4] = { {0x02, 0x03, 0x01, 0x01},
                       {0x01, 0x02, 0x03, 0x01},
                       {0x01, 0x01, 0x02, 0x03},
                       {0x03, 0x01, 0x01, 0x02} };
    /* copy state[4][4] to tmp[4][4] */
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {  //٤�޻���ӷ��ͳ˷�
            state[i][j] = GMul(M[i][0], tmp[0][j]) ^ GMul(M[i][1], tmp[1][j])
                ^ GMul(M[i][2], tmp[2][j]) ^ GMul(M[i][3], tmp[3][j]);
        }
    }
    return 0;
}

// ���л��
int invMixColumns(uint8_t(*state)[4]) {
    uint8_t tmp[4][4];
    uint8_t M[4][4] = { {0x0E, 0x0B, 0x0D, 0x09},
                       {0x09, 0x0E, 0x0B, 0x0D},
                       {0x0D, 0x09, 0x0E, 0x0B},
                       {0x0B, 0x0D, 0x09, 0x0E} };  //ʹ���л�Ͼ���������
    int i, j;
    /* copy state[4][4] to tmp[4][4] */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = GMul(M[i][0], tmp[0][j]) ^ GMul(M[i][1], tmp[1][j])
                ^ GMul(M[i][2], tmp[2][j]) ^ GMul(M[i][3], tmp[3][j]);
        }
    }

    return 0;
}

// AES-128���ܽӿڣ�����keyӦΪ16�ֽڳ��ȣ����볤��Ӧ����16�ֽ���������
// ����������������볤����ͬ�����������ⲿΪ������ݷ����ڴ�
int aesEncrypt(const uint8_t* key, uint32_t keyLen, const uint8_t* pt, uint8_t* ct, uint32_t len) {

    AesKey aesKey;
    uint8_t* pos = ct;
    const uint32_t* rk = aesKey.eK;  //������Կָ��
    uint8_t out[BLOCKSIZE] = { 0 };
    uint8_t actualKey[16] = { 0 };
    uint8_t state[4][4] = { 0 };

    if (NULL == key || NULL == pt || NULL == ct) {
        printf("param err.\n");
        return -1;
    }

    if (keyLen > 16) {
        printf("keyLen must be 16.\n");
        return -1;
    }

    if (len % BLOCKSIZE) {
        printf("inLen is invalid.\n");
        return -1;
    }

    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  // ��Կ��չ
    int i;
    // ʹ��ECBģʽѭ�����ܶ�����鳤�ȵ�����
    for (i = 0; i < len; i += BLOCKSIZE) {
        // ��16�ֽڵ�����ת��Ϊ4x4״̬���������д���
        loadStateArray(state, pt);
        // ����Կ��
        addRoundKey(state, rk);
        int j;
        for (j = 1; j < 10; ++j) {
            rk += 4;
            subBytes(state);   // �ֽ��滻
            shiftRows(state);  // ����λ
            mixColumns(state); // �л��
            addRoundKey(state, rk); // ����Կ��
        }

        subBytes(state);    // �ֽ��滻
        shiftRows(state);  // ����λ
        // �˴��������л��
        addRoundKey(state, rk + 4); // ����Կ��

        // ��4x4״̬����ת��Ϊuint8_tһά�����������
        storeStateArray(state, pos);

        pos += BLOCKSIZE;  // ���������ڴ�ָ���ƶ�����һ������
        pt += BLOCKSIZE;   // ��������ָ���ƶ�����һ������
        rk = aesKey.eK;    // �ָ�rkָ�뵽��Կ��ʼλ��
    }
    return 0;
}

// AES128���ܣ� ����Ҫ��ͬ����
int aesDecrypt(const uint8_t* key, uint32_t keyLen, const uint8_t* ct, uint8_t* pt, uint32_t len) {
    AesKey aesKey;
    uint8_t* pos = pt;
    const uint32_t* rk = aesKey.dK;  //������Կָ��
    uint8_t out[BLOCKSIZE] = { 0 };
    uint8_t actualKey[16] = { 0 };
    uint8_t state[4][4] = { 0 };

    if (NULL == key || NULL == ct || NULL == pt) {
        printf("param err.\n");
        return -1;
    }

    if (keyLen > 16) {
        printf("keyLen must be 16.\n");
        return -1;
    }

    if (len % BLOCKSIZE) {
        printf("inLen is invalid.\n");
        return -1;
    }

    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  //��Կ��չ��ͬ����
    int i, j;
    for (i = 0; i < len; i += BLOCKSIZE) {
        // ��16�ֽڵ�����ת��Ϊ4x4״̬���������д���
        loadStateArray(state, ct);
        // ����Կ�ӣ�ͬ����
        addRoundKey(state, rk);

        for (j = 1; j < 10; ++j) {
            rk += 4;
            invShiftRows(state);    // ������λ
            invSubBytes(state);     // ���ֽ��滻��������˳����Եߵ�
            addRoundKey(state, rk); // ����Կ�ӣ�ͬ����
            invMixColumns(state);   // ���л��
        }

        invSubBytes(state);   // ���ֽ��滻
        invShiftRows(state);  // ������λ
        // �˴�û�����л��
        addRoundKey(state, rk + 4);  // ����Կ�ӣ�ͬ����

        storeStateArray(state, pos);  // ������������
        pos += BLOCKSIZE;  // ��������ڴ�ָ����λ���鳤��
        ct += BLOCKSIZE;   // ���������ڴ�ָ����λ���鳤��
        rk = aesKey.dK;    // �ָ�rkָ�뵽��Կ��ʼλ��
    }
    return 0;
}
/* For CMAC Calculation */
unsigned char const_Rb[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};
unsigned char const_Zero[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Basic Functions */

void xor_128(unsigned char* a, unsigned char* b, unsigned char* out)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}

void print_hex(unsigned char* buf, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0 && i != 0) printf("     ");
        printf("%02x", buf[i]);
        if ((i % 4) == 3) printf(" ");
        if ((i % 16) == 15) printf("\n");
    }
    if ((i % 16) != 0) printf("\n");
}
void print128(unsigned char* bytes)
{
    int j;
    for (j = 0; j < 16; j++) {
        printf("%02x", bytes[j]);
        if ((j % 4) == 3) printf(" ");
    }
}
void printMIC(unsigned char* bytes)
{
    int j;
    for (j = 0; j < 4; j++) {
        printf("%02x", bytes[j]);
        if ((j % 4) == 3) printf(" ");
    }
}

void print96(unsigned char* bytes)
{
    int         j;
    for (j = 0; j < 12; j++) {
        printf("%02x", bytes[j]);
        if ((j % 4) == 3) printf(" ");
    }
}

/* AES-CMAC Generation Function */

void leftshift_onebit(unsigned char* input, unsigned char* output)
{
    int i;
    unsigned char overflow = 0;

    for (i = 15; i >= 0; i--) {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80) ? 1 : 0;
    }
    return;
}

void generate_subkey(unsigned char* key, unsigned char* K1, unsigned
    char* K2)
{
    unsigned char L[16];
    unsigned char Z[16];
    unsigned char tmp[16];
    int i;

    for (i = 0; i < 16; i++) Z[i] = 0;

    aesEncrypt(key, 16, Z, L, 16);

    if ((L[0] & 0x80) == 0) { /* If MSB(L) = 0, then K1 = L << 1 */
        leftshift_onebit(L, K1);
    }
    else {    /* Else K1 = ( L << 1 ) (+) Rb */
        leftshift_onebit(L, tmp);
        xor_128(tmp, const_Rb, K1);
    }

    if ((K1[0] & 0x80) == 0) {
        leftshift_onebit(K1, K2);
    }
    else {
        leftshift_onebit(K1, tmp);
        xor_128(tmp, const_Rb, K2);
    }
    return;
}

void padding(unsigned char* lastb, unsigned char* pad, int length)
{
    int j;

    /* original last block */
    for (j = 0; j < 16; j++) {
        if (j < length) {
            pad[j] = lastb[j];
        }
        else if (j == length) {
            pad[j] = 0x80;
        }
        else {
            pad[j] = 0x00;
        }
    }
}

void AES_CMAC(unsigned char* key, unsigned char* input, int length,
    unsigned char* mac)
{
    unsigned char X[16], Y[16], M_last[16], padded[16];
    unsigned char K1[16], K2[16];
    int n, i, flag;
    generate_subkey(key, K1, K2);

    n = (length + 15) / 16;       /* n is number of rounds */

    if (n == 0) {
        n = 1;
        flag = 0;
    }
    else {
        if ((length % 16) == 0) { /* last block is a complete block */
            flag = 1;
        }
        else { /* last block is not complete block */
            flag = 0;
        }
    }

    if (flag) { /* last block is complete block */
        xor_128(&input[16 * (n - 1)], K1, M_last);
    }
    else {
        padding(&input[16 * (n - 1)], padded, length % 16);
        xor_128(padded, K2, M_last);
    }

    for (i = 0; i < 16; i++) X[i] = 0;
    for (i = 0; i < n - 1; i++) {
        xor_128(X, &input[16 * i], Y); /* Y := Mi (+) X  */
        aesEncrypt(key, 16, Y, X, 16);      /* X := AES-128(KEY, Y); */
    }

    xor_128(X, M_last, Y);
    aesEncrypt(key, 16, Y, X, 16);

    for (i = 0; i < 16; i++) {
        mac[i] = X[i];
    }
}

//int* aes128_cmac2(int keyIndex, int* msg, int len)
//{
//    unsigned char CMAC[16];
//    int MIC[4];
//    unsigned char message[16];
//    for (int i = 0; i < len; i++)
//    {
//        message[i] = msg[i];
//    }
//    AES_CMAC(key[keyIndex], message, len, CMAC);
//    for (int i = 0; i < 4; i++)
//    {
//        MIC[i] = CMAC[i];
//    }
//    return MIC;
//}
void aes128_cmac(int keyIndex, int* msg, int len,int* MIC)
{
    unsigned char CMAC[16];
    //int MIC[4];
    unsigned char message[16];
    for (int i = 0; i < len; i++)
    {
        message[i] = msg[i];
    }
    AES_CMAC(key[keyIndex], message, len, CMAC);
    for (int i = 0; i < 4; i++)
    {
        MIC[i] = CMAC[i];
    }
}
void aes128_cmac2(int* key, int* msg, int len, int* MIC)
{
    unsigned char CMAC[16];
    //int MIC[4];
    unsigned char message[16];
    unsigned char KEY[16];
    for (int i = 0; i < len; i++)
    {
        message[i] = msg[i];
    }
    for (int i = 0; i < 16; i++)
    {
        KEY[i] = key[i];
    }
    AES_CMAC(KEY, message, len, CMAC);
    for (int i = 0; i < 4; i++)
    {
        MIC[i] = CMAC[i];
    }
}

void Compute_SessionKey(int keyIndex, int AppNonce, int NetID, int DevNonce, int pad,int* ans)
{

    unsigned char msg[16];
    unsigned char session[16];
   /* int ans[16];*/
    msg[0] = AppNonce;
    msg[1] = NetID;
    msg[2] = DevNonce;
    for (int i = 3; i < 16; i++)
    {
        msg[i] = pad;
    }
    aesEncrypt(key[keyIndex], 16, msg, session, 16);
    for (int i = 0; i < 16; i++)
    {
        ans[i] = session[i];
    }
}
void aes128_encrypt(int keyIndex, int* data, int len, int pad, int* ans)
{
    unsigned char msg[16];
    unsigned char session[16];
    for (int i = 0; i < len; i++)
    {
        msg[i] = data[i];
    }
    for (int i = len; i < 16; i++)
    {
        msg[i] = pad;
    }
    aesEncrypt(key[keyIndex], 16, msg, session, 16);
    for (int i = 0; i < 16; i++)
    {
        ans[i] = session[i];
    }
}
//int* Compute_SessionKey2(int keyIndex,int AppNonce,int NetID,int DevNonce,int pad)
//{
//    
//    unsigned char msg[16];
//    unsigned char session[16];
//    int ans[16];
//    msg[0] = AppNonce;
//    msg[1] = NetID;
//    msg[2] = DevNonce;
//    for (int i = 3; i < 16; i++)
//    {
//        msg[i] = pad;
//    }
//    aesEncrypt(key[keyIndex], 16, msg, session, 16);
//    for (int i = 0; i < 16; i++)
//    {
//        ans[i] = session[i];
//    }
//    return ans;
//}
int main()
{
    int ans_mic[4];
    int msg[5] = { 11, 22, 33, 44, 55 };
    int key[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
    int appnonce = 19;
    int netid = 5;
    int devnonce = 13;
    int pad = 2;
    int nwksKey[16];
    aes128_cmac(0, msg, 5, ans_mic);
    printf("mic_answer is:  ");
    for (int i = 0; i < 4; i++)
    {
        printf("%d", ans_mic[i]);
    }
    printf("\n");
    Compute_SessionKey(0, appnonce, netid, devnonce, pad, nwksKey);
    printf("nwksKey is:  ");
    for (int i = 0; i < 16; i++)
    {
        printf("%d", nwksKey[i]);
    }
    aes128_cmac2(key, msg, 5, ans_mic);
    printf("new mic_answer is:  ");
    for (int i = 0; i < 4; i++)
    {
        printf("%d", ans_mic[i]);
    }
    printf("\n");
    return 0;

}