# prob3
3번 문제 풀이

# Benchmark time
## Improvement (answer = 12345)
| Ref. Code | Opt. code | Bitslice |
| --------- | --------- | -------- |
| 15399ms   | 70ms      | 89ms     |

## Profiling using clock (ref. code, answer = 12345)
| function       | time(ms) | percentage |
| --------       | -------- | ---------- |
| ```int_to_char``` | 5975  | 39         |
| ```Func1```    | 460      | 3          |
| ```Func2```    | 8852     | 58         |
| ```matching``` | 33       | 0.2        |

Bottlenecks: ```int_to_char, Func2```

## Profiling using clock (opt. code, answer = 99999999)
| function       | time(ms) | percentage |
| --------       | -------- | ---------- |
| ```int_to_char``` | 195   | 13         |
| ```Func1```    | 175      | 11         |
| ```Func2```    | 1186     | 76         |

## Profiling using clock (Bitslice, func2, answer = 99999999)
| operation      | time(ms)    | percentage |
| -------------  | ----------- | ---------- |
| ```convert key```     | 24   | 0.64       |
| ```setup parallel```  | 12   | 0.32       |
| ```convert data```    | 483  | 13         |
| ```increase cnt```    | 126  | 3.3        |
| ```xor```             | 1685 | 45         |
| ```s_box```           | 687  | 18         |
| ```permutate```       | 747  | 20         |

# Major changes
## ```int_to_char```
```c
// converts 8 digit decimal integer to ascii string
void int_to_char(u32 in, u8 *out)
{
    // ...
    digit = 0;
    if (in > 9999999)
    {
        digit = in < 50000000
                    ? (in < 30000000 ? (in < 20000000 ? 1 : 2) : (in < 40000000 ? 3 : 4))
                    : (in < 80000000 ? (in < 60000000 ? 5 : (in < 70000000 ? 6 : 7)) : (in < 90000000 ? 8 : 9));
        in -= digit * 10000000;
    }
    out[7] = digit + '0';
    //...
}
```
avr은 DIV 연산을 지원하지 않음  
branching 사용하여 최적화  
```in```이 충분히 작다면 ```u16, u8```로 cast하여 비교 연산 최적화

## ```Func1```
```c
// Func1: 4 bytes -> 4 bytes
void Func1(u8 *in, u8 *out)
{
    // ...
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

    // ...
}
```
16 라운드 중 첫 라운드에 대해 pre-calculation

## ```Func2```
```c
// Func2: 4 bytes -> 4 bytes
void Func2(u8 *in, u8 *out)
{
    // ...
    for (i = 0; i < 16; i++)
    {
        cnt = i;
        for (j = 0; j < 4; j++)
        {
            func2_key[j] = func2_key[j] ^ cnt;
            // use calculated s-box
            func2_tmp[j] = calculated_s_box[func2_tmp[j]] ^ func2_key[j];
        }

        // byte-to-byte permutation
        permutate_func(func2_tmp, func2_tmp_aux);
        *(u32 *)func2_tmp = *(u32 *)func2_tmp_aux;
    }
    // ...
}
```
### s-box
ref. 상위 4 bit, 하위 4 bit로 나누어 2번 lookup 후 shift  
opt. 8bit로 1번 lookup
추가 메모리 +240B
 
### permutation
ref. 4 bytes -> 32 bits -> 4 bytes의 과정  
opt. 4 bytes -> 4 bytes로 직접 치환
```c
void permutate_func(u8 *in, u8 *out)
{
    u32 in_32 = *(u32 *)in;
    u32 out_32 = 0;

    // ...

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
```

## ```Cracking```
```c
u8 Cracking(u32 init_int, u8 *password, u8 *output, u8 *answer)
{
    u8 check_l = check_flag & 0x01;
    u8 check_h = check_flag & 0x02;

    int_to_char(init_int, password);

    if (!check_l)
    {
        Func1(password, output);
        check_l = matching(output, answer, 1);
    }

    if (!check_h)
    {
        Func2(&password[4], &output[4]);
        check_h = matching(&output[4], &answer[4], 2);
    }

    check_flag = check_h + check_l;

    return check_l && check_h;
}
```
```Func1```은 ```password```의 아래 4자리, ```Func2```는 ```password```의 위 4자리를 사용  
전역 변수 ```check_flag``` 설정

## ```setup```
```c
    // ...
    u32 step = 10001;

    check_flag = 0;

    for (init_int = 0; init_int <= 99999999; init_int += step)
    {
        check = Cracking(init_int, password, output, answer_bench);
        switch (check_flag)
        {
        case 1: // Func1 cracking finished
            step = 10000;
            break;
        case 2: // Func2 cracking finished
            step = 1;
            break;
        case 3: // Cracking finished
            Serial.print("Answer is ");
            Serial.println(init_int);
            goto found;
            break;
        default:
            break;
        }
    }
found:
    // ...
```
```Cracking```이 변경하는 ```check_flag```를 사용하여 병렬화  
