#
# $Header: /var/cvs/d4x/admin/strip.awk,v 1.1 2002/03/16 12:44:40 zaufi Exp $
#
# Split incoming string, sort, and merge excluding duplicates
#
END {
    for (i = 1; i <= NF; i++) a[i]=$i
    n = asort(a)
    result=a[1]
    prev=a[1]
    for (i = 2; i <= n; i++)
    {
	if (prev != a[i]) result = result " " a[i]
	prev = a[i]
    }
    print result
}
