def solution(S):
    """
    odd, the last bit is 1, subtract 1 , then the last bit is 0
    even, the last bit is 0, divide it by2, which is equal to shifting left 1 bit
    ===> for every bits except the first "1", 
        if it is "1", it need 2 such operations(subtract & divide)
        if it is "0", it need 1 such operation(divide)
         for the first "1"bit, it need 1 such operation(subtract)
    """
    ans = 0
    
    n = len(S)
    idx = 0
    # find the first "1"
    for i in range(n):
        if S[i] == "1":
            idx = i
            break
    ans += 1
    
    print(ans)
    # for every bits except the first "1", 
    for i in range(idx+1, n):
        
        if S[i] == "1": 
            ans += 2    # "1", it need 2 such operations(subtract & divide)
        else:
            ans += 1    # "0", it need 1 such operation(divide)
    return ans


solution('100000')