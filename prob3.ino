#pragma GCC optimize("-O3")

// type definition
typedef unsigned char u8;
typedef unsigned short u16;

// rotation left function
#define ROL(X, Y) ((X << Y) | (X >> (8 - Y)))      // optimization
#define ROL_4BIT(X, Y) ((X << Y) | (X >> (4 - Y))) // optimization
#define T1(B, C, D, E) (B & C) | (D & E)
#define T2(F, G, H) (F ^ G ^ H)
#define DATA_SIZE 4
#define SIZE_TEXT 32

u8 init_constant[8] = {0xAA, 0x11, 0xBB, 0x22, 0xCC, 0x33, 0xDD, 0x44};
u8 init_constant2[4] = {0x12, 0x34, 0x56, 0x78};

// https://eprint.iacr.org/2017/622.pdf (GIFT block cipher)
u32 s_box[16] = {0x1, 0xa, 0x4, 0xc, 0x6, 0xf, 0x3, 0x9, 0x2, 0xd, 0xb, 0x7, 0x5, 0x0, 0x8, 0xe};
const u8 calculated_s_box[256] = {
    0x11, 0x1a, 0x14, 0x1c, 0x16, 0x1f, 0x13, 0x19, 0x12, 0x1d, 0x1b, 0x17, 0x15, 0x10, 0x18, 0x1e,
    0xa1, 0xaa, 0xa4, 0xac, 0xa6, 0xaf, 0xa3, 0xa9, 0xa2, 0xad, 0xab, 0xa7, 0xa5, 0xa0, 0xa8, 0xae,
    0x41, 0x4a, 0x44, 0x4c, 0x46, 0x4f, 0x43, 0x49, 0x42, 0x4d, 0x4b, 0x47, 0x45, 0x40, 0x48, 0x4e,
    0xc1, 0xca, 0xc4, 0xcc, 0xc6, 0xcf, 0xc3, 0xc9, 0xc2, 0xcd, 0xcb, 0xc7, 0xc5, 0xc0, 0xc8, 0xce,
    0x61, 0x6a, 0x64, 0x6c, 0x66, 0x6f, 0x63, 0x69, 0x62, 0x6d, 0x6b, 0x67, 0x65, 0x60, 0x68, 0x6e,
    0xf1, 0xfa, 0xf4, 0xfc, 0xf6, 0xff, 0xf3, 0xf9, 0xf2, 0xfd, 0xfb, 0xf7, 0xf5, 0xf0, 0xf8, 0xfe,
    0x31, 0x3a, 0x34, 0x3c, 0x36, 0x3f, 0x33, 0x39, 0x32, 0x3d, 0x3b, 0x37, 0x35, 0x30, 0x38, 0x3e,
    0x91, 0x9a, 0x94, 0x9c, 0x96, 0x9f, 0x93, 0x99, 0x92, 0x9d, 0x9b, 0x97, 0x95, 0x90, 0x98, 0x9e,
    0x21, 0x2a, 0x24, 0x2c, 0x26, 0x2f, 0x23, 0x29, 0x22, 0x2d, 0x2b, 0x27, 0x25, 0x20, 0x28, 0x2e,
    0xd1, 0xda, 0xd4, 0xdc, 0xd6, 0xdf, 0xd3, 0xd9, 0xd2, 0xdd, 0xdb, 0xd7, 0xd5, 0xd0, 0xd8, 0xde,
    0xb1, 0xba, 0xb4, 0xbc, 0xb6, 0xbf, 0xb3, 0xb9, 0xb2, 0xbd, 0xbb, 0xb7, 0xb5, 0xb0, 0xb8, 0xbe,
    0x71, 0x7a, 0x74, 0x7c, 0x76, 0x7f, 0x73, 0x79, 0x72, 0x7d, 0x7b, 0x77, 0x75, 0x70, 0x78, 0x7e,
    0x51, 0x5a, 0x54, 0x5c, 0x56, 0x5f, 0x53, 0x59, 0x52, 0x5d, 0x5b, 0x57, 0x55, 0x50, 0x58, 0x5e,
    0x01, 0x0a, 0x04, 0x0c, 0x06, 0x0f, 0x03, 0x09, 0x02, 0x0d, 0x0b, 0x07, 0x05, 0x00, 0x08, 0x0e,
    0x81, 0x8a, 0x84, 0x8c, 0x86, 0x8f, 0x83, 0x89, 0x82, 0x8d, 0x8b, 0x87, 0x85, 0x80, 0x88, 0x8e,
    0xe1, 0xea, 0xe4, 0xec, 0xe6, 0xef, 0xe3, 0xe9, 0xe2, 0xed, 0xeb, 0xe7, 0xe5, 0xe0, 0xe8, 0xee};

