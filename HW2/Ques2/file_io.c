#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

void main() {

	printf("\nYou'll Never Walk Alone");

	FILE * fp;
	fp= fopen("output.txt","r");

	fchmod( fileno(fp), S_IRUSR|S_IWUSR );

	fp= fopen("output.txt", "w");

	fputc( 'A', fp );

	fclose(fp);

	fp= fopen("output.txt", "a");

	char *str= (char *)malloc(sizeof(char)*50);

	printf("\nEnter a string:");
	scanf("%s",str);

	fputs(str, fp);

	fflush(fp);
	
	fclose(fp);

	fp = fopen("output.txt", "r");

	char c= fgetc(fp);
	printf("\nReceived character: %c",c);

	char recdStr[sizeof(str)];
	fgets(recdStr, sizeof(str),fp);
	printf("\nReceived string: %s\n", recdStr);

	fclose (fp);

	free (fp);
	free (str);
	
}
