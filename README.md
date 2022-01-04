# 基本语法规则
## 一.基础语法
#### 1.//行注释
#### 2./*
块注释
*/
3."字符串"

## 二.变量的定义
#### 1.var 定义局部变量
用法: var a = 1, b, c

#### 2.set 定义全局变量
用法: set g1 = "aa", g2, g3
注意:本人很反感在代码中使用全局变量，本脚本语言的全局变量必须显示定义才能使用

## 三.表达式求解语法
#### 1.一元运算
!逻辑取反
~位取反
-负号
用法: var a = !true, b = ~1, c = -5

#### 2.数值运算
+加法运算
-减法运算
*乘法运算
/除法运算
%求模运算
&位与运算
|位或运算
^位异或运算
<<位左移运算
>>位右移运算
用法1: var a = 3 << 2 + -1 * ~-(5 * -5) - (6 >> 1)
用法2: var a = 11 & ~(4 - 1)

#### 3.关系运算
>大于运算
<小于运算
==等于运算
!=不等于运算
>=大于等于运算
<=小于等于运算
用法:var b = 1 + 1 > 3 - 2

#### 4.逻辑运算
&&逻辑并运算
||逻辑或运算
用法1:var b = 1 > 3 || 1 != 0 && 1 == 1
用法2:var bb = test(true) || test(false)
提示:当test(true)返回真时,则后面的test(false)不会执行

#### 5.字符串运算
..连接运算
用法: var a = "aa"..(3 + 7)..test("abc").."bb"

#### 6.混合运算
var a = 10
function b(x)
{
    return x
}
用法:var b = -(a - 10) % 3 >> ~b(2 & 1)

## 四.条件判断语法
#### 1.无分支
if (condi)
{

}

#### 2.有分支
if (condi)
{

}
else
{

}
#### 3.多重条件判断
if (condi1)
{

}
else if (condi2)
{

}
else
{

}
注意:本人不喜欢省略大括号，所以类似if (condi) print(condi)这种语法不支持

## 五.循环语法
#### 1.for循环
for(var i = 0; i < 100; i = i + 1)
{

}

var i = 0;
for (; i < 100; i = i + 1)
{

}

var i = 0;
for (; ; i = i + 1)
{

}

for (var i = 0; ; i = i + 1)
{

}

for (;;)
{

}
#### 2.while循环
while (true)
{

}

#### 3.do循环
do
{

} when (true)

## 六.跳转语法
#### 1.break跳出当前循环
for (var i = 0; ; i = i + 1)
{
    if (i >= 100)
    {
        break
    }
}

#### 2.continue跳转到循环条件之前
var i = 100
while (i)
{
    i = i - 1
    if (i & 1)
    {
        continue
    }
}

## 七.函数语法
#### 1.函数定义
fucntion GetName(object)
{

}

#### 2.函数递归
用法:以斐波那契数列为例
function fibonacii(x)
{
    if (x < 2)
    {
        return 1
    }

    return fibonacii(x - 1) + fibonacii(x - 2)
}
