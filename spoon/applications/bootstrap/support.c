

int atoi(const char *nptr)
{
	int ans = 0;
	int i = 0;

	while ( nptr[i] != 0 )
	{
		int c = (nptr[i] - '0');
		if ( (c < 0) || (c > 9) )	return -1;
		ans = ans * 10 + c;

		i += 1;
	}

	return ans;
}