u8 func2_cache[8][4];
u8 func2_tag = 0xff;
u8 key_tag = 0xff;

u8 check_flag = 0;

u32 t_int_to_char = 0;
u32 t_func1 = 0;
u32 t_func2 = 0;

u32 t_convert_key = 0;
u32 t_setup_parallel = 0;
u32 t_convert_data = 0;
u32 t_increase_cnt = 0;
u32 t_s_box = 0;
u32 t_xor = 0;
u32 t_permutate = 0;

// 0x76543210 -> 0x65432107
void byte_permutation(u8 *inout)
{
    u8 tmp;
    tmp = inout[7];
    inout[7] = inout[6];
    inout[6] = inout[5];
    inout[5] = inout[4];
    inout[4] = inout[3];
    inout[3] = inout[2];
    inout[2] = inout[1];
    inout[1] = inout[0];
    inout[0] = tmp;
}

void Func1(u8 *in, u8 *out)
{
    u32 i = 0;
    u8 tmp1, tmp2;
    u8 func1_tmp[8] = {
        0,
    };
    // constant initialization
    // pre-calculation for one round
    func1_tmp[2] = 0x11;
    func1_tmp[3] = 0xBB;
    func1_tmp[4] = 0x22;
    func1_tmp[5] = 0xCC;
    func1_tmp[6] = 0x33;
    func1_tmp[7] = 0xDD;
    func1_tmp[0] = 0x44;
    func1_tmp[1] = 0x65 + in[0];
    func1_tmp[1] = ROL(func1_tmp[1], 1);

    for (i = 1; i < 16; i++)
    {
        // computation
        tmp1 = T1(func1_tmp[1], func1_tmp[2], func1_tmp[3], func1_tmp[4]);
        tmp2 = T2(func1_tmp[5], func1_tmp[6], func1_tmp[7]);
        func1_tmp[0] = func1_tmp[0] + tmp1 + tmp2;
        func1_tmp[0] = func1_tmp[0] + in[i % 4];
        func1_tmp[0] = ROL(func1_tmp[0], 1);
        // permutation
        byte_permutation(func1_tmp);
    }
    for (i = 0; i < 4; i++)
    {
        out[i] = func1_tmp[i] ^ func1_tmp[i + 4];
    }
}

// void PERMUTATE_FUNC(u8 *in, u8 *out)
// {
//     // u8 text_tmp[SIZE_TEXT];
//     // u32 i, j;
//     // for (i = 0; i < 4; i++)
//     // {
//     //     for (j = 0; j < 8; j++)
//     //     {
//     //         text_tmp[i * 8 + j] = in[i + j * 4];
//     //     }
//     // }
//     // for (i = 0; i < SIZE_TEXT; i++)
//     // {
//     //     out[i] = text_tmp[i];
//     // }

//     // pre-calculated permutation
//     out[0] = in[0];
//     out[1] = in[4];
//     out[2] = in[8];
//     out[3] = in[12];
//     out[4] = in[16];
//     out[5] = in[20];
//     out[6] = in[24];
//     out[7] = in[28];
//     out[8] = in[1];
//     out[9] = in[5];
//     out[10] = in[9];
//     out[11] = in[13];
//     out[12] = in[17];
//     out[13] = in[21];
//     out[14] = in[25];
//     out[15] = in[29];
//     out[16] = in[2];
//     out[17] = in[6];
//     out[18] = in[10];
//     out[19] = in[14];
//     out[20] = in[18];
//     out[21] = in[22];
//     out[22] = in[26];
//     out[23] = in[30];
//     out[24] = in[3];
//     out[25] = in[7];
//     out[26] = in[11];
//     out[27] = in[15];
//     out[28] = in[19];
//     out[29] = in[23];
//     out[30] = in[27];
//     out[31] = in[31];
// }

