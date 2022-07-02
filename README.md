# prob3
3번 문제 풀이

# Benchmark time
## Improvement
| Ref. Code | Opt. code |
| --------- | --------- |
| 15399ms   | 1927ms    |

## Profiling using clock (ref. code)
| function       | time(ms) | percentage |
| --------       | -------- | ---------- |
| ```int_to_char``` | 5975  | 39         |
| ```Func1```    | 460      | 3          |
| ```Func2```    | 8852     | 58         |
| ```matching``` | 33       | 0.2        |

Bottlenecks: ```int_to_char, Func2```

## Profiling using clock (opt. code)
| function       | time(ms) | percentage |
| --------       | -------- | ---------- |
| ```int_to_char``` | 146   | 7.3        |
| ```Func1```    | 179      | 8.9        |
| ```Func2```    | 1656     | 82         |
| ```matching``` | 32       | 1.6        |

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
// Func1: 8 bytes -> 8 bytes
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
