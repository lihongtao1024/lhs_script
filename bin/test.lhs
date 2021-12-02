//这是行注释

/*
这是区域注释
*/

/*
全局变量声明
由于经常帮别人排查全局变量引起的脚本bug,
所以我设计的脚本要求全局变量必须显式声明
全局变量的作用域为整个虚拟机运行环境中
*/
set global0 = 17
var global1 = "aaac"
var global2 = -global1 + 3.5 - (5 - ~1) * 6  + 2 / 2 - 1
var global3 = !global2+-1
--6
var global4 = 1 * global0
global1 = 55
//global3 + 3 + 1.2 * 7 - 6
global2 = "abc"
//wer

if (global1 > 20)
1{
    var x = 5
    x = global1 * x
}
else
{
    global1 = global1 - 20
}
