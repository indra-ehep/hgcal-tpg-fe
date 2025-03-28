/**********************************************************************
 Created on : 25/03/2025
 Purpose    : Test fixed point conversion
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include "ap_fixed.h"
#include <iostream>

int main(int argc, const char* argv[]) {

  //Follow details at,
  //https://docs.amd.com/r/en-US/ug1399-vitis-hls/Fixed-Point-Identifier-Summary?tocId=VrUU0M1dYUvYtuO8n1Yhmg
  
  // std::cout << "Please input a number: ";  
  // float x; //tested with 0.49994753607190034
  // std::cin >> x;
  
  double x = 0.49994753607190034; //tested with 0.49994753607190034
  float y =  x;
  
  ap_fixed<64,11>                       m_double(x);
  ap_fixed<32,8>                        m_float(y);
  ap_ufixed<32,20>			m_default(x); //takes the default value
  ap_ufixed<32,20,AP_RND>		m_round_plusinf(x);
  ap_ufixed<32,20,AP_RND_ZERO>		m_round_zero(x);
  ap_ufixed<32,20,AP_RND_MIN_INF>	m_round_minusinf(x);
  ap_ufixed<32,20,AP_RND_INF>		m_round_inf(x);
  ap_ufixed<32,20,AP_RND_CONV>		m_round_conv(x);
  ap_ufixed<32,20,AP_TRN>		m_trunc_minusinf(x); //Default
  ap_ufixed<32,20,AP_TRN_ZERO>		m_trunc_zero(x);

  std::cout << std::setw(18) << std::setprecision(17) << std::fixed;
  std::cout << "You have entered:                            " << x  << std::endl;
  std::cout << "C++ float:                                   " << y  << std::endl;
  std::cout << "HLS fixedpoint double:                       " << m_double.to_double() << " " << m_double.to_string() << std::endl;
  std::cout << "HLS fixedpoint float:                        " << m_float.to_double() << " " << m_float.to_string() << std::endl;
  std::cout << "Default result (AP_TRN):                     " << m_default.to_float() << " " << m_default.to_string() << std::endl;
  std::cout << "Rounded to plus infinity (AP_RND):           " << m_round_plusinf.to_float() << " " << m_round_plusinf.to_string() << std::endl;
  std::cout << "Rounded to zero (AP_RND_ZERO):               " << m_round_zero.to_float() << " " << m_round_zero.to_string() << std::endl;
  std::cout << "Rounded to minus infinity (AP_RND_MIN_INF):  " << m_round_minusinf.to_float() << " " << m_round_minusinf.to_string() << std::endl;
  std::cout << "Rounded to infinity (AP_RND_INF):            " << m_round_inf.to_float() << " " << m_round_inf.to_string() << std::endl;
  std::cout << "Convergent rounding (AP_RND_CONV):           " << m_round_conv.to_float() << " " << m_round_conv.to_string() << std::endl;
  std::cout << "Truncation to minus infinity (AP_TRN):       " << m_trunc_minusinf.to_float() << " " << m_trunc_minusinf.to_string() << std::endl;
  std::cout << "Truncation to zero (AP_TRN_ZERO):            " << m_trunc_zero.to_float() << " " << m_trunc_zero.to_string() << std::endl;
  
  // std::cout << "\n p1 : ";
  // for(int i=8-1;i>=0;i--) std::cout << p1[i] ;
  // std::cout << std::endl;
  
  float value1 = 1.25;
  //Check rounding cases
  //===================================
  ap_ufixed<16,8> a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11;
  
  a0.range(15,8) = 0b0011;
  a0.range(7,0) = 0x00 + 0x07;

  a1.range(15,8) = 0b0011;
  a1.range(7,0) = 0x00 + 0x0f;
  
  a2.range(15,8) = 0b0011;
  a2.range(7,0) = 0x10 + 0x07;
  
  a3.range(15,8) = 0b0011;
  a3.range(7,0) = 0x10 + 0x0f;
  
  a4.range(15,8) = 0b0011;
  a4.range(7,0) = 0x20 + 0x07;
  
  a5.range(15,8) = 0b0011;
  a5.range(7,0) = 0x20 + 0x0f;

  a6.range(15,8) = 0b0011;
  a6.range(7,0) = 0x30 + 0x07;

  a7.range(15,8) = 0b0011;
  a7.range(7,0) = 0x30 + 0x0f;

  a8.range(15,8) = 0b0011;
  a8.range(7,0) = 0x40 + 0x07;

  a9.range(15,8) = 0b0011;
  a9.range(7,0) = 0x40 + 0x0f;

  a10.range(15,8) = 0b0011;
  a10.range(7,0) = 0x50 + 0x07;

  a11.range(15,8) = 0b0011;
  a11.range(7,0) = 0x50 + 0x0f;

  ap_ufixed<4,1> p2 = value1;
  ap_ufixed<4,2> p3 = value1;
  ap_ufixed<4,3> p4 = value1;
  
  ap_ufixed<4,3, AP_RND> p5 = value1;  //AP_RND 
  ap_ufixed<8,4, AP_RND> p6 = a0;  //AP_RND
  ap_ufixed<8,4, AP_RND> p7 = a1;  //AP_RND
  ap_ufixed<8,4, AP_RND> p8 = a2;  //AP_RND
  ap_ufixed<8,4, AP_RND> p9 = a3;  //AP_RND
  ap_ufixed<8,4, AP_RND> p10 = a4;  //AP_RND
  ap_ufixed<8,4, AP_RND> p11 = a5;  //AP_RND
  ap_ufixed<8,4, AP_RND> p12 = a6;  //AP_RND
  ap_ufixed<8,4, AP_RND> p13 = a7;  //AP_RND
  ap_ufixed<8,4, AP_RND> p14 = a8;  //AP_RND
  ap_ufixed<8,4, AP_RND> p15 = a9;  //AP_RND
  ap_ufixed<8,4, AP_RND> p16 = a10;  //AP_RND
  ap_ufixed<8,4, AP_RND> p17 = a11;  //AP_RND
  
  ap_ufixed<8,4, AP_RND_ZERO> q6 = a0;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q7 = a1;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q8 = a2;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q9 = a3;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q10 = a4;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q11 = a5;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q12 = a6;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q13 = a7;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q14 = a8;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q15 = a9;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q16 = a10;  //AP_RND_ZERO
  ap_ufixed<8,4, AP_RND_ZERO> q17 = a11;  //AP_RND_ZERO

  ap_ufixed<8,4, AP_RND_MIN_INF> r6 = a0;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r7 = a1;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r8 = a2;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r9 = a3;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r10 = a4;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r11 = a5;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r12 = a6;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r13 = a7;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r14 = a8;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r15 = a9;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r16 = a10;  //AP_RND_MIN_INF
  ap_ufixed<8,4, AP_RND_MIN_INF> r17 = a11;  //AP_RND_MIN_INF

  ap_ufixed<8,4, AP_RND_INF> s6 = a0;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s7 = a1;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s8 = a2;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s9 = a3;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s10 = a4;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s11 = a5;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s12 = a6;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s13 = a7;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s14 = a8;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s15 = a9;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s16 = a10;  //AP_RND_INF
  ap_ufixed<8,4, AP_RND_INF> s17 = a11;  //AP_RND_INF

  ap_ufixed<8,4, AP_RND_CONV> t6 = a0;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t7 = a1;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t8 = a2;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t9 = a3;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t10 = a4;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t11 = a5;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t12 = a6;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t13 = a7;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t14 = a8;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t15 = a9;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t16 = a10;  //AP_RND_CONV
  ap_ufixed<8,4, AP_RND_CONV> t17 = a11;  //AP_RND_CONV

  ap_ufixed<8,4, AP_TRN> u6 = a0;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u7 = a1;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u8 = a2;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u9 = a3;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u10 = a4;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u11 = a5;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u12 = a6;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u13 = a7;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u14 = a8;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u15 = a9;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u16 = a10;  //AP_TRN
  ap_ufixed<8,4, AP_TRN> u17 = a11;  //AP_TRN

  ap_ufixed<8,4, AP_TRN_ZERO> v6 = a0;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v7 = a1;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v8 = a2;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v9 = a3;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v10 = a4;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v11 = a5;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v12 = a6;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v13 = a7;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v14 = a8;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v15 = a9;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v16 = a10;  //AP_TRN_ZERO
  ap_ufixed<8,4, AP_TRN_ZERO> v17 = a11;  //AP_TRN_ZERO

  std::cout << std::setw(20) << std::setprecision(10) << std::fixed;
  std::cout << "value1 : " << value1 << std::endl;
  std::cout << "ap_ufixed<16,8> a0:                " << a0  << ", in binary: "<< a0.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a1:                " << a1  << ", in binary: "<< a1.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a2:                " << a2  << ", in binary: "<< a2.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a3:                " << a3  << ", in binary: "<< a3.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a4:                " << a4  << ", in binary: "<< a4.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a5:                " << a5  << ", in binary: "<< a5.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a6:                " << a6  << ", in binary: "<< a6.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a7:                " << a7  << ", in binary: "<< a7.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a8:                " << a8  << ", in binary: "<< a8.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a9:                " << a9  << ", in binary: "<< a9.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a10:               " << a10  << ", in binary: "<< a10.to_string() << std::endl;
  std::cout << "ap_ufixed<16,8> a11:               " << a11  << ", in binary: "<< a11.to_string() << std::endl;
  //std::cout << "ap_ufixed<16,8> a5:                " << a4  << ", in binary: "<< a5.to_string() << std::endl;
  std::cout << "ap_ufixed<4,1> p2:                 " << p2  << ", in binary: "<< p2.to_string() << std::endl;
  std::cout << "ap_ufixed<4,2> p3:                 " << p3  << ", in binary: "<< p3.to_string() << std::endl;
  std::cout << "ap_ufixed<4,3> p4:                 " << p4  << ", in binary: "<< p4.to_string() << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<4,3, AP_RND> p5(value1): " << p5  << ", in binary: "<< p5.to_string() << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p6(a0):     " << p6  << ", in binary: "<< p6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << p6[3] << p6[2] << p6[1] << p6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p7(a1):     " << p7  << ", in binary: "<< p7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << p7[3] << p7[2] << p7[1] << p7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p8(a2):     " << p8  << ", in binary: "<< p8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << p8[3] << p8[2] << p8[1] << p8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p9(a3):     " << p9  << ", in binary: "<< p9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << p9[3] << p9[2] << p9[1] << p9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p10(a4):    " << p10  << ", in binary: "<< p10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << p10[3] << p10[2] << p10[1] << p10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p11(a5):    " << p11  << ", in binary: "<< p11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << p11[3] << p11[2] << p11[1] << p11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p12(a6):    " << p12  << ", in binary: "<< p12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << p12[3] << p12[2] << p12[1] << p12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p13(a7):    " << p13  << ", in binary: "<< p13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << p13[3] << p13[2] << p13[1] << p13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p14(a8):    " << p14  << ", in binary: "<< p14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << p14[3] << p14[2] << p14[1] << p14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p15(a9):    " << p15  << ", in binary: "<< p15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << p15[3] << p15[2] << p15[1] << p15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p16(a10):    " << p16  << ", in binary: "<< p16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << p16[3] << p16[2] << p16[1] << p16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND> p17(a11):    " << p17  << ", in binary: "<< p17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << p17[3] << p17[2] << p17[1] << p17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q6(a0):     " << q6  << ", in binary: "<< q6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << q6[3] << q6[2] << q6[1] << q6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q7(a1):     " << q7  << ", in binary: "<< q7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << q7[3] << q7[2] << q7[1] << q7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q8(a2):     " << q8  << ", in binary: "<< q8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << q8[3] << q8[2] << q8[1] << q8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q9(a3):     " << q9  << ", in binary: "<< q9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << q9[3] << q9[2] << q9[1] << q9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q10(a4):    " << q10  << ", in binary: "<< q10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << q10[3] << q10[2] << q10[1] << q10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q11(a5):    " << q11  << ", in binary: "<< q11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << q11[3] << q11[2] << q11[1] << q11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q12(a6):    " << q12  << ", in binary: "<< q12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << q12[3] << q12[2] << q12[1] << q12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q13(a7):    " << q13  << ", in binary: "<< q13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << q13[3] << q13[2] << q13[1] << q13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q14(a8):    " << q14  << ", in binary: "<< q14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << q14[3] << q14[2] << q14[1] << q14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q15(a9):    " << q15  << ", in binary: "<< q15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << q15[3] << q15[2] << q15[1] << q15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q16(a10):    " << q16  << ", in binary: "<< q16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << q16[3] << q16[2] << q16[1] << q16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_ZERO> q17(a11):    " << q17  << ", in binary: "<< q17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << q17[3] << q17[2] << q17[1] << q17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r6(a0):     " << r6  << ", in binary: "<< r6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << r6[3] << r6[2] << r6[1] << r6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r7(a1):     " << r7  << ", in binary: "<< r7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << r7[3] << r7[2] << r7[1] << r7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r8(a2):     " << r8  << ", in binary: "<< r8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << r8[3] << r8[2] << r8[1] << r8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r9(a3):     " << r9  << ", in binary: "<< r9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << r9[3] << r9[2] << r9[1] << r9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r10(a4):    " << r10  << ", in binary: "<< r10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << r10[3] << r10[2] << r10[1] << r10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r11(a5):    " << r11  << ", in binary: "<< r11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << r11[3] << r11[2] << r11[1] << r11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r12(a6):    " << r12  << ", in binary: "<< r12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << r12[3] << r12[2] << r12[1] << r12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r13(a7):    " << r13  << ", in binary: "<< r13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << r13[3] << r13[2] << r13[1] << r13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r14(a8):    " << r14  << ", in binary: "<< r14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << r14[3] << r14[2] << r14[1] << r14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r15(a9):    " << r15  << ", in binary: "<< r15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << r15[3] << r15[2] << r15[1] << r15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r16(a10):    " << r16  << ", in binary: "<< r16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << r16[3] << r16[2] << r16[1] << r16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_MIN_INF> r17(a11):    " << r17  << ", in binary: "<< r17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << r17[3] << r17[2] << r17[1] << r17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s6(a0):     " << s6  << ", in binary: "<< s6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << s6[3] << s6[2] << s6[1] << s6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s7(a1):     " << s7  << ", in binary: "<< s7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << s7[3] << s7[2] << s7[1] << s7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s8(a2):     " << s8  << ", in binary: "<< s8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << s8[3] << s8[2] << s8[1] << s8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s9(a3):     " << s9  << ", in binary: "<< s9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << s9[3] << s9[2] << s9[1] << s9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s10(a4):    " << s10  << ", in binary: "<< s10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << s10[3] << s10[2] << s10[1] << s10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s11(a5):    " << s11  << ", in binary: "<< s11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << s11[3] << s11[2] << s11[1] << s11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s12(a6):    " << s12  << ", in binary: "<< s12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << s12[3] << s12[2] << s12[1] << s12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s13(a7):    " << s13  << ", in binary: "<< s13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << s13[3] << s13[2] << s13[1] << s13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s14(a8):    " << s14  << ", in binary: "<< s14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << s14[3] << s14[2] << s14[1] << s14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s15(a9):    " << s15  << ", in binary: "<< s15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << s15[3] << s15[2] << s15[1] << s15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s16(a10):    " << s16  << ", in binary: "<< s16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << s16[3] << s16[2] << s16[1] << s16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_INF> s17(a11):    " << s17  << ", in binary: "<< s17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << s17[3] << s17[2] << s17[1] << s17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t6(a0):     " << t6  << ", in binary: "<< t6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << t6[3] << t6[2] << t6[1] << t6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t7(a1):     " << t7  << ", in binary: "<< t7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << t7[3] << t7[2] << t7[1] << t7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t8(a2):     " << t8  << ", in binary: "<< t8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << t8[3] << t8[2] << t8[1] << t8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t9(a3):     " << t9  << ", in binary: "<< t9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << t9[3] << t9[2] << t9[1] << t9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t10(a4):    " << t10  << ", in binary: "<< t10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << t10[3] << t10[2] << t10[1] << t10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t11(a5):    " << t11  << ", in binary: "<< t11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << t11[3] << t11[2] << t11[1] << t11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t12(a6):    " << t12  << ", in binary: "<< t12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << t12[3] << t12[2] << t12[1] << t12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t13(a7):    " << t13  << ", in binary: "<< t13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << t13[3] << t13[2] << t13[1] << t13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t14(a8):    " << t14  << ", in binary: "<< t14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << t14[3] << t14[2] << t14[1] << t14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t15(a9):    " << t15  << ", in binary: "<< t15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << t15[3] << t15[2] << t15[1] << t15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t16(a10):    " << t16  << ", in binary: "<< t16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << t16[3] << t16[2] << t16[1] << t16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_RND_CONV> t17(a11):    " << t17  << ", in binary: "<< t17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << t17[3] << t17[2] << t17[1] << t17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u6(a0):     " << u6  << ", in binary: "<< u6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << u6[3] << u6[2] << u6[1] << u6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u7(a1):     " << u7  << ", in binary: "<< u7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << u7[3] << u7[2] << u7[1] << u7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u8(a2):     " << u8  << ", in binary: "<< u8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << u8[3] << u8[2] << u8[1] << u8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u9(a3):     " << u9  << ", in binary: "<< u9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << u9[3] << u9[2] << u9[1] << u9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u10(a4):    " << u10  << ", in binary: "<< u10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << u10[3] << u10[2] << u10[1] << u10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u11(a5):    " << u11  << ", in binary: "<< u11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << u11[3] << u11[2] << u11[1] << u11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u12(a6):    " << u12  << ", in binary: "<< u12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << u12[3] << u12[2] << u12[1] << u12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u13(a7):    " << u13  << ", in binary: "<< u13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << u13[3] << u13[2] << u13[1] << u13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u14(a8):    " << u14  << ", in binary: "<< u14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << u14[3] << u14[2] << u14[1] << u14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u15(a9):    " << u15  << ", in binary: "<< u15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << u15[3] << u15[2] << u15[1] << u15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u16(a10):    " << u16  << ", in binary: "<< u16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << u16[3] << u16[2] << u16[1] << u16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN> u17(a11):    " << u17  << ", in binary: "<< u17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << u17[3] << u17[2] << u17[1] << u17[0] << std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v6(a0):     " << v6  << ", in binary: "<< v6.to_string() << ", float: " << a0[7] << a0[6] << a0[5] << a0[4] << "(" << a0[3] << ")" << "-->" << v6[3] << v6[2] << v6[1] << v6[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v7(a1):     " << v7  << ", in binary: "<< v7.to_string() << ", float: " << a1[7] << a1[6] << a1[5] << a1[4] << "(" << a1[3] << ")" << "-->" << v7[3] << v7[2] << v7[1] << v7[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v8(a2):     " << v8  << ", in binary: "<< v8.to_string() << ", float: " << a2[7] << a2[6] << a2[5] << a2[4] << "(" << a2[3] << ")" << "-->" << v8[3] << v8[2] << v8[1] << v8[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v9(a3):     " << v9  << ", in binary: "<< v9.to_string() << ", float: " << a3[7] << a3[6] << a3[5] << a3[4] << "(" << a3[3] << ")" << "-->" << v9[3] << v9[2] << v9[1] << v9[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v10(a4):    " << v10  << ", in binary: "<< v10.to_string() << ", float: " << a4[7] << a4[6] << a4[5] << a4[4] << "(" << a4[3] << ")" << "-->" << v10[3] << v10[2] << v10[1] << v10[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v11(a5):    " << v11  << ", in binary: "<< v11.to_string() << ", float: " << a5[7] << a5[6] << a5[5] << a5[4] << "(" << a5[3] << ")" << "-->" << v11[3] << v11[2] << v11[1] << v11[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v12(a6):    " << v12  << ", in binary: "<< v12.to_string() << ", float: " << a6[7] << a6[6] << a6[5] << a6[4] << "(" << a6[3] << ")" << "-->" << v12[3] << v12[2] << v12[1] << v12[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v13(a7):    " << v13  << ", in binary: "<< v13.to_string() << ", float: " << a7[7] << a7[6] << a7[5] << a7[4] << "(" << a7[3] << ")" << "-->" << v13[3] << v13[2] << v13[1] << v13[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v14(a8):    " << v14  << ", in binary: "<< v14.to_string() << ", float: " << a8[7] << a8[6] << a8[5] << a8[4] << "(" << a8[3] << ")" << "-->" << v14[3] << v14[2] << v14[1] << v14[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v15(a9):    " << v15  << ", in binary: "<< v15.to_string() << ", float: " << a9[7] << a9[6] << a9[5] << a9[4] << "(" << a9[3] << ")" << "-->" << v15[3] << v15[2] << v15[1] << v15[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v16(a10):    " << v16  << ", in binary: "<< v16.to_string() << ", float: " << a10[7] << a10[6] << a10[5] << a10[4] << "(" << a10[3] << ")" << "-->" << v16[3] << v16[2] << v16[1] << v16[0] << std::endl;
  std::cout << "ap_ufixed<8,4, AP_TRN_ZERO> v17(a11):    " << v17  << ", in binary: "<< v17.to_string() << ", float: " << a11[7] << a11[6] << a11[5] << a11[4] << "(" << a11[3] << ")" << "-->" << v17[3] << v17[2] << v17[1] << v17[0] << std::endl;
  //===================================
  
  //AP_RND:         a)Round the value to the nearest representable value for the specific ap_[u]fixed type
  //AP_RND_ZERO:    a)Round the value to the nearest representable value.
  //                b)Round towards zero.
  //AP_RND_MIN_INF: a)Round the value to the nearest representable value.
  //                b)Round towards minus infinity.
  //                  i)For positive values, delete the redundant bits.
  //                  ii)For negative values, add the least significant bits.
  //AP_RND_INF:     a)Round the value to the nearest representable value.
  //                b)The rounding depends on the least significant bit
  //                  i)For positive values, if the least significant bit is set, round towards plus infinity. Otherwise, round towards minus infinity.
  //                  ii)For negative values, if the least significant bit is set, round towards minus infinity. Otherwise, round towards plus infinity
  //AP_RND_CONV:    a)Round to the nearest representable value with "ties" rounding to even, that is, the least significant bit (after rounding) is forced to zero.
  //                b)A "tie" is the midpoint of two representable values and occurs when the bit following the least significant bit (after rounding) is 1 and all the bits below it are zero.
  //AP_TRN:         a)Always round the value towards minus infinity. [DEFAULT]
  //AP_TRN_ZERO:    a)For positive values, the rounding is the same as mode AP_TRN.
  //                b)For negative values, round towards zero.

  
  //=====================================================================
  
  //AP_SAT:      a)To the maximum value in case of overflow.
  //             b)To the negative maximum value in case of negative overflow. 
  //AP_SAT_ZERO: a)Force the value to zero in case of overflow, or negative overflow.
  //AP_SAT_SYM:  a)To the maximum value in case of overflow.
  //             b)To the minimum value in case of negative overflow.
  //               i)Negative maximum for signed ap_fixed types
  //               ii)Zero for unsigned ap_ufixed types
  //AP_WRAP:     a)Wrap the value around in case of overflow. [DEFAULT]

 
  //Check saturation cases
  //===================================
  x += ((1<<20) - 1) ; //tested with 0.49994753607190034 
  y =  x;
  
  ap_ufixed<32,20>			s_default(x);
  ap_ufixed<32,20,AP_RND, AP_SAT>	s_sat(x);
  ap_ufixed<32,20,AP_RND, AP_SAT_ZERO>	s_sat_zero(x);
  ap_ufixed<32,20,AP_RND, AP_SAT_SYM>	s_sat_sym(x);
  ap_ufixed<32,20,AP_RND, AP_WRAP>	s_wrap(x);

  std::cout << "==========================="<< std::endl;
  std::cout << "Saturation (double):  " << x  << std::endl;
  std::cout << "C++ float:            " << y  << std::endl;
  std::cout << "s_default:            " << s_default.to_float() << " " << s_default.to_string() << std::endl;
  std::cout << "s_sat:                " << s_sat.to_float() << " " << s_sat.to_string() << std::endl;
  std::cout << "s_sat_zero:           " << s_sat_zero.to_float() << " " << s_sat_zero.to_string() << std::endl;
  std::cout << "s_sat_sym:            " << s_sat_sym.to_float() << " " << s_sat_sym.to_string() << std::endl;
  std::cout << "s_wrap:               " << s_wrap.to_float() << " " << s_wrap.to_string() << std::endl;

  x++;
  y++;
  s_default = s_default + 1;
  s_sat = s_sat + 1;
  s_sat_zero = s_sat_zero + 1;
  s_sat_sym = s_sat_sym + 1;
  s_wrap = s_wrap + 1;

  std::cout << "==========================="<< std::endl;
  std::cout <<"increase by 1...."<< std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "Saturation (double):  " << x  << std::endl;
  std::cout << "C++ float:            " << y  << std::endl;
  std::cout << "s_default:            " << s_default.to_float() << " " << s_default.to_string() << std::endl;
  std::cout << "s_sat:                " << s_sat.to_float() << " " << s_sat.to_string() << std::endl;
  std::cout << "s_sat_zero:           " << s_sat_zero.to_float() << " " << s_sat_zero.to_string() << std::endl;
  std::cout << "s_sat_sym:            " << s_sat_sym.to_float() << " " << s_sat_sym.to_string() << std::endl;
  std::cout << "s_wrap:               " << s_wrap.to_float() << " " << s_wrap.to_string() << std::endl;

  x++;
  y++;
  s_default = s_default + 1;
  s_sat = s_sat + 1;
  s_sat_zero = s_sat_zero + 1;
  s_sat_sym = s_sat_sym + 1;
  s_wrap = s_wrap + 1;

  std::cout << "==========================="<< std::endl;
  std::cout <<"increase by 1...."<< std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "Saturation (double):  " << x  << std::endl;
  std::cout << "C++ float:            " << y  << std::endl;
  std::cout << "s_default:            " << s_default.to_float() << " " << s_default.to_string() << std::endl;
  std::cout << "s_sat:                " << s_sat.to_float() << " " << s_sat.to_string() << std::endl;
  std::cout << "s_sat_zero:           " << s_sat_zero.to_float() << " " << s_sat_zero.to_string() << std::endl;
  std::cout << "s_sat_sym:            " << s_sat_sym.to_float() << " " << s_sat_sym.to_string() << std::endl;
  std::cout << "s_wrap:               " << s_wrap.to_float() << " " << s_wrap.to_string() << std::endl;

  x++;
  y++;
  s_default = s_default + 1;
  s_sat = s_sat + 1;
  s_sat_zero = s_sat_zero + 1;
  s_sat_sym = s_sat_sym + 1;
  s_wrap = s_wrap + 1;

  std::cout << "==========================="<< std::endl;
  std::cout <<"increase by 1...."<< std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "Saturation (double):  " << x  << std::endl;
  std::cout << "C++ float:            " << y  << std::endl;
  std::cout << "s_default:            " << s_default.to_float() << " " << s_default.to_string() << std::endl;
  std::cout << "s_sat:                " << s_sat.to_float() << " " << s_sat.to_string() << std::endl;
  std::cout << "s_sat_zero:           " << s_sat_zero.to_float() << " " << s_sat_zero.to_string() << std::endl;
  std::cout << "s_sat_sym:            " << s_sat_sym.to_float() << " " << s_sat_sym.to_string() << std::endl;
  std::cout << "s_wrap:               " << s_wrap.to_float() << " " << s_wrap.to_string() << std::endl;

  x++;
  y++;
  s_default = s_default + 1;
  s_sat = s_sat + 1;
  s_sat_zero = s_sat_zero + 1;
  s_sat_sym = s_sat_sym + 1;
  s_wrap = s_wrap + 1;

  std::cout << "==========================="<< std::endl;
  std::cout <<"increase by 1...."<< std::endl;
  std::cout << "==========================="<< std::endl;
  std::cout << "Saturation (double):  " << x  << std::endl;
  std::cout << "C++ float:            " << y  << std::endl;
  std::cout << "s_default:            " << s_default.to_float() << " " << s_default.to_string() << std::endl;
  std::cout << "s_sat:                " << s_sat.to_float() << " " << s_sat.to_string() << std::endl;
  std::cout << "s_sat_zero:           " << s_sat_zero.to_float() << " " << s_sat_zero.to_string() << std::endl;
  std::cout << "s_sat_sym:            " << s_sat_sym.to_float() << " " << s_sat_sym.to_string() << std::endl;
  std::cout << "s_wrap:               " << s_wrap.to_float() << " " << s_wrap.to_string() << std::endl;
  //===================================
  
  return 0;
}