// byte-to-byte permutation
void permutate_func(u8 *in, u8 *out)
{
    u32 in_32 = *(u32 *)in;
    u32 out_32 = 0;

    int i, j;
    u8 init_mask;
    u32 in_mask;
    u32 out_mask;

    for (i = 0, init_mask = 0x01, out_mask = 0x01; i < 4; i++, init_mask <<= 1)
    {
        for (j = 0, in_mask = init_mask; j < 8; j++, in_mask <<= 4)
        {
            if (in_32 & in_mask)
                out_32 |= out_mask;

            out_mask <<= 1;
        }
    }

    *(u32 *)out = out_32;
}

void opt_permutate(u8 *in, u8 *out)
{
    u32 out_32 = 0;
    u32 o, e, v;

    o = in[0] & 0xaa;
    e = in[0] & 0x55;
    out_32 |= (o * 2359440 | ((e * 147465) >> 3)) & 0x03030303;

    o = in[1] & 0xaa;
    e = in[1] & 0x55;
    out_32 |= (o * 9437760 | ((e * 147465) >> 1)) & 0x0c0c0c0c;

    o = in[2] & 0xaa;
    e = in[2] & 0x55;
    out_32 |= (o * 37751040 | e * 294930) & 0x30303030;

    o = in[3] & 0xaa;
    e = in[3] & 0x55;
    out_32 |= (o * 151004160 | e * 1179720) & 0xc0c0c0c0;

    *(u32 *)out = out_32;
}

// {0, 1}^4 -> {0, 1}^4
// 8 bit register
// inplace
void bitslice_s_box(u8 *x) {
    u8 tmp;
    x[1] ^= (x[0] & x[2]);
    tmp = x[0] ^ (x[1] & x[3]);
    x[2] ^= (tmp | x[1]);
    x[0] = x[2] ^ x[3];
    x[1] ^= x[0];
    x[0] = ~x[0];
    x[2] ^= (tmp & x[1]);
    x[3] = tmp;
}

// {0, 1}^32 -> {0, 1}^32
// 8 bit register
// inplace
void bitslice_xor(u8 *x, u8 *y) {
    for (int i = 0; i < 32; i++) {
        x[i] ^= y[i];
    }
}

void bitslice_increase_cnt(u8 *bitslice_cnt, u8 cnt) {
    switch(cnt) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    case 11:
    case 13:
    case 15:
        bitslice_cnt[0] = 0xff;
        bitslice_cnt[8] = 0xff;
        bitslice_cnt[16] = 0xff;
        bitslice_cnt[24] = 0xff;
        break;
    case 2:
    case 6:
    case 10:
    case 14:
        bitslice_cnt[0] = 0;
        bitslice_cnt[1] = 0xff;
        bitslice_cnt[8] = 0;
        bitslice_cnt[9] = 0xff;
        bitslice_cnt[16] = 0;
        bitslice_cnt[17] = 0xff;
        bitslice_cnt[24] = 0;
        bitslice_cnt[25] = 0xff;
        break;
    case 4:
    case 12:
        bitslice_cnt[0] = 0;
        bitslice_cnt[1] = 0;
        bitslice_cnt[2] = 0xff;
        bitslice_cnt[8] = 0;
        bitslice_cnt[9] = 0;
        bitslice_cnt[10] = 0xff;
        bitslice_cnt[16] = 0;
        bitslice_cnt[17] = 0;
        bitslice_cnt[18] = 0xff;
        bitslice_cnt[24] = 0;
        bitslice_cnt[25] = 0;
        bitslice_cnt[26] = 0xff;
        break;
    case 8:
        bitslice_cnt[0] = 0;
        bitslice_cnt[1] = 0;
        bitslice_cnt[2] = 0;
        bitslice_cnt[3] = 0xff;
        bitslice_cnt[8] = 0;
        bitslice_cnt[9] = 0;
        bitslice_cnt[10] = 0;
        bitslice_cnt[11] = 0xff;
        bitslice_cnt[16] = 0;
        bitslice_cnt[17] = 0;
        bitslice_cnt[18] = 0;
        bitslice_cnt[19] = 0xff;
        bitslice_cnt[24] = 0;
        bitslice_cnt[25] = 0;
        bitslice_cnt[26] = 0;
        bitslice_cnt[27] = 0xff;
        break;
    default:
        break;
    }
}

