/*
@brief 一些实用的宏
*/
#ifndef __MACRO_PRACTICAL_HPP__
#define __MACRO_PRACTICAL_HPP__


#define MACRO_STRING(mac_s) #mac_s
// 打印宏,即获取宏展开后的字符串 
#define PRINT_MACRO(mac) MACRO_STRING(mac)

#define AUTO_COMMA , 
#define AUTO_ELEMENT_DEF(elem) AUTO_##elem
#define AUTO_ELEMENT_DEF_1()
#define AUTO_ELEMENT__(type,p,n,s) type p##n##s
#define AUTO_ELEMENT(type,p,n,s) AUTO_ELEMENT__(type,p,n,s)
#define AUTO_ELEMENT_0(type,p,s)
#define AUTO_ELEMENT_1(type,p,s) AUTO_ELEMENT__(type,p,1,)
#define AUTO_ELEMENT_2(type,p,s) AUTO_ELEMENT(type,p,2,s)AUTO_ELEMENT_1(type,p,s)
#define AUTO_ELEMENT_3(type,p,s) AUTO_ELEMENT(type,p,3,s)AUTO_ELEMENT_2(type,p,s)
#define AUTO_ELEMENT_4(type,p,s) AUTO_ELEMENT(type,p,4,s)AUTO_ELEMENT_3(type,p,s)
#define AUTO_ELEMENT_5(type,p,s) AUTO_ELEMENT(type,p,5,s)AUTO_ELEMENT_4(type,p,s)
#define AUTO_ELEMENT_6(type,p,s) AUTO_ELEMENT(type,p,6,s)AUTO_ELEMENT_5(type,p,s)
#define AUTO_ELEMENT_7(type,p,s) AUTO_ELEMENT(type,p,7,s)AUTO_ELEMENT_6(type,p,s)
#define AUTO_ELEMENT_8(type,p,s) AUTO_ELEMENT(type,p,8,s)AUTO_ELEMENT_7(type,p,s)
#define AUTO_ELEMENT_9(type,p,s) AUTO_ELEMENT(type,p,9,s)AUTO_ELEMENT_8(type,p,s)
#define AUTO_ELEMENT_10(type,p,s) AUTO_ELEMENT(type,p,10,s)AUTO_ELEMENT_9(type,p,s)
#define AUTO_ELEMENT_11(type,p,s) AUTO_ELEMENT(type,p,11,s)AUTO_ELEMENT_10(type,p,s)
#define AUTO_ELEMENT_12(type,p,s) AUTO_ELEMENT(type,p,12,s)AUTO_ELEMENT_11(type,p,s)
#define AUTO_ELEMENT_13(type,p,s) AUTO_ELEMENT(type,p,13,s)AUTO_ELEMENT_12(type,p,s)
#define AUTO_ELEMENT_14(type,p,s) AUTO_ELEMENT(type,p,14,s)AUTO_ELEMENT_13(type,p,s)
#define AUTO_ELEMENT_15(type,p,s) AUTO_ELEMENT(type,p,15,s)AUTO_ELEMENT_14(type,p,s)
#define AUTO_ELEMENT_16(type,p,s) AUTO_ELEMENT(type,p,16,s)AUTO_ELEMENT_15(type,p,s)
#define AUTO_ELEMENT_17(type,p,s) AUTO_ELEMENT(type,p,17,s)AUTO_ELEMENT_16(type,p,s)
#define AUTO_ELEMENT_18(type,p,s) AUTO_ELEMENT(type,p,18,s)AUTO_ELEMENT_17(type,p,s)
#define AUTO_ELEMENT_19(type,p,s) AUTO_ELEMENT(type,p,19,s)AUTO_ELEMENT_18(type,p,s)
#define AUTO_ELEMENT_20(type,p,s) AUTO_ELEMENT(type,p,20,s)AUTO_ELEMENT_19(type,p,s)
#define AUTO_ELEMENT_21(type,p,s) AUTO_ELEMENT(type,p,21,s)AUTO_ELEMENT_20(type,p,s)
#define AUTO_ELEMENT_22(type,p,s) AUTO_ELEMENT(type,p,22,s)AUTO_ELEMENT_21(type,p,s)
#define AUTO_ELEMENT_23(type,p,s) AUTO_ELEMENT(type,p,23,s)AUTO_ELEMENT_22(type,p,s)
#define AUTO_ELEMENT_24(type,p,s) AUTO_ELEMENT(type,p,24,s)AUTO_ELEMENT_23(type,p,s)
#define AUTO_ELEMENT_25(type,p,s) AUTO_ELEMENT(type,p,25,s)AUTO_ELEMENT_24(type,p,s)
#define AUTO_ELEMENT_26(type,p,s) AUTO_ELEMENT(type,p,26,s)AUTO_ELEMENT_25(type,p,s)
#define AUTO_ELEMENT_27(type,p,s) AUTO_ELEMENT(type,p,27,s)AUTO_ELEMENT_26(type,p,s)
#define AUTO_ELEMENT_28(type,p,s) AUTO_ELEMENT(type,p,28,s)AUTO_ELEMENT_27(type,p,s)
#define AUTO_ELEMENT_29(type,p,s) AUTO_ELEMENT(type,p,29,s)AUTO_ELEMENT_28(type,p,s)
#define AUTO_ELEMENT_30(type,p,s) AUTO_ELEMENT(type,p,30,s)AUTO_ELEMENT_29(type,p,s)
#define AUTO_ELEMENT_31(type,p,s) AUTO_ELEMENT(type,p,31,s)AUTO_ELEMENT_30(type,p,s)
#define AUTO_ELEMENT_32(type,p,s) AUTO_ELEMENT(type,p,32,s)AUTO_ELEMENT_31(type,p,s)
#define AUTO_ELEMENT_33(type,p,s) AUTO_ELEMENT(type,p,33,s)AUTO_ELEMENT_32(type,p,s)
#define AUTO_REPEAT(type,p,n) AUTO_ELEMENT_##n(type,p,AUTO_COMMA)
#define AUTO_REPEAT_S(type,p,n,s) AUTO_ELEMENT_##n(type,p,s) // 只具备单循环点能力

/*类函数模板宏*/
#define MEM_FUNCTION_TEM_0(func_return,class_type) template<class func_return,class class_type> 
#define MEM_FUNCTION_TEM(func_return,class_type,p,n) template<class func_return,class class_type,AUTO_REPEAT(class,p,n)> 

/*函数模析宏*/
#define FUNCTION_TEM_0(func_return) template<class func_return>
#define FUNCTION_TEM(func_return,p,n) template<class func_return,AUTO_REPEAT(class,p,n)>

/*数成员函数指针宏*/
#define MEM_FUNCTION_POINTER(func_return,class_type,pfunc,p,n) func_return(class_type::*pfunc)(AUTO_REPEAT(,p,n))
#define FUNCTION_POINTER(func_return,pfunc,p,n) func_return(*pfunc)(AUTO_REPEAT(,p,n))

/*函数指针类型*/
#define MEM_FUNCTION_POINTER_TYPE(func_return,class_type,p,n) func_return(class_type::*)(AUTO_REPEAT(,p,n))
#define FUNCTION_POINTER_TYPE(func_return,p,n) func_return(*)(AUTO_REPEAT(,p,n))

/*函数执行宏*/
#define MEM_FUNCTION_EXEC(pobj,pfunc,p,n) (pobj->*pfunc)(AUTO_REPEAT(,p,n));
#define FUNCTION_EXEC(pfunc,p,n) (pfunc)(AUTO_REPEAT(,p,n));

#endif
