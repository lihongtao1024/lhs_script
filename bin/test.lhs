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
1{
    var x = 5
    x = global1 * x
}
else
{
    global1 = global1 - 20
}