// {0, 1}^32 -> {0, 1}^32
// 8 bit register
// results saved in y
void bitslice_permutate(u8 *x, u8 *y) {
    y[0] = x[0];
    y[1] = x[4];
    y[2] = x[8];
    y[3] = x[12];
    y[4] = x[16];
    y[5] = x[20];
    y[6] = x[24];
    y[7] = x[28];
    y[8] = x[1];
    y[9] = x[5];
    y[10] = x[9];
    y[11] = x[13];
    y[12] = x[17];
    y[13] = x[21];
    y[14] = x[25];
    y[15] = x[29];
    y[16] = x[2];
    y[17] = x[6];
    y[18] = x[10];
    y[19] = x[14];
    y[20] = x[18];
    y[21] = x[22];
    y[22] = x[26];
    y[23] = x[30];
    y[24] = x[3];
    y[25] = x[7];
    y[26] = x[11];
    y[27] = x[15];
    y[28] = x[19];
    y[29] = x[23];
    y[30] = x[27];
    y[31] = x[31];
}

// u8[8] -> u8[8]
void convert_bitslice(u8 *in, u8 *out) {
    u8 in_mask, out_mask;
    for (int i = 0, in_mask = 1; i < 8; i++, in_mask <<= 1) {
        out[i] = 0;
        for (int j = 0, out_mask = 1; j < 8; j++, out_mask <<= 1) {
            if (in[j] & in_mask) {
                out[i] |= out_mask;
            }
        }
    }
}

void convert_bitslice_key(u8 in, u8 *out) {
    u8 in_mask;
    for (int i = 0, in_mask = 1; i < 8; i++, in_mask <<= 1) {
        if (in & in_mask) {
            out[i] = 0xff;
        } else {
            out[i] = 0;
        }
    }
}

void setup_parallel(u8 *in, u8 *out2, u8 *out3) {
    u8 in3 = in[1];
    u8 in2 = in[0];

    out3[0] = in3;
    out2[0] = in2;

    for (int i = 1; i < 8; i++) {
        in2++;
        if (in2 > '9') {
            in2 = '0';
            in3++;
        }

        out2[i] = in2;
        out3[i] = in3;
    }
}

void int_to_char_2_digit (u8 in, u8 *out) {
    u8 digit = 0;

    if (in > 9) {
        digit = in < 50
                    ? (in < 30 ? (in < 20 ? 1 : 2) : (in < 40 ? 3 : 4))
                    : (in < 80 ? (in < 60 ? 5 : (in < 70 ? 6 : 7)) : (in < 90 ? 8 : 9));
        in -= digit * 10;
    }
    out[1] = digit + '0';
    out[0] = in + '0';
}

