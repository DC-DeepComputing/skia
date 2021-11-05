
| Place | Command | Comment |
| ----- | ------- | ------- |
|main_loop:
| |cmp N, 0
| |jle tail_loop
| |                             |// (float x, float y)
| |<b>v0 = load32 ptr0 		    |// load input values
| |<b>v1 = load32 ptr1   		|// load input values
| |
| | | // if (x < y) { return x; } else { return y; }
| |<s>v2 = splat 0 (0)
|↑|v3 = splat FFFFFFFF (nan)
| |<b>v4 = gt_f32 v1 v0         | // calculate if expression
| |<s>v0 = bit_and v0,v3        | // then branch
| |<b>v5 = bit_and v0 v4        | // else branch
| |<b>v6 = bit_xor v3 v4        | // make else mask
| |<b>v7 = select v6 v1 v5      | // if result based on mask
| |<b>v8 = bit_or v4 v6         | // return if mask
| |
| |<b>store32 ptr2 v7 		    | // store output value
| |                             
| |add ptr0, 32                 | // increment varying pointers by K 
| |add ptr1, 32
| |add ptr2, 32
| |sub N, K
| |jump main_loop
|tail_loop:
| |cmp N, 0
| |jle done
| |                             |// (float x, float y)
| |<b>v0 = load32 ptr0 		    |// load input values
| |<b>v1 = load32 ptr1   		|// load input values
| | | // if (x < y) { return x; } else { return y; }
| |<s>v2 = splat 0 (0)
|↑|v3 = splat FFFFFFFF (nan)
| |<b>v4 = gt_f32 v1 v0         | // if expression
| |<s>v0 = bit_and v0,v3        | // then branch
| |<b>v5 = bit_and v0 v4        | // else branch
| |<b>v6 = bit_xor v3 v4        | // make else mask
| |<b>v7 = select v6 v1 v5      | // if result based on mask
| |<b>v8 = bit_or v4 v6         | // return if mask
| |<b>store32 ptr2 v7 		    | // store output value
| |                             
| |add ptr0, 4                  | // increment varying pointers by 1
| |add ptr1, 4
| |add ptr2, 4
| | 
| |sub N, 1
| |jump tail_loop
|done:|


```
main_loop:
   v0 = load32 ptr0   // load input values
   v1 = load32 ptr1   // load input values
   v2 = load32 ptr2   // load input values
↑  v3 = splat 0 (0)
↑  v4 = splat FFFFFFFF (nan)
↑  v5 = splat 40000000 (2)
   v6 = mul_f32 v0 v5
↑  v7 = splat 40400000 (3)
   v8 = mul_f32 v1 v7
   v9 = fma_f32 v0 v5 v8
↑  v10 = splat 40800000 (4)
   v11 = mul_f32 v2 v10
   v12 = fma_f32 v2 v10 v9
↑  v13 = splat 40A00000 (5)
   v14 = mul_f32 v0 v13
↑  v15 = splat 40C00000 (6)
   v16 = mul_f32 v1 v15
   v17 = fma_f32 v0 v13 v16
↑  v18 = splat 40E00000 (7)
   v19 = mul_f32 v2 v18
   v20 = fma_f32 v2 v18 v17
   v21 = div_f32 v12 v20
↑  v22 = splat 41400000 (12)
   v23 = mul_f32 v0 v22
↑  v24 = splat 41500000 (13)
   v25 = mul_f32 v1 v24
   v26 = fma_f32 v0 v22 v25
↑  v27 = splat 41600000 (14)
   v28 = mul_f32 v2 v27
   v29 = fma_f32 v2 v27 v26
↑  v30 = splat 41700000 (15)
   v31 = mul_f32 v0 v30
↑  v32 = splat 41800000 (16)
   v33 = mul_f32 v1 v32
   v34 = fma_f32 v0 v30 v33
↑  v35 = splat 41880000 (17)
   v36 = mul_f32 v2 v35
   v37 = fma_f32 v2 v35 v34
   v38 = div_f32 v29 v37
   v39 = sub_f32 v21 v38
   store32 ptr3 v39   // store back output values
/* Tail */
   v0 = load32 ptr0   // load input values
   v1 = load32 ptr1   // load input values
   v2 = load32 ptr2   // load input values
   v6 = mul_f32 v0 v5
   v8 = mul_f32 v1 v7
   v9 = fma_f32 v0 v5 v8
   v11 = mul_f32 v2 v10
   v12 = fma_f32 v2 v10 v9
   v14 = mul_f32 v0 v13
   v16 = mul_f32 v1 v15
   v17 = fma_f32 v0 v13 v16
   v19 = mul_f32 v2 v18
   v20 = fma_f32 v2 v18 v17
   v21 = div_f32 v12 v20
   v23 = mul_f32 v0 v22
   v25 = mul_f32 v1 v24
   v26 = fma_f32 v0 v22 v25
   v28 = mul_f32 v2 v27
   v29 = fma_f32 v2 v27 v26
   v31 = mul_f32 v0 v30
   v33 = mul_f32 v1 v32
   v34 = fma_f32 v0 v30 v33
   v36 = mul_f32 v2 v35
   v37 = fma_f32 v2 v35 v34
   v38 = div_f32 v29 v37
   v39 = sub_f32 v21 v38
   store32 ptr3 v39   // store back output values
/* Done */

Program:
17 registers, 41 instructions:
0	r0 = splat 0 (0)
1	r0 = splat FFFFFFFF (nan)
2	r0 = splat 40000000 (2)
3	r1 = splat 40400000 (3)
4	r2 = splat 40800000 (4)
5	r3 = splat 40A00000 (5)
6	r4 = splat 40C00000 (6)
7	r5 = splat 40E00000 (7)
8	r6 = splat 41400000 (12)
9	r7 = splat 41500000 (13)
10	r8 = splat 41600000 (14)
11	r9 = splat 41700000 (15)
12	r10 = splat 41800000 (16)
13	r11 = splat 41880000 (17)
loop:
14	    r12 = load32 ptr0
15	    r13 = load32 ptr1
16	    r14 = load32 ptr2
17	    r0 = mul_f32 r12 r0
18	    r15 = mul_f32 r13 r1
19	    r15 = fma_f32 r12 r0 r15
20	    r0 = mul_f32 r14 r2
21	    r15 = fma_f32 r14 r2 r15
22	    r0 = mul_f32 r12 r3
23	    r16 = mul_f32 r13 r4
24	    r16 = fma_f32 r12 r3 r16
25	    r0 = mul_f32 r14 r5
26	    r16 = fma_f32 r14 r5 r16
27	    r16 = div_f32 r15 r16
28	    r0 = mul_f32 r12 r6
29	    r15 = mul_f32 r13 r7
30	    r15 = fma_f32 r12 r6 r15
31	    r0 = mul_f32 r14 r8
32	    r15 = fma_f32 r14 r8 r15
33	    r0 = mul_f32 r12 r9
34	    r13 = mul_f32 r13 r10
35	    r13 = fma_f32 r12 r9 r13
36	    r0 = mul_f32 r14 r11
37	    r13 = fma_f32 r14 r11 r13
38	    r13 = div_f32 r15 r13
39	    r13 = sub_f32 r16 r13
40	    store32 ptr3 r13

Process finished with exit code 0

