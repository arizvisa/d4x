#
# $Header: /var/cvs/d4x/admin/strip.awk,v 1.3 2002/03/20 15:15:14 zaufi Exp $
#
# Split incoming strings, sort, and merge excluding duplicates
#
BEGIN {
    curpos = 0
}
{
    for (i = 1; i <= NF; i++) array[curpos++] = $i;
}
END {
    r_curpos = 0
    for (i = 0; i < curpos; i++)
    {
	for (j = 0; j < r_curpos; j++)  if (result[j] == array[i]) break;
	if (j == r_curpos)  result[r_curpos++] = array[i];
    }
    result_str = result[0]
    for (i = 1; i < r_curpos; result_str = result_str " " result[i++]);

    print result_str
}
