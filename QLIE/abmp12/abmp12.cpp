#include <iostream>

int main(int argc, char *argv[])
{
	if (argc != 2) return 0;

	char format[300], newname[300];
	strcpy(format, argv[1]);
	format[strlen(format)-2] = 0;		// *.b
	strcat(format, "_%u.png");

	FILE *fp = fopen(argv[1], "rb");

	fseek(fp, 0, 2);
	
	long len = ftell(fp);
	unsigned char *file = new unsigned char[len];

	rewind(fp);
	fread(file, 1, len, fp);
	fclose(fp);

	unsigned char *p = file, *pend = file + len;
	int cnt = 1;
	while (p < pend)
	{
		
		while (p<pend && *(unsigned long *)p != 0x474e5089) // ‰PNG
			++p;

		if (p >= pend) break;

		unsigned long sublen = *(unsigned long *)(p-4);

		unsigned char *filesave = new unsigned char [sublen];

		memcpy(filesave, p, sublen);
		sprintf(newname, format, cnt++);
		fp = fopen(newname, "wb");

		fwrite(filesave, 1, sublen, fp);
		fclose(fp);
		delete[] filesave;
		p += sublen;
	}
	delete[] file;
	return 0;
}