void Func2(u8 *in, u8 *out)
{
    u8 func2_tmp[4] = {
        0,
    };

    u8 func2_tmp_aux[4] = {
        0,
    };

    u8 func2_key[4] = {
        0,
    };

    u8 cnt;
    int i, j = 0;
    func2_key[0] = in[0] ^ 0x12;
    func2_key[1] = in[1] ^ 0x34;
    func2_key[2] = in[0] ^ 0x56;
    func2_key[3] = in[1] ^ 0x78;

    func2_tmp[0] = in[2];
    func2_tmp[1] = in[3];
    func2_tmp[2] = in[2];
    func2_tmp[3] = in[3];
    for (i = 0; i < 16; i++)
    {
        cnt = i;
        for (j = 0; j < 4; j++)
        {
            func2_key[j] = func2_key[j] ^ cnt;
            func2_tmp[j] = calculated_s_box[func2_tmp[j]] ^ func2_key[j];
        }

        // byte-to-byte permutation
        permutate_func(func2_tmp, func2_tmp_aux);
        *(u32 *)func2_tmp = *(u32 *)func2_tmp_aux;
    }
    out[0] = func2_tmp[0];
    out[1] = func2_tmp[1];
    out[2] = func2_tmp[2];
    out[3] = func2_tmp[3];
}

void bitslice_Func2(u8 *in, u8 *out)
{
    u8 bitslice_tmp[32] = {0, };
    u8 bitslice_tmp_aux[32] = {0, };
    u8 bitslice_key[32] = {0, };
    u8 bitslice_cnt[32] = {0, };

    u8 in2[8] = {0, };
    u8 in3[8] = {0, };

    u32 t1, t2;

    u8 *bitslice_tmp_ptr, *bitslice_tmp_aux_ptr, *tmptr;

    bitslice_tmp_ptr = bitslice_tmp;
    bitslice_tmp_aux_ptr = bitslice_tmp_aux;

    u8 index = (in[3] - '0') * 10 + (in[2] - '0');
    u8 tag_index = index >> 3;
    u8 block_index = index - (tag_index << 3);

    u8 tag_key = (in[1] - '0') * 10 + (in[0] - '0');

    if (func2_tag == tag_index && key_tag == tag_key) {
        goto func2_cache_hit;
    }

    u8 cnt;
    int i, j = 0;

    t1 = micros();
    convert_bitslice_key(in[0] ^ 0x12, bitslice_key);
    convert_bitslice_key(in[1] ^ 0x34, bitslice_key + 8);
    convert_bitslice_key(in[0] ^ 0x56, bitslice_key + 16);
    convert_bitslice_key(in[1] ^ 0x78, bitslice_key + 24);
    t2 = micros();
    t_convert_key += (t2 - t1);

    u8 index_char[2] = {0, };
    t1 = micros();
    int_to_char_2_digit(tag_index << 3, index_char);
    setup_parallel(index_char, in2, in3);
    t2 = micros();
    t_setup_parallel += (t2 - t1);

    t1 = micros();
    convert_bitslice(in2, bitslice_tmp);
    convert_bitslice(in3, bitslice_tmp + 8);
    memcpy(bitslice_tmp + 16, bitslice_tmp, sizeof(u8) * 8);
    memcpy(bitslice_tmp + 24, bitslice_tmp + 8, sizeof(u8) * 8);
    t2 = micros();
    t_convert_data += (t2 - t1);

    for (i = 0; i < 16; i++)
    {
        t1 = micros();
        cnt = i;
        bitslice_increase_cnt(bitslice_cnt, cnt);
        t2 = micros();
        t_increase_cnt += (t2 - t1);

        t1 = micros();
        bitslice_xor(bitslice_key, bitslice_cnt);
        t2 = micros();
        t_xor += (t2 - t1);

        t1 = micros();
        for (j = 0; j < 32; j += 4) {
            bitslice_s_box(bitslice_tmp_ptr + j);
        }
        t2 = micros();
        t_s_box += (t2 - t1);

        t1 = micros();
        bitslice_xor(bitslice_tmp_ptr, bitslice_key);
        t2 = micros();
        t_xor += (t2 - t1);

        t1 = micros();
        bitslice_permutate(bitslice_tmp_ptr, bitslice_tmp_aux_ptr);

        tmptr = bitslice_tmp_ptr;
        bitslice_tmp_ptr = bitslice_tmp_aux_ptr;
        bitslice_tmp_aux_ptr = tmptr;
        t2 = micros();
        t_permutate += (t2 - t1);
    }

    t1 = micros();
    convert_bitslice(bitslice_tmp_ptr, bitslice_tmp_aux_ptr);
    convert_bitslice(bitslice_tmp_ptr + 8, bitslice_tmp_aux_ptr + 8);
    convert_bitslice(bitslice_tmp_ptr + 16, bitslice_tmp_aux_ptr + 16);
    convert_bitslice(bitslice_tmp_ptr + 24, bitslice_tmp_aux_ptr + 24);
    t2 = micros();
    t_convert_data += (t2 - t1);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            func2_cache[j][i] = bitslice_tmp_aux_ptr[8 * i + j];
        }
    }

    func2_tag = tag_index;
    key_tag = tag_key;

