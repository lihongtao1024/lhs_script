
/*aa(3)
var global1 = "aaac"
var global7 = (3 - 4) * 2
var global3 = 4 + ~-5 * 7 - (8 * (6 - 3)) % 2
var global2 = -global1 + 3.5 - aa(3, "bb", (bb(10)))-(5 - 1) - 6  + 2 / 2 - 1
var global4 = -global1 + 3.5 - (7-(5 * 1)) - 6  + 2 / 2 - 1
var global5 = 18 * 2 / (5 + 3) - (6 - 9) * 3 - 5
var global = true && (3 > 5)
var global6 = global * (5 --11) + 38*/

//var aa = 1 + 2 * 3 - (5 / 6)

/*function test_print(x, y, z)
{
    y = y + 2
    print(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    test_print_ex(x, y, z)
    return 6
}
*/
/*
set cc = 3.14
print("\ttest\n\"AAAA\"\n")

print(test_print("aa'\"aa'\n\n", 12 / 6, 33 / 5))
test_print("bbbb", 22, 44)

function test_print(x, y, z)
{
    var xx = cc + 7 * y
    var yy = y + 16
    print(xx, yy, z)
    //test_print(x, y, z)
    return xx
}
*/

function fibonacci(x)
{
    if (x < 2)
    {
        return 1
    }
    
    return fibonacci(x - 1) + fibonacci(x - 2)
}

print(fibonacci(0))
print(fibonacci(1))
print(fibonacci(2))
print(fibonacci(3))
print(fibonacci(4))
print(fibonacci(5))
print(fibonacci(6))

/*
function test(x)
{
    x = x + 5
    if (x > 5)
    {
        var y = 10
        print(x + y, ">5")
    }
    else if (x > 3)
    {
        var y = "aaa"
        print(x, y, ">3")
        if (x > 10)
        {
            print(">10")
        }
        else
        {
            print("<10")
        }
        print("step")
    }
    else
    {
        print(x)
        if (x == 0)
        {
            print("==0")
        }
    }

    x = x / 5
}

//test(0)
//test(3)
//test(-5)*/
//var expr = 8 + -(-(-3 + 4)) * 6
//int aa = 8 + ~(-3 + 4) * 6;
//call(5)

/*
8+   -(    -3+    4)   *   6
8+   -(    1)     *    6
8+   -1*   6
8+   -6
2
*/

/*
var a = 0
if (a < 2)
{
    a = 2
}
*/
/*
function echo(x)
{
    if (x % 2)
    {
        x = x * 0.1
    }

    return x
}

print(echo(1) + echo(2))
*/

/*
function test(x)
{
    print(x)
    if (x <= 0)
    {
        return 0
    }

    return test(x - 1)
}

test(10)

function test(x)
{
    return x + 1
}
*/
