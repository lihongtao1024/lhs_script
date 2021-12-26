var a = true && false || true
var b1 = true, b2 = false, b3 = true
a = b1 && b2 || b3
a = check(true) && check(false) || check(true)

function check(x)
{
    return x
}