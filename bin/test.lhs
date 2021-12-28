
function echo(param)
{
    print(param)
    return param
}
/*
var v1 = 8, v2 = 5, v3 = 7, v4 = 6, v5 = 4, v6 = 9
set d = 8 + -(5 - -7) - 6
set dd = v1 + -(v2 - -v3) - v4
set ddd = echo(v1) + -(echo(v2) - -echo(v3)) - echo(v4)
set dddd = echo(v1) + -(echo(v2) - echo(-v3)) - echo(v4)
//-10
set c = 4 / 8 * -(5 - -7) - 6
set cc = 4 / v1 * -(v2 - -v3) - v4
set ccc = 4 / echo(v1) * -(echo(v2) - -echo(v3)) - echo(v4)
set cccc = echo(4) / echo(v1) * -(echo(v2) - echo(-v3)) - echo(v4)
//-12
set b = 8 * -(5 - -7) - 6
set bb = v1 * -(v2 - -v3) - 6
set bbb = echo(v1) * -(echo(v2) - -echo(v3)) - 6
set bbbb = echo(v1) * -(echo(v2) - echo(-v3)) - echo(6)
//-102
set a = -(6)
set aa = -(v4)
set aaa = -(echo(6))
set aaaa = echo(-(echo(v4)))
//-6
set z = -(5 - -7)
set zz = -(echo(v2) - -echo(v3))
set zzz = -(echo(v2) - echo(-v3))
set zzzz = echo(-(echo(v2) - -echo(v3)))
//-12
set y = -(5 + 3)
set yy = -(echo(v2) + echo(3))
set yyy = -(5 + 3)
set yyyy = echo(-(echo(v2) + echo(3)))
//-8
set x = -5 + 6 * 7 - 4 / 8 * -(5 - -7) - 6
set xx = -v2 + v4 * v3 - v5 / v1 * -(v2 - -v3) - v4
set xxx = echo(-v2) + echo(v4) * echo(v3) - echo(v5) / echo(v1) * echo(-(echo(v2) - echo(-v3))) - echo(v4)
set xxxx = -echo(v2) + echo(v4) * echo(v3) - echo(v5) / echo(v1) * -(echo(v2) - -echo(v3)) - echo(v4)
//37
set k = 9
set kk = v6
set kkk = echo(v6)
set kkkk = echo(9)
//9
set l = -(-(-(1*3-2)-6))
set ll = -(-(-(1*3-2)-v4))
set lll = -(-(echo(-echo((echo(1)*echo(3)-echo(2))))-echo(v4)))
set llll = -(-(-(echo(1)*echo(3)-echo(2))-echo(v4)))
//-7

print(d, "\n")
print(c, "\n")
print(b, "\n")
print(a, "\n")
print(z, "\n")
print(y, "\n")
print(x, "\n")
print(k, "\n")
print(l, "\n\n\n")

print(dd, "\n")
print(cc, "\n")
print(bb, "\n")
print(aa, "\n")
print(zz, "\n")
print(yy, "\n")
print(xx, "\n")
print(kk, "\n")
print(ll, "\n\n\n")

print(ddd, "\n")
print(ccc, "\n")
print(bbb, "\n")
print(aaa, "\n")
print(zzz, "\n")
print(yyy, "\n")
print(xxx, "\n")
print(kkk, "\n")
print(lll, "\n\n\n")

print(dddd, "\n")
print(cccc, "\n")
print(bbbb, "\n")
print(aaaa, "\n")
print(zzzz, "\n")
print(yyyy, "\n")
print(xxxx, "\n")
print(kkkk, "\n")
print(llll, "\n\n\n")
*/
/*
a = true && false || true
var b1 = true, b2 = false, b3 = true
a = b1 && b2 || b3
a = check(true) && check(false) || check(true)

var bt = true, bf = false
var b1 = false && true || true
var b2 = bf && bt || bt
*/
/*
var bt = true, bf = false, bx = false
var b1 = (!true || !(true && false))
var b22 = !true || !(echo(true) && bx)
var bb1 = (true || true && false)
var b2 = bf && (bt || bx)
var b222 = bf && (bt || bx && bf)
var b3 = !echo(bf) && (!echo(bt) || !echo(bt)) && echo(bf)
print(b1, "\n")
print(b22, "\n")
print(bb1, "\n")
print(b2, "\n")
print(b222, "\n")
print(b3, "\n")
*/
/*
push l[2]
call g[1], 1, 1
je false, 115

push l[1]
call g[1], 1, 1
je true, 120

push l[1]
call g[1], 1, 1

movs l[3]
*/

var bbb1 = true
var bbb2 = bbb1 || echo(false) || false
print(bbb2)