func2_cache_hit:
    out[0] = func2_cache[block_index][0];
    out[1] = func2_cache[block_index][1];
    out[2] = func2_cache[block_index][2];
    out[3] = func2_cache[block_index][3];
}

u8 matching(u8 *in1, u8 *in2, u8 ret_true)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        if (in1[i] != in2[i])
        {
            return 0;
        }
    }
    return ret_true;
}

// converts 8 digit unsigned decimal integer to ascii string
void int_to_char(u32 in, u8 *out)
{
    u8 l, digit;
    u16 lo;

    digit = 0;
    if (in > 9999999)
    {
        digit = in < 50000000
                    ? (in < 30000000 ? (in < 20000000 ? 1 : 2) : (in < 40000000 ? 3 : 4))
                    : (in < 80000000 ? (in < 60000000 ? 5 : (in < 70000000 ? 6 : 7)) : (in < 90000000 ? 8 : 9));
        in -= digit * 10000000;
    }
    out[7] = digit + '0';

    digit = 0;
    if (in > 999999)
    {
        digit = in < 5000000
                    ? (in < 3000000 ? (in < 2000000 ? 1 : 2) : (in < 4000000 ? 3 : 4))
                    : (in < 8000000 ? (in < 6000000 ? 5 : (in < 7000000 ? 6 : 7)) : (in < 9000000 ? 8 : 9));
        in -= digit * 1000000;
    }
    out[6] = digit + '0';

    digit = 0;
    if (in > 99999)
    {
        digit = in < 500000
                    ? (in < 300000 ? (in < 200000 ? 1 : 2) : (in < 400000 ? 3 : 4))
                    : (in < 800000 ? (in < 600000 ? 5 : (in < 700000 ? 6 : 7)) : (in < 900000 ? 8 : 9));
        in -= digit * 100000;
    }
    out[5] = digit + '0';

    digit = 0;
    if (in > 9999)
    {
        digit = in < 50000
                    ? (in < 30000 ? (in < 20000 ? 1 : 2) : (in < 40000 ? 3 : 4))
                    : (in < 80000 ? (in < 60000 ? 5 : (in < 70000 ? 6 : 7)) : (in < 90000 ? 8 : 9));
        in -= digit * 10000;
    }
    out[4] = digit + '0';

    digit = 0;
    lo = in;
    if (lo > 999)
    {
        digit = lo < 5000
                    ? (lo < 3000 ? (lo < 2000 ? 1 : 2) : (lo < 4000 ? 3 : 4))
                    : (lo < 8000 ? (lo < 6000 ? 5 : (lo < 7000 ? 6 : 7)) : (lo < 9000 ? 8 : 9));
        lo -= digit * 1000;
    }
    out[3] = digit + '0';

    digit = 0;
    if (lo > 99)
    {
        digit = lo < 500
                    ? (lo < 300 ? (lo < 200 ? 1 : 2) : (lo < 400 ? 3 : 4))
                    : (lo < 800 ? (lo < 600 ? 5 : (lo < 700 ? 6 : 7)) : (lo < 900 ? 8 : 9));
        lo -= digit * 100;
    }
    out[2] = digit + '0';

    digit = 0;
    l = lo;
    if (l > 9)
    {
        digit = l < 50
                    ? (l < 30 ? (l < 20 ? 1 : 2) : (l < 40 ? 3 : 4))
                    : (l < 80 ? (l < 60 ? 5 : (l < 70 ? 6 : 7)) : (l < 90 ? 8 : 9));
        l -= digit * 10;
    }
    out[1] = digit + '0';

    out[0] = l + '0';
}

