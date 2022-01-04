function test(x)
{
    return x
}

function test1()
{
    return {aa = {["1"] = 12}}
}

var t = {{4, aa = test1}, cc = {7, 99}, test="test1", 5.5, 89, aa="test11", [12] = 10, ["aa"] = 100, [test(55)] = 88, bb = 6 * 6 - 8, test, [test] = "ccc"}
print(t[0].aa().aa["1"] + t[55])
print(t[0].aa()["aa"]["1"] + t[55])
print(t[55])
//print(t[1][2])
//print(t.test)
//print(t.aa)
//var tt = 4 + 5 * {} - -6
