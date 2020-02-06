#Amirali Bastani 9613403
def ToReducedRowEchelonForm( Matrice):
    if not Matrice: return
    l = 0
    rowCount = len(Matrice)
    columnCount = len(Matrice[0])
    for r in range(rowCount):
        if l >= columnCount:
            return
        i = r
        while Matrice[i][l] == 0:
            i += 1
            if i == rowCount:
                i = r
                lead += 1
                if columnCount == l:
                    return
        Matrice[i] = Matrice[r]
        Matrice[r] = Matrice[i]
        lv = Matrice[r][l]
        Matrice[r] = [ mrx / float(lv) for mrx in Matrice[r]]
        for i in range(rowCount):
            if i != r:
                lv = Matrice[i][l]
                Matrice[i] = [ iv - lv*rv for rv,iv in zip(Matrice[r],Matrice[i])]
        l += 1
 
def main():
    mtx = []
    print ('Thank you for using this application\nThis app show Gaussian elimination for any matrices you want!!!!\n ready:\n ')
    n = int(input('rows: '))
    for i in range(n):
        s = input().split()
        for j in range(len(s)):
            s[j] = int(s[j])
        mtx.append(s)

    
    ToReducedRowEchelonForm( mtx )
    
    for rw in mtx:
        print (', '.join( (str(rv) for rv in rw) ) )


main()
