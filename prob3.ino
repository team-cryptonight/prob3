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
const u32 calculated_s_box[256] = {
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

// void byte_to_bit(u8 *in, u8 *out, u32 size_bit)
// {
//     u32 size_byte = size_bit / 8;
//     u32 i, j;
//     u8 bit_selector = 1;
//     for (i = 0; i < size_byte; i++)
//     {
//         bit_selector = 1;
//         for (j = 0; j < 8; j++)
//         {
//             if (in[i] & bit_selector)
//             {
//                 out[i * 8 + j] = 1;
//             }
//             else
//             {
//                 out[i * 8 + j] = 0;
//             }
//             bit_selector = bit_selector << 1;
//         }
//     }
// }

// void bit_to_byte(u8 *in, u8 *out, u32 size_bit)
// {
//     u32 size_byte = size_bit / 8;
//     u32 i, j;
//     for (i = 0; i < size_byte; i++)
//     {
//         out[i] = 0;
//         for (j = 0; j < 8; j++)
//         {
//             out[i] = out[i] + (in[i * 8 + j] << j);
//         }
//     }
// }

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
    // u8 func2_bit_tmp1[32] = {
    //     0,
    // };
    // u8 func2_bit_tmp2[32] = {
    //     0,
    // };
    u8 tmp1, tmp2;
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
            // tmp1 = s_box[(func2_tmp[j] & 0xF)];
            // tmp2 = s_box[((func2_tmp[j] >> 4) & 0xF)];
            // func2_tmp[j] = (tmp1 + (tmp2 << 4)) ^ func2_key[j];
            func2_tmp[j] = calculated_s_box[func2_tmp[j]] ^ func2_key[j];
        }

        // byte_to_bit(func2_tmp, func2_bit_tmp1, DATA_SIZE * 8);
        // PERMUTATE_FUNC(func2_bit_tmp1, func2_bit_tmp2);
        // bit_to_byte(func2_bit_tmp2, func2_tmp, DATA_SIZE * 8);

        // byte-to-byte permutation
        permutate_func(func2_tmp, func2_tmp_aux);
        *(u32 *)func2_tmp = *(u32 *)func2_tmp_aux;
    }
    out[0] = func2_tmp[0];
    out[1] = func2_tmp[1];
    out[2] = func2_tmp[2];
    out[3] = func2_tmp[3];
}

u8 Matching(u8 *in1, u8 *in2)
{
    int i;
    u8 flag = 1;
    for (i = 0; i < 8; i++)
    {
        if (in1[i] != in2[i])
        {
            flag = 0;
            return flag;
        }
    }
    return flag;
}

const char digit_pairs[201] = {
    "00102030405060708090"
    "01112131415161718191"
    "02122232425262728292"
    "03132333435363738393"
    "04142434445464748494"
    "05152535455565758595"
    "06162636465666768696"
    "07172737475767778797"
    "08182838485868788898"
    "09192939495969798999"};

void int_to_char(u32 in, u8 *out)
{
    int i = 0;
    u32 offset;
    for (int i = 0; i < 8; i += 2, in /= 100)
    {
        offset = in % 100;
        *(short *)(out + i) = *(short *)(digit_pairs + (offset << 1));
    }
}

u8 Cracking(u32 init_int, u8 *password, u8 *output, u8 *answer)
{
    u8 check;
    int_to_char(init_int, password);
    Func1(password, output);
    Func2(&password[4], &output[4]);
    check = Matching(output, answer);
    return check;
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

    Serial.println("-----------------");
    Serial.println(" BENCHMARK ");
    Serial.println("-----------------");
    u32 time1;
    u32 time2;
    time1 = millis();

    init_int = 0;
    for (i = 0; i < 99999999; i++)
    {
        check = Cracking(init_int, password, output, answer_bench);
        if (check)
        {
            Serial.print("Answer is ");
            Serial.println(init_int);
            break;
        }

        for (j = 0; j < 8; j++)
        {
            output[j] = 0;
        }
        init_int++;
    }
    time2 = millis();
    Serial.print(">> ");
    Serial.println((time2 - time1));
    Serial.println("-----------------");
}

void loop()
{
}
