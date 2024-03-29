# 基本语法规则
## 一.基础语法
#### 1.
```
//行注释
```
#### 2.
```
/*
块注释
*/ 
```
3.
```
"字符串"
```

## 二.变量的定义
#### 1.var 定义局部变量

用法:
``` 
var a = 1, b, c
```

#### 2.set 定义全局变量  

用法: 
```
set g1 = "aa", g2, g3
``` 
**注意:本人很反感在代码中使用全局变量，本脚本语言的全局变量必须显示声明才能使用**  
*变量的默认值为`null`*  

## 三.复合数据结构的定义
#### 1.什么是复合数据结构
复合数据结构是一种数组与哈希表的结合体

#### 2.如何声明复合数据结构

用法:  
``` 
var data = 
{  
    key1 = 1, 
    key2 = "value1",
    "value2", 
    value3,  
    [key3] = 1,  
    ["key4"] = 2, 
    [key5()] = 3,
    key6 =  
    { 
        key7 = value4 
    }  
}  
```
*复合数据结构的`key`可以为除`null`以外的任意值*   
以上为例:`key1 = 1`等价于`["key1"] = 1`，代表往哈希表中插入键为`"key1"`的值`1`  
没有指定`key`时，代表往数组尾部插入值  
`[key3] = 1`代表以`key3`的值为`key`插入值  
**注意:数组下标从`0`开始**  

## 四.表达式求解语法
#### 1.一元运算
`!`逻辑取反  
`~`位取反  
`-`负号  

用法: 
```
var a = !true, b = ~1, c = -5 
```

#### 2.算术运算
`+`加法运算  
`-`减法运算  
`*`乘法运算  
`/`除法运算  
`%`求模运算   

用法:
```
var a = 1 + 2 % 5 - 6 * (3 + 5) / 5
```

#### 3.位运算
`&`位与运算  
`|`位或运算  
`^`位异或运算  
`<<`位左移运算  
`>>`位右移运算  

用法1:
``` 
var a = 3 << 2 + -1 * ~-(5 * -5) - (6 >> 1)  
```
用法2: 
```
var a = 11 & ~(4 - 1) 
```
#### 4.关系运算
`>`大于运算  
`<`小于运算  
`==`等于运算  
`!=`不等于运算  
`>=`大于等于运算  
`<=小于等于运算 

用法:
```
var b = 1 + 1 > 3 - 2
```
#### 5.逻辑运算
`&&`逻辑并运算  
`||`逻辑或运算  

用法1:
```
var b = 1 > 3 || 1 != 0 && 1 == 1
```
用法2:
```
var bb = test(true) || test(false) 
```
提示:当`test(true)`返回真时,则后面的`test(false)`不会执行  

#### 6.字符串运算
`..`连接运算  

用法: 
```
var a = "aa"..(3 + 7)..test("abc").."bb"
```
#### 7.复合数据结构运算
用法:
```
function test(x)
{ 
    return x   
}   

var t = 
{
    {1, 2, 3},
    key = {4, 5, test}, 
    "abc", 
    ["abc"] = 10, 
    [test] = 1,                       
    [test(10)] = 1
} 

print(t.key[1] + t[0][0]) 
print(t.key[2]("aa"))   
print(t.abc)  
print(t[10]) 
```
#### 8.混合运算
```
var a = 10
function b(x) 
{
    return x  
}
```
用法:
```
var b = -(a - 10) % 3 >> ~b(2 & 1)
```

## 五.条件判断语法
#### 1.无分支
```
if (condi) 
{

}
```
#### 2.有分支
```
if (condi) 
{  

} 
else
{

} 
```
#### 3.多重条件判断
```
if (condi1)
{ 

}  
else if (condi2) 
{ 

}  
else 
{ 

}
```
注意:本人不喜欢省略大括号，所以类似`if (condi) print(condi)`这种语法不支持

## 六.循环语法
#### 1.for循环
```
for(var i = 0; i < 100; i = i + 1)  
{  

}  

var i = 0  
for (; i < 100; i = i + 1)  
{  

}  

var i = 0  
for (; ; i = i + 1)  
{  

}  

for (var i = 0; ; i = i + 1)  
{  

}  

for (;;)  
{  

} 
``` 
#### 2.while循环
```
while (true)  
{  

}  
```
#### 3.do循环
```
do  
{  

} when (true)  
```
## 七.跳转语法
#### 1.break跳出当前循环
```
for (var i = 0; ; i = i + 1)  
{  
    if (i >= 100)  
    {  
        break  
    }  
}  
```
#### 2.continue跳转到循环条件之前
```
var i = 100
while (i)  
{  
    i = i - 1  
    if (i & 1)  
    {  
        continue  
    }  
}  
```
## 八.函数语法
#### 1.函数定义
```
fucntion GetName(object)  
{  

}  
```
#### 2.函数递归
用法:以斐波那契数列为例  
```
function fibonacii(x)  
{  
    if (x < 2)  
    {  
        return 1  
    }  
    return fibonacii(x - 1) + fibonacii(x - 2)  
}  
```

## 九.总结
动手实践是检验学习成果的最好选择。该项目作为一个学习阶段的总结，
目前已经发现了很多不尽如人意的地方。通过这次检验，我在表达式语法
的解析上已经有了更好的实现思路。字节码的设计上本应花更多时间来思考，
很多实现现在想来有很大的进步空间，可以提升脚本的运行效率。但知识的
积累也是螺旋式上升的，就期待下一个学习阶段吧。