u8 Cracking(u32 init_int, u8 *password, u8 *output, u8 *answer)
{
    u8 check_l = check_flag & 0x01;
    u8 check_h = check_flag & 0x02;

    // u32 time1, time2;

    // time1 = micros();
    int_to_char(init_int, password);
    // time2 = micros();
    // t_int_to_char += (time2 - time1);

    // time1 = micros();
    if (!check_l)
    {
        Func1(password, output);
        check_l = matching(output, answer, 1);
    }
    // time2 = micros();
    // t_func1 += (time2 - time1);

    // time1 = micros();
    if (!check_h)
    {
        // Func2(&password[4], &output[4]);
        // FIX: why does cache returns same value?
        bitslice_Func2(&password[4], &output[4]);
        check_h = matching(&output[4], &answer[4], 2);
    }
    // time2 = micros();
    // t_func2 += (time2 - time1);

    check_flag = check_h + check_l;

    return check_l && check_h;
}

void encrypt(u32 plaintext, u8 *ciphertext)
{
    u8 password[8];
    int_to_char(plaintext, password);
    Func1(password, ciphertext);
    Func2(&password[4], &ciphertext[4]);
}

void setup()
{
    Serial.begin(9600); // open the serial port at 9600 bps:
    pinMode(LED_BUILTIN, OUTPUT);
    u8 password[8] = {
        0,
    };
    u8 output[8] = {
        0,
    };
    u8 answer_test[8] = {0x26, 0xce, 0x59, 0x80, 0x98, 0x73, 0x64, 0x16};
    u32 init_int = 12345678;
    u32 i, j = 0;
    u8 check = 0;
    Serial.println("-----------------");
    Serial.println(" TEST VECTOR ");
    Serial.println("-----------------");
    // TEST VECTOR
    check = Cracking(init_int, password, output, answer_test);

    if (check)
    {
        Serial.println(">> CORRECT");
    }
    else
    {
        Serial.println(">> WRONG");
    }
    // Warning: This is example. This test vector will be changed in evaluation.
    // password range: 00000000 ~ 99999999
    u8 answer_bench[8] = {0x2b, 0x46, 0x9d, 0x42, 0x7b, 0x8f, 0x11, 0x9e};

    // encrypt(99999999, answer_bench);

    Serial.println("-----------------");
    Serial.println(" BENCHMARK ");
    Serial.println("-----------------");
    u32 time1;
    u32 time2;
    time1 = millis();

    u32 step = 1000001;

    check_flag = 0;

    for (init_int = 0; init_int <= 99999999;)
    {
        check = Cracking(init_int, password, output, answer_bench);
        switch (check_flag)
        {
        case 1:
            step = 1000000;
            break;
        case 2:
            step = 1;
            break;
        case 3:
            Serial.print("Answer is ");
            Serial.println(init_int);
            goto found;
            break;
        default:
            break;
        }

        init_int += step;
        if (init_int > 99999999) {
            init_int -= (100000000 - 10000);
        }
    }
found:
    time2 = millis();
    Serial.print(">> ");
    Serial.println((time2 - time1));
    Serial.println("-----------------");
    Serial.print("t_convert_key\t");
    Serial.println(t_convert_key / 1000);
    Serial.print("t_setup_parallel\t");
    Serial.println(t_setup_parallel / 1000);
    Serial.print("t_convert_data\t");
    Serial.println(t_convert_data / 1000);
    Serial.print("t_increase_cnt\t");
    Serial.println(t_increase_cnt / 1000);
    Serial.print("t_xor\t\t");
    Serial.println(t_xor / 1000);
    Serial.print("t_s_box\t\t");
    Serial.println(t_s_box / 1000);
    Serial.print("t_permutate\t");
    Serial.println(t_permutate / 1000);
}

void loop()
{
}
