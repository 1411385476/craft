#include <stdio.h>
#include <sqlite3.h>
void main( void )
{
	int rc,i;
	char msg_temp[600];
	sqlite3 *db;
	char msg[] = "ssjfkjslfjslfdjsajfsssss";
	rc = sqlite3_open("/home/scrooph/craft/hash/traffic.db", &db);
	char * pErrMsg = 0;
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else{ 
		//printf("You have opened a sqlite3 database named zieckey.db successfully!nCongratulations! Have fun ! ^-^ n");
		sprintf(msg_temp,"insert into nat(`content`) values('%s');",msg);
		sqlite3_exec( db,msg_temp, 0, 0, &pErrMsg);
		printf("%s\n",pErrMsg);
		sqlite3_exec( db,"insert into nat(`content`) values('ssssssssssss');", 0, 0, &pErrMsg);
		printf("%s\n",pErrMsg);
	}
	sqlite3_close(db); //关闭
}
