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
}
else
{
    global1 = global1 - 20
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