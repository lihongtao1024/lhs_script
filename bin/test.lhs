if ( 5 < 6)
{

}
else if ( 5< 7)
{

}
//������ע��

/*
��������ע��
*/

/*
ȫ�ֱ�������
���ھ���������Ų�ȫ�ֱ�������Ľű�bug,
��������ƵĽű�Ҫ��ȫ�ֱ���������ʽ����
ȫ�ֱ�����������Ϊ������������л�����
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
{
    var x = 5
    x = global1 * x
    if (x > 0)
    {
        x = x - 3
    }
    global1 = global2 / 2
}
else if (global1 == 3)
{
    if (global1> 1)
    {
        if (global1 < 2 )
        {
            if (global1 == 1.5)
            {
                global1 = 100
            }
            else if (global1 == 3.3)
            {
                global1 = 200
            }
        }
    }
}
else
{
    global1 = global1 - 20
    if (global1 < 0)
    {
        global1 = 1
    }
    global1 = global1 % 5
}

/*
push local[2]
push 20
great stack[-2], stack[-1]

jmpf 48
pushc 1
push 5
mov local[6], statck[-1]
push local[2]
push local[6]
mul stack[-2], stack[-1]
mov local[6], stack[-1]
popc
jmp 54

pushc 2
push local[2]
push 20
sub stack[-2], stack[-1]
mov local[2], stack[-1]
popc
nop
*/
/*
if (1 > 2)
{
    if (2 > 1)
    {

    }
}
else if ( 2 > 1)
{
    if (5 < 10)
    {
    }
}
*/
//else
//{
//    var i = 7
//}

/*
push 1
push 2
great stack[-2], stack[-1]
jmpf 0
pushc 1
push 2
push 1
great stack[-2], stack[-1]
jmpf 0
pushc 2
popc
jmp 0
popc

*/
/*
if (1 > 2)
{
    if (3 < 1)
    {

    }
    else
    {

    }
}
else if ( 2 > 1)
{
    if (3 == 1)
    {

    
}
else
{
    if (2 % 2)
    {

    }
    else if (2 * 2)
    {

    }
}
*/