
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

print(test_print("aaaa\n\n", 11 * 6, 33 * 5))
test_print("bbbb", 22, 44)

function test_print(x, y, z)
{
    print(x, y, z)
    //test_print(x, y, z)
    return "111"